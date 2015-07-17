// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012 The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file file_stream_base.h  Item type-agnostic file_stream operations
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_FILE_STREAM_BASE_H__
#define __TPIE_FILE_STREAM_BASE_H__

#include <tpie/file_base_crtp.h>
#include <tpie/stream_crtp.h>

namespace tpie {

class file_stream_base: public file_base_crtp<file_stream_base>, public stream_crtp<file_stream_base> {
public:
	typedef file_base_crtp<file_stream_base> p_t;
	typedef stream_crtp<file_stream_base> s_t;

	friend class file_base_crtp<file_stream_base>;

	struct block_t {
		memory_size_type size;
		stream_size_type number;
		bool dirty;
		char * data;
	};

	/////////////////////////////////////////////////////////////////////////
	/// \brief Close the file and release resources.
	///
	/// This will close the file and resources used by buffers and such.
	/////////////////////////////////////////////////////////////////////////
	inline void close() throw(stream_exception) {
		if (m_open) flush_block();
		tpie_delete_array(m_block.data, m_itemSize * m_blockItems);
		m_block.data = 0;
		p_t::close();
	}


	///////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::truncate()
	/// \sa file_base::truncate()
	///
	/// Note that when using a file_stream the stream will automatically be
	/// rewound if it is beyond the new end of the file. 
	///////////////////////////////////////////////////////////////////////////
	inline void truncate(stream_size_type size) {
		stream_size_type o=offset();
		flush_block();
		m_block.number = std::numeric_limits<stream_size_type>::max();
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();
		m_size = size;
		m_fileAccessor->truncate(size);
		if (m_tempFile)
			m_tempFile->update_recorded_size(m_fileAccessor->byte_size());
		seek(std::min(o, size));
	}

protected:
	file_stream_base(memory_size_type itemSize,
					 double blockFactor,
					 file_accessor::file_accessor * fileAccessor);

	inline ~file_stream_base() {
		close();
	}

	void swap(file_stream_base & other) {
		using std::swap;
		swap(m_index,           other.m_index);
		swap(m_nextBlock,       other.m_nextBlock);
		swap(m_nextIndex,       other.m_nextIndex);
		swap(m_blockStartIndex, other.m_blockStartIndex);
		swap(m_blockItems,      other.m_blockItems);
		swap(m_blockSize,       other.m_blockSize);
		swap(m_size,            other.m_size);
		swap(m_canRead,         other.m_canRead);
		swap(m_canWrite,        other.m_canWrite);
		swap(m_itemSize,        other.m_itemSize);
		swap(m_open,            other.m_open);
		swap(m_fileAccessor,    other.m_fileAccessor);
		swap(m_block.size,      other.m_block.size);
		swap(m_block.number,    other.m_block.number);
		swap(m_block.dirty,     other.m_block.dirty);
		swap(m_block.data,      other.m_block.data);
		swap(m_ownedTempFile,   other.m_ownedTempFile);
		swap(m_tempFile,        other.m_tempFile);
	}

	inline void open_inner(const std::string & path,
						   access_type accessType,
						   memory_size_type userDataSize,
						   cache_hint cacheHint) throw (stream_exception) {
		p_t::open_inner(path, accessType, userDataSize, cacheHint);

		m_blockStartIndex = 0;
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();

		m_block.size = 0;
		m_block.number = std::numeric_limits<stream_size_type>::max();
		m_block.dirty = false;
		m_block.data = tpie_new_array<char>(m_blockItems * m_itemSize);

		initialize();
		seek(0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Use file_accessor to fetch indicated block number into m_block.
	///////////////////////////////////////////////////////////////////////////
	void get_block(stream_size_type block);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Write block to disk.
	///////////////////////////////////////////////////////////////////////////
	inline void flush_block() {
		if (m_block.dirty) {
			assert(m_canWrite);
			update_vars();
			m_fileAccessor->write_block(m_block.data, m_block.number, m_block.size);
			if (m_tempFile)
				m_tempFile->update_recorded_size(m_fileAccessor->byte_size());
		}
		m_block.dirty = false;
	}

	inline void update_vars() {
		if (m_block.dirty && m_index != std::numeric_limits<memory_size_type>::max()) {
			assert(m_index <= m_blockItems);
			m_block.size = std::max(m_block.size, m_index);
			m_size = std::max(m_size, static_cast<stream_size_type>(m_index)+m_blockStartIndex);
		}
	}

	inline void initialize() {
		flush_block();
		s_t::initialize();
	}

	inline void write_update() {
		m_block.dirty = true;
	}


	block_t m_block;

private:
	friend class stream_crtp<file_stream_base>;
	file_stream_base & get_file() {return *this;}
	const file_stream_base & get_file() const {return *this;}
	block_t & get_block() {return m_block;}
	const block_t & get_block() const {return m_block;}
	void update_block_core();
};

} // namespace tpie

#endif // __TPIE_FILE_STREAM_BASE_H__
