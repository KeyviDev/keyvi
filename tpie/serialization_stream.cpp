// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#include <tpie/serialization_stream.h>
#include <tpie/array.h>

///////////////////////////////////////////////////////////////////////////////
// serialization_header {{{

namespace tpie {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \class Stream accessor for serialization streams.
///
/// This class handles the stream header of a given file accessor.
///////////////////////////////////////////////////////////////////////////////
class serialization_header {
public:
	static memory_size_type header_size() {
		memory_size_type sz = sizeof(stream_header_t);
		memory_size_type align = 4096;
		return (sz + (align-1))/align * align;
	}

	serialization_header(file_accessor::raw_file_accessor & file)
		: m_headerPtr(new stream_header_t())
		, m_header(*m_headerPtr)
		, m_fileAccessor(file)
	{
		m_header.magic = stream_header_t::magicConst;
		m_header.version = stream_header_t::versionConst;
		m_header.size = 0;
		m_header.cleanClose = 0;
	}

	void read() {
		m_fileAccessor.seek_i(0);
		m_fileAccessor.read_i(&m_header, sizeof(m_header));
	}

	void write(bool cleanClose) {
		m_header.cleanClose = cleanClose;

		tpie::array<char> headerArea(header_size());
		std::fill(headerArea.begin(), headerArea.end(), '\x42');
		char * headerData = reinterpret_cast<char *>(&m_header);
		std::copy(headerData, sizeof(m_header) + headerData,
				  headerArea.begin());

		m_fileAccessor.seek_i(0);
		m_fileAccessor.write_i(&headerArea[0], headerArea.size());
	}

	void verify() {
		if (m_header.magic != m_header.magicConst)
			throw stream_exception("Bad header magic");
		if (m_header.version < m_header.versionConst)
			throw stream_exception("Stream version too old");
		if (m_header.version > m_header.versionConst)
			throw stream_exception("Stream version too new");
		if (m_header.cleanClose != 1)
			throw stream_exception("Stream was not closed properly");
		if (m_header.reverse != 0 && m_header.reverse != 1)
			throw stream_exception("Reverse flag is not a boolean");
	}

	stream_size_type get_size() {
		return m_header.size;
	}

	void set_size(stream_size_type size) {
		m_header.size = size;
	}

	bool get_clean_close() {
		return m_header.cleanClose;
	}

	bool get_reverse() {
		return m_header.reverse;
	}

	void set_reverse(bool reverse) {
		m_header.reverse = reverse;
	}

private:
#pragma pack(push, 1)
	struct stream_header_t {
		static const uint64_t magicConst = 0xfa340f49edbada67ll;
		static const uint64_t versionConst = 1;

		uint64_t magic;
		uint64_t version;
		uint64_t size;
		// bool has funny semantics with regards to equality. we want to reject
		// invalid bool values (>1), but that is not easy to express with a C++
		// bool variable.
		char cleanClose;
		char reverse;
	};
#pragma pack(pop)

	std::auto_ptr<stream_header_t> m_headerPtr;
	stream_header_t & m_header;

	file_accessor::raw_file_accessor & m_fileAccessor;
};

} // namespace bits

} // namespace tpie

// }}}
///////////////////////////////////////////////////////////////////////////////

namespace {
	class open_guard {
		bool & open_flag;
		tpie::file_accessor::raw_file_accessor & fa;
		bool committed;
	public:
		open_guard(bool & open_flag, tpie::file_accessor::raw_file_accessor & fa)
			: open_flag(open_flag)
			, fa(fa)
			, committed(false)
		{
			open_flag = true;
		}

		void commit() {
			committed = true;
		}

		~open_guard() {
			if (!committed) {
				open_flag = false;
				fa.close_i();
			}
		}
	};
} // unnamed namespace

