// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#ifndef TPIE_COMPRESSED_REQUEST_H
#define TPIE_COMPRESSED_REQUEST_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/request.h  Compressor thread requests and responses.
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <thread>
#include <condition_variable>
#include <tpie/tpie_assert.h>
#include <tpie/tempname.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/file_accessor/byte_stream_accessor.h>
#include <tpie/compressed/predeclare.h>
#include <tpie/compressed/direction.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Response to an I/O request.
///
/// The response object is used to relay information back from the compressor
/// thread to the stream class.
/// The compressor mutex must be acquired before any compressor_response method
/// is called.
/// In the code, each method is annotated with a (request kind, caller)-comment,
/// identifying whether the method relates to a read request or a write
/// request, and whether the stream object (main thread) or the compressor
/// thread should call it.
///////////////////////////////////////////////////////////////////////////////
class compressor_response {
public:
	compressor_response()
		: m_done(false)
		, m_blockNumber(std::numeric_limits<stream_size_type>::max())
		, m_readOffset(0)
		, m_blockSize(0)
		, m_endOfStream(false)
		, m_nextReadOffset(0)
		, m_nextBlockSize(0)
	{
	}

	// any, stream
	// Waits on the condition variable m_changed.
	void wait(compressor_thread_lock & lock);

	// any, stream
	void initiate_request() {
		m_done = m_endOfStream = false;
		m_nextReadOffset = m_nextBlockSize = 0;
	}

	// write, stream
	void clear_block_info() {
		m_blockNumber = std::numeric_limits<stream_size_type>::max();
	}

	// write, thread -- must have lock!
	void set_block_info(stream_size_type blockNumber,
						stream_size_type readOffset,
						memory_size_type blockSize)
	{
		if (m_blockNumber != std::numeric_limits<stream_size_type>::max()
			&& blockNumber < m_blockNumber)
		{
			//log_debug() << "set_block_info(blockNumber=" << blockNumber
			//	<< ", readOffset=" << readOffset << ", blockSize=" << blockSize << "): "
			//	<< "We already know the size of block " << m_blockNumber << std::endl;
		} else {
			//log_debug() << "set_block_info(blockNumber=" << blockNumber
			//	<< ", readOffset=" << readOffset << ", blockSize=" << blockSize << "): "
			//	<< "Previous was " << m_blockNumber << std::endl;
			m_blockNumber = blockNumber;
			m_readOffset = readOffset;
			m_blockSize = blockSize;
			m_changed.notify_all();
		}
	}

	// write, stream
	bool has_block_info(stream_size_type blockNumber)
	{
		if (m_blockNumber == std::numeric_limits<stream_size_type>::max())
			return false;

		if (blockNumber < m_blockNumber) {
			std::stringstream ss;
			ss << "Wanted block number " << blockNumber << ", but recalled was " << m_blockNumber;
			throw exception(ss.str());
		}

		if (blockNumber == m_blockNumber)
			return true;
		else // blockNumber > m_blockNumber
			return false;
	}

	// write, stream
	memory_size_type get_block_size(stream_size_type blockNumber)
	{
		tp_assert(has_block_info(blockNumber), "get_block_size: !has_block_info");
		unused(blockNumber);
		return m_blockSize;
	}

	// write, stream
	stream_size_type get_read_offset(stream_size_type blockNumber) {
		tp_assert(has_block_info(blockNumber), "get_read_offset: !has_block_info");
		unused(blockNumber);
		return m_readOffset;
	}

	// any, thread
	void set_done() {
		m_done = true;
	}

	// any, stream
	bool done() {
		return m_done;
	}

	// read, stream
	stream_size_type next_read_offset() {
		return m_nextReadOffset;
	}

	// read, thread
	void set_next_block_offset(stream_size_type offset) {
		m_done = true;
		m_nextReadOffset = offset;
		m_changed.notify_all();
	}

private:
	std::condition_variable m_changed;

	// Information about either read or write
	bool m_done;

	// Information about the write
	stream_size_type m_blockNumber;
	stream_size_type m_readOffset;
	memory_size_type m_blockSize;

