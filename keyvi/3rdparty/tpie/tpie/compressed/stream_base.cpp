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

#include <tpie/compressed/stream.h>
#include <tpie/file_base_crtp.inl>

namespace tpie {

compressed_stream_base::compressed_stream_base(memory_size_type itemSize,
											   double blockFactor)
	: m_bufferDirty(false)
	, m_blockItems(block_size(blockFactor) / itemSize)
	, m_blockSize(block_size(blockFactor))
	, m_canRead(false)
	, m_canWrite(false)
	, m_open(false)
	, m_itemSize(itemSize)
	, m_cachedReads(0)
	, m_cachedWrites(0)
	, m_ownedTempFile(/* empty auto_ptr */)
	, m_tempFile(0)
	, m_byteStreamAccessor()
	, m_size(0)
	, m_buffers(m_blockSize)
	, m_buffer(/* empty shared_ptr */)
	, m_streamBlocks(0)
	, m_lastBlockReadOffset(0)
	, m_currentFileSize(0)
	, m_response()
	, m_seekState(seek_state::beginning)
	, m_readOffset(0)
	, m_offset(0)
	, m_nextPosition(/* not a position */)
	, m_nextReadOffset(0)
{
	// Empty constructor.
}

compressed_stream_base::~compressed_stream_base() {
	// We cannot close() here since flush_block() is pure virtual at this point.
	if (is_open())
		log_debug() << "compressed_stream_base destructor reached "
			<< "while stream is still open." << std::endl;
	// non-trivial field destructors:
	// m_response::~compressor_response()
	// m_buffer::~shared_ptr()
	// m_buffers::~stream_buffers()
	// m_byteStreamAccessor::~byte_stream_accessor()
	// m_ownedTempFile::~auto_ptr()
}

void compressed_stream_base::open_inner(const std::string & path,
										access_type accessType,
										memory_size_type userDataSize,
										cache_hint cacheHint,
										int compressionFlags)
{
	m_canRead = accessType == access_read || accessType == access_read_write;
	m_canWrite = accessType == access_write || accessType == access_read_write;
	m_byteStreamAccessor.open(path, m_canRead, m_canWrite, m_itemSize,
							  m_blockSize, userDataSize, cacheHint,
							  compressionFlags);
	m_size = m_byteStreamAccessor.size();
	m_open = true;
	m_streamBlocks = (m_size + m_blockItems - 1) / m_blockItems;
	m_lastBlockReadOffset = m_byteStreamAccessor.get_last_block_read_offset();
	m_currentFileSize = m_byteStreamAccessor.file_size();
	m_response.clear_block_info();

	this->post_open();
}

/*static*/ memory_size_type compressed_stream_base::block_size(double blockFactor) throw () {
	return static_cast<memory_size_type>(get_block_size() * blockFactor);
}

/*static*/ double compressed_stream_base::calculate_block_factor(memory_size_type blockSize) throw () {
	return (double)blockSize / (double)block_size(1.0);
}

/*static*/ memory_size_type compressed_stream_base::block_memory_usage(double blockFactor) {
	return block_size(blockFactor);
}

memory_size_type compressed_stream_base::block_items() const {
	return m_blockItems;
}

memory_size_type compressed_stream_base::block_size() const {
	return m_blockSize;
}

memory_size_type compressed_stream_base::read_user_data(void * data, memory_size_type count) {
	tp_assert(is_open(), "read_user_data: !is_open");
	return m_byteStreamAccessor.read_user_data(data, count);
}

void compressed_stream_base::write_user_data(const void * data, memory_size_type count) {
	tp_assert(is_open(), "write_user_data: !is_open");
	m_byteStreamAccessor.write_user_data(data, count);
}

memory_size_type compressed_stream_base::user_data_size() const {
	tp_assert(is_open(), "user_data_size: !is_open");
	return m_byteStreamAccessor.user_data_size();
}

memory_size_type compressed_stream_base::max_user_data_size() const {
	tp_assert(is_open(), "max_user_data_size: !is_open");
	return m_byteStreamAccessor.max_user_data_size();
}

const std::string & compressed_stream_base::path() const {
	assert(m_open);
	return m_byteStreamAccessor.path();
}

void compressed_stream_base::open(const std::string & path,
								  access_type accessType /*= access_read_write*/,
								  memory_size_type userDataSize /*= 0*/,
								  cache_hint cacheHint/*=access_sequential*/,
								  compression_flags compressionFlags/*=compression_normal*/)
{
	close();
	open_inner(path, accessType, userDataSize, cacheHint, compressionFlags);
}

void compressed_stream_base::open(memory_size_type userDataSize /*= 0*/,
								  cache_hint cacheHint/*=access_sequential*/,
								  compression_flags compressionFlags/*=compression_normal*/)
{
	close();
	m_ownedTempFile.reset(tpie_new<temp_file>());
	m_tempFile = m_ownedTempFile.get();
	open_inner(m_tempFile->path(), access_read_write, userDataSize,
			   cacheHint, compressionFlags);
}

void compressed_stream_base::open(temp_file & file,
								  access_type accessType /*= access_read_write*/,
								  memory_size_type userDataSize /*= 0*/,
								  cache_hint cacheHint/*=access_sequential*/,
								  compression_flags compressionFlags/*=compression_normal*/)
{
	close();
	m_tempFile = &file;
	open_inner(m_tempFile->path(), accessType, userDataSize,
			   cacheHint, compressionFlags);
}

void compressed_stream_base::close() {
	uncache_read_writes();
	if (m_open) {
		compressor_thread_lock l(compressor());

		if (m_bufferDirty)
			flush_block(l);

		m_buffer.reset();

		finish_requests(l);

		if (use_compression()) {
			m_byteStreamAccessor.set_last_block_read_offset(last_block_read_offset(l));
		}
		m_byteStreamAccessor.set_size(m_size);
		m_byteStreamAccessor.close();
	}
	m_open = false;
	m_tempFile = NULL;
	m_ownedTempFile.reset();

	// If read/write is called on the closed stream,
	// perform_seek throws an exception.
	m_seekState = seek_state::beginning;
}

void compressed_stream_base::finish_requests(compressor_thread_lock & l) {
	tp_assert(!(m_buffer.get() != 0), "finish_requests called when own buffer is still held");
	m_buffers.clean();
	while (!m_buffers.empty()) {
		compressor().wait_for_request_done(l);
		m_buffers.clean();
	}
}

stream_size_type compressed_stream_base::last_block_read_offset(compressor_thread_lock & l) {
	tp_assert(use_compression(), "last_block_read_offset: !use_compression");
	if (m_streamBlocks == 0 || m_streamBlocks == 1)
		return 0;
	if (m_lastBlockReadOffset != std::numeric_limits<stream_size_type>::max())
		return m_lastBlockReadOffset;
	// We assume that streamBlocks is monotonically increasing over time;
	// the response object might throw otherwise.
	while (!m_response.has_block_info(m_streamBlocks - 1))
		m_response.wait(l);
	return m_response.get_read_offset(m_streamBlocks - 1);
}

stream_size_type compressed_stream_base::current_file_size(compressor_thread_lock & l) {
	tp_assert(use_compression(), "current_file_size: !use_compression");
	if (m_streamBlocks == 0)
		return 0;
	if (m_currentFileSize != std::numeric_limits<stream_size_type>::max())
		return m_currentFileSize;
	// We assume that streamBlocks is monotonically increasing over time;
	// the response object might throw otherwise.
	while (!m_response.has_block_info(m_streamBlocks - 1))
		m_response.wait(l);
	return m_response.get_read_offset(m_streamBlocks - 1)
		+ m_response.get_block_size(m_streamBlocks - 1);
}

} // namespace tpie