namespace tpie {

namespace bits {

serialization_writer_base::serialization_writer_base()
	: m_blocksWritten(0)
	, m_size(0)
	, m_open(false)
	, m_tempFile(0)
{
}

void serialization_writer_base::open_inner(std::string path, bool reverse) {
	close(reverse);
	m_fileAccessor.set_cache_hint(access_sequential);
	m_fileAccessor.open_wo(path);
	open_guard guard(m_open, m_fileAccessor);
	m_blocksWritten = 0;
	m_size = 0;

	bits::serialization_header header(m_fileAccessor);
	header.set_reverse(reverse);
	header.write(false);
	guard.commit();
}

void serialization_writer_base::open(std::string path, bool reverse) {
	m_tempFile = 0;
	open_inner(path, reverse);
}

void serialization_writer_base::open(temp_file & tempFile, bool reverse) {
	m_tempFile = &tempFile;
	open_inner(tempFile.path(), reverse);
}

void serialization_writer_base::write_block(const char * const s, const memory_size_type n) {
	assert(n <= block_size());
	stream_size_type offset = m_blocksWritten * block_size();
	m_fileAccessor.seek_i(bits::serialization_header::header_size() + offset);
	m_fileAccessor.write_i(s, n);
	++m_blocksWritten;
	m_size = offset + n;
	if (m_tempFile)
		m_tempFile->update_recorded_size(m_size);
}

void serialization_writer_base::close(bool reverse) {
	if (!m_open) return;
	bits::serialization_header header(m_fileAccessor);
	header.set_size(m_size);
	header.set_reverse(reverse);
	header.write(true);
	m_fileAccessor.close_i();
	m_open = false;
	m_tempFile = 0;
}

stream_size_type serialization_writer_base::file_size() {
	return serialization_header::header_size() + m_size;
}

} // namespace bits

serialization_writer::serialization_writer()
	: m_index(0)
{
}

serialization_writer::~serialization_writer() {
	close();
}

void serialization_writer::write_block() {
	p_t::write_block(m_block.get(), m_index);
	m_index = 0;
}

void serialization_writer::open(std::string path) {
	p_t::open(path, false);
	m_block.resize(block_size());
	m_index = 0;
}

void serialization_writer::open(temp_file & tempFile) {
	p_t::open(tempFile, false);
	m_block.resize(block_size());
	m_index = 0;
}

void serialization_writer::close() {
	if (m_index > 0) write_block();
	m_block.resize(0);
	m_index = 0;
	p_t::close(false);
}

serialization_reverse_writer::serialization_reverse_writer()
	: m_index(0)
{
}

serialization_reverse_writer::~serialization_reverse_writer() {
	close();
}

void serialization_reverse_writer::write_block() {
	// See note about m_index and its semantics.
	std::reverse(m_block.get(), m_block.get() + block_size());
	p_t::write_block(m_block.get(), m_index);
	m_index = 0;
}

void serialization_reverse_writer::open(std::string path) {
	p_t::open(path, true);
	m_block.resize(block_size());
	m_index = 0;
}

void serialization_reverse_writer::open(temp_file & tempFile) {
	p_t::open(tempFile, true);
	m_block.resize(block_size());
	m_index = 0;
}

void serialization_reverse_writer::close() {
	if (m_index > 0) write_block();
	m_block.resize(0);
	m_index = 0;
	p_t::close(true);
}

namespace bits {

serialization_reader_base::serialization_reader_base()
	: m_open(false)
	, m_size(0)
	, m_index(0)
	, m_blockSize(0)
{
}

void serialization_reader_base::open(std::string path, bool reverse) {
	close();
	m_fileAccessor.set_cache_hint(reverse ? access_normal : access_sequential);
	m_fileAccessor.open_ro(path);
	open_guard guard(m_open, m_fileAccessor);
	m_block.resize(block_size());
	m_index = 0;
	m_blockSize = 0;

	bits::serialization_header header(m_fileAccessor);
	header.read();
	header.verify();
	m_size = header.get_size();
	if (reverse && !header.get_reverse())
		throw stream_exception("Opened a non-reverse stream for reverse reading");
	if (!reverse && header.get_reverse())
		throw stream_exception("Opened a reverse stream for non-reverse reading");
	guard.commit();
}

void serialization_reader_base::read_block(const stream_size_type blk) {
	stream_size_type from = blk * block_size();
	stream_size_type to = std::min(from + block_size(), m_size);
	if (to <= from) throw end_of_stream_exception();
	m_index = 0;
	m_blockSize = to-from;
	m_fileAccessor.seek_i(bits::serialization_header::header_size()
						  + from);
	m_fileAccessor.read_i(m_block.get(), m_blockSize);
}

void serialization_reader_base::close() {
	if (!m_open) return;
	m_fileAccessor.close_i();
	m_open = false;
	m_block.resize(0);
}

stream_size_type serialization_reader_base::file_size() {
	return serialization_header::header_size() + m_size;
}

stream_size_type serialization_reader_base::size() {
	return m_size;
}

} // namespace bits

void serialization_reader::next_block() /*override*/ {
	if (m_blockSize == 0) {
		m_blockNumber = 0;
	} else {
		++m_blockNumber;
	}
	read_block(m_blockNumber);
}

serialization_reader::serialization_reader()
	: m_blockNumber(0)
{
}

void serialization_reader::open(std::string path) {
	p_t::open(path, false);
	m_blockNumber = 0;
}

void serialization_reader::open(temp_file & tempFile) {
	open(tempFile.path());
}

stream_size_type serialization_reader::offset() {
	if (m_blockSize == 0)
		return 0;

	return m_blockNumber * block_size() + m_index;
}

void serialization_reverse_reader::next_block() /*override*/ {
	if (m_blockNumber == 0)
		throw end_of_stream_exception();
	--m_blockNumber;
	read_block(m_blockNumber);
	std::reverse(m_block.begin(), m_block.begin() + m_blockSize);
}

serialization_reverse_reader::serialization_reverse_reader()
	: m_blockNumber(0)
{
}

void serialization_reverse_reader::open(std::string path) {
	p_t::open(path, true);
	m_blockNumber = (m_size + (block_size() - 1)) / block_size();
}

void serialization_reverse_reader::open(temp_file & tempFile) {
	open(tempFile.path());
}

stream_size_type serialization_reverse_reader::offset() {
	if (m_blockSize == 0)
		return size();

	// size of blocks not read at all
	stream_size_type remainingBlocks = m_blockNumber * block_size();
	return size() - remainingBlocks - m_blockSize + m_index;
}

} // namespace tpie