	// Information about the read
	bool m_endOfStream;
	stream_size_type m_nextReadOffset;
	memory_size_type m_nextBlockSize;
};

#ifdef __GNUC__
class __attribute__((__may_alias__)) request_base;
class __attribute__((__may_alias__)) read_request;
class __attribute__((__may_alias__)) write_request;
#endif // __GNUC__

///////////////////////////////////////////////////////////////////////////////
/// \brief  Base class for read_request and write_request.
///
/// Each request should have a pointer to the response object, which is
/// contained in this base class.
///////////////////////////////////////////////////////////////////////////////
class request_base {
protected:
	request_base(compressor_response * response)
		: m_response(response)
	{
	}

public:
	void initiate_request() {
		m_response->initiate_request();
	}

protected:
	compressor_response * m_response;
};

class read_request : public request_base {
public:
	typedef std::shared_ptr<compressor_buffer> buffer_t;
	typedef file_accessor::byte_stream_accessor<default_raw_file_accessor> file_accessor_t;
	typedef std::condition_variable condition_t;

	read_request(buffer_t buffer,
				 file_accessor_t * fileAccessor,
				 stream_size_type readOffset,
				 read_direction::type readDirection,
				 compressor_response * response)
		: request_base(response)
		, m_buffer(buffer)
		, m_fileAccessor(fileAccessor)
		, m_readOffset(readOffset)
		, m_readDirection(readDirection)
	{
	}

	buffer_t buffer() {
		return m_buffer;
	}

	file_accessor_t & file_accessor() {
		return *m_fileAccessor;
	}

	stream_size_type read_offset() {
		return m_readOffset;
	}

	read_direction::type get_read_direction() {
		return m_readDirection;
	}

	void set_next_block_offset(stream_size_type offset) {
		m_response->set_next_block_offset(offset);
	}

private:
	buffer_t m_buffer;
	file_accessor_t * m_fileAccessor;
	const stream_size_type m_readOffset;
	const read_direction::type m_readDirection;
};

class write_request : public request_base {
public:
	typedef std::shared_ptr<compressor_buffer> buffer_t;
	typedef file_accessor::byte_stream_accessor<default_raw_file_accessor> file_accessor_t;

	write_request(const buffer_t & buffer,
				  file_accessor_t * fileAccessor,
				  temp_file * tempFile,
				  stream_size_type writeOffset,
				  memory_size_type blockItems,
				  stream_size_type blockNumber,
				  compressor_response * response)
		: request_base(response)
		, m_buffer(buffer)
		, m_fileAccessor(fileAccessor)
		, m_tempFile(tempFile)
		, m_writeOffset(writeOffset)
		, m_blockItems(blockItems)
		, m_blockNumber(blockNumber)
	{
	}

	file_accessor_t & file_accessor() {
		return *m_fileAccessor;
	}

	buffer_t buffer() {
		return m_buffer;
	}

	temp_file * get_temp_file() {
		return m_tempFile;
	}

	memory_size_type block_items() {
		return m_blockItems;
	}

	bool should_append() {
		return m_writeOffset == std::numeric_limits<stream_size_type>::max();
	}

	stream_size_type write_offset() {
		return m_writeOffset;
	}

	// must have lock!
	void set_block_info(stream_size_type readOffset,
						memory_size_type blockSize)
	{
		m_response->set_block_info(m_blockNumber, readOffset, blockSize);
	}

	// must have lock!
	void update_recorded_size() {
		m_response->set_done();
		if (m_tempFile != NULL) m_tempFile->update_recorded_size(m_fileAccessor->file_size());
	}

	// must have lock!
	void update_recorded_size(stream_size_type fileSize) {
		m_response->set_done();
		if (m_tempFile != NULL) m_tempFile->update_recorded_size(fileSize);
	}

private:
	buffer_t m_buffer;
	file_accessor_t * m_fileAccessor;
	temp_file * m_tempFile;
	const stream_size_type m_writeOffset;
	const memory_size_type m_blockItems;
	const stream_size_type m_blockNumber;
};

class compressor_request_kind {
public:
	enum type {
		NONE,
		READ,
		WRITE
	};

private:
	compressor_request_kind() /*= delete*/;
	compressor_request_kind(const compressor_request_kind &) /*= delete*/;
	~compressor_request_kind() /*= delete*/;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Tagged union containing either a read_request or a write_request.
///
/// In C++11, this can be implemented more elegantly since the C++11 standard
/// allows unions with member types having constructors and destructors.
/// In C++03, we have to emulate this with a char buffer that is large enough.
///
/// To turn the tagged union into a T, call set_T(ctor params) which returns a
/// reference to T. When the tagged union is a T (check with kind()), use
/// get_T() to get the reference to T.
///////////////////////////////////////////////////////////////////////////////
class compressor_request {
public:
	compressor_request()
		: m_kind(compressor_request_kind::NONE)
	{
	}

	~compressor_request() {
		destruct();
	}

	compressor_request(const compressor_request & other)
		: m_kind(compressor_request_kind::NONE)
	{
		switch (other.kind()) {
			case compressor_request_kind::NONE:
				break;
			case compressor_request_kind::READ:
				set_read_request(other.get_read_request());
				break;
			case compressor_request_kind::WRITE:
				set_write_request(other.get_write_request());
				break;
		}
	}

	read_request & set_read_request(const read_request::buffer_t & buffer,
									read_request::file_accessor_t * fileAccessor,
									stream_size_type readOffset,
									read_direction::type readDirection,
									compressor_response * response)
	{
		destruct();
		m_kind = compressor_request_kind::READ;
		return *new (m_payload) read_request(buffer, fileAccessor, readOffset,
											 readDirection, response);
	}

	read_request & set_read_request(const read_request & other) {
		destruct();
		m_kind = compressor_request_kind::READ;
		return *new (m_payload) read_request(other);
	}

	write_request & set_write_request(const write_request::buffer_t & buffer,
									  write_request::file_accessor_t * fileAccessor,
									  temp_file * tempFile,
									  stream_size_type writeOffset,
									  memory_size_type blockItems,
									  stream_size_type blockNumber,
									  compressor_response * response)
	{
		destruct();
		m_kind = compressor_request_kind::WRITE;
		return *new (m_payload) write_request(buffer, fileAccessor, tempFile,
											  writeOffset, blockItems,
											  blockNumber, response);
	}

	write_request & set_write_request(const write_request & other) {
		destruct();
		m_kind = compressor_request_kind::WRITE;
		return *new (m_payload) write_request(other);
	}

	// Precondition: kind() == READ
	read_request & get_read_request() {
		return *reinterpret_cast<read_request *>(m_payload);
	}

	// Precondition: kind() == READ
	const read_request & get_read_request() const {
		return *reinterpret_cast<const read_request *>(m_payload);
	}

	// Precondition: kind() == WRITE
	write_request & get_write_request() {
		return *reinterpret_cast<write_request *>(m_payload);
	}

	// Precondition: kind() == WRITE
	const write_request & get_write_request() const {
		return *reinterpret_cast<const write_request *>(m_payload);
	}

	// Precondition: kind() != NONE
	request_base & get_request_base() {
		return *reinterpret_cast<request_base *>(m_payload);
	}

	// Precondition: kind() != NONE
	const request_base & get_request_base() const {
		return *reinterpret_cast<const request_base *>(m_payload);
	}

	compressor_request_kind::type kind() const {
		return m_kind;
	}

private:
	void destruct() {
		switch (m_kind) {
			case compressor_request_kind::NONE:
				break;
			case compressor_request_kind::READ:
				get_read_request().~read_request();
				break;
			case compressor_request_kind::WRITE:
				get_write_request().~write_request();
				break;
		}
		m_kind = compressor_request_kind::NONE;
	}

	compressor_request_kind::type m_kind;

	enum {
		payload_bytes = sizeof(read_request) < sizeof(write_request) ? sizeof(write_request) : sizeof(read_request),
		payload_items = (payload_bytes + sizeof(size_t) - 1) / sizeof(size_t)
	};
	size_t m_payload[payload_items];
};

} // namespace tpie

#endif // TPIE_COMPRESSED_REQUEST_H
