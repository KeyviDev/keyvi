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

#ifndef TPIE_COMPRESSED_BUFFER_H
#define TPIE_COMPRESSED_BUFFER_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/buffer.h  Buffers for compressed streams.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/array.h>
#include <tpie/tpie_assert.h>
#include <tpie/compressed/thread.h>
#include <map>
#include <memory>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Used by tpie::init to initialize stream buffer pool.
///////////////////////////////////////////////////////////////////////////////
void init_stream_buffer_pool();

///////////////////////////////////////////////////////////////////////////////
/// \brief  Used by tpie::finish to free stream buffer pool.
///////////////////////////////////////////////////////////////////////////////
void finish_stream_buffer_pool();

///////////////////////////////////////////////////////////////////////////////
/// \brief  The different states of a compressor buffer.
///
/// dirty: The buffer is different from the contents on the disk.
/// Only stream may read and write.
/// This is the initial state of a freshly allocated buffer.
///
/// writing: The buffer will soon be written to disk.
/// Stream and thread may only read.
///
/// reading: The buffer will soon change to reflect the contents on the disk.
/// Only thread may read and write.
///
/// clean: The buffer is equal to the contents on the disk.
/// Only stream may read and write.
///
/// dirty->writing/reading: When the buffer is sent from stream to thread.
/// writing/reading->clean: When the thread is done.
/// clean->dirty: When the stream has changed the contents of the buffer.
///////////////////////////////////////////////////////////////////////////////
struct compressor_buffer_state {
	enum type {
		/** The buffer is different from the contents on the disk.
		 * Only stream may read and write. */
		dirty,
		/** The buffer will soon be written to disk.
		 * Stream and thread may only read. */
		writing,
		/** The buffer will soon change to reflect the contents on the disk.
		 * Only thread may read and write. */
		reading,
		/** The buffer is equal to the contents on the disk.
		 * Only stream may read and write. */
		clean
	};
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  A buffer for elements belonging to a specific stream block.
///////////////////////////////////////////////////////////////////////////////
class compressor_buffer {
private:
	typedef array<char> storage_t;

	storage_t m_storage;
	memory_size_type m_size;
	compressor_buffer_state::type m_state;
	stream_size_type m_readOffset;
	memory_size_type m_blockSize;

public:
	compressor_buffer(memory_size_type capacity)
		: m_storage(capacity)
		, m_size(0)
		, m_state(compressor_buffer_state::dirty)
		, m_readOffset(1111111111111111111ull)
		, m_blockSize(std::numeric_limits<memory_size_type>::max())
	{
	}

	compressor_buffer_state::type get_state() const {
		return m_state;
	}

	void set_state(compressor_buffer_state::type to) {
		m_state = to;
	}

	void transition_state(compressor_buffer_state::type from,
						  compressor_buffer_state::type to)
	{
		tp_assert(!(m_state != from), "compressor_buffer: invalid state transition");
		unused(from);
		set_state(to);
	}

	bool is_busy() {
		switch (m_state) {
			case compressor_buffer_state::dirty: return false;
			case compressor_buffer_state::writing: return true;
			case compressor_buffer_state::reading: return true;
			case compressor_buffer_state::clean: return false;
		}
		tp_assert(false, "is_busy: compressor_buffer in invalid state");
		return false; // suppress compiler warning
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief  Get pointer to buffer storage.
	///////////////////////////////////////////////////////////////////////////////
	char * get() {
		return m_storage.get();
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief  Get pointer to buffer storage.
	///////////////////////////////////////////////////////////////////////////////
	const char * get() const {
		return m_storage.get();
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief  Get number of bytes used to store items.
	///////////////////////////////////////////////////////////////////////////////
	memory_size_type size() const {
		return m_size;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief  Get maximal byte size of buffer.
	///////////////////////////////////////////////////////////////////////////////
	memory_size_type capacity() const {
		return m_storage.size();
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief  Set number of bytes used to store items.
	///////////////////////////////////////////////////////////////////////////////
	void set_size(memory_size_type size) {
		m_size = size;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief  Resize internal buffer, clearing all elements.
	///////////////////////////////////////////////////////////////////////////////
	void set_capacity(memory_size_type capacity) {
		m_storage.resize(capacity);
		m_size = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief  Return buffer to a newly constructed state.
	///////////////////////////////////////////////////////////////////////////////
	void reset() {
		m_state = compressor_buffer_state::dirty;
		m_size = 0;
		m_readOffset = 1111111111111111111ull;
		m_blockSize = std::numeric_limits<memory_size_type>::max();
	}

	memory_size_type get_block_size() { return m_blockSize; }
	stream_size_type get_read_offset() { return m_readOffset; }
	void set_block_size(memory_size_type s) { m_blockSize = s; }
	void set_read_offset(stream_size_type s) { m_readOffset = s; }
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Pool of shared buffers.
///
/// Streams need block buffers to store data read from disk and data to be
/// written to disk.
///
/// Each stream owns a number of buffers which it may allocate after open()
/// and must deallocate on close(). Currently, each stream has just one own
/// buffer.
///
/// In addition, on program startup we allocate a number of shared buffers
/// on program startup which any stream may use for additional efficiency.
/// Currently, we allocate two such shared buffers.
///
/// The stream_buffer_pool class is responsible for allocating and deallocating
/// both the streams' own buffers and the shared buffers.
///
/// The stream_buffers class is responsible for making sure it only requests
/// as many own buffers as a stream is allowed to.
///////////////////////////////////////////////////////////////////////////////
class stream_buffer_pool {
public:
	typedef std::shared_ptr<compressor_buffer> buffer_t;

	stream_buffer_pool();
	~stream_buffer_pool();

	buffer_t allocate_own_buffer();
	void release_own_buffer(buffer_t &);

	bool can_take_shared_buffer();
	buffer_t take_shared_buffer();
	void release_shared_buffer(buffer_t &);

private:
	class impl;
	impl * pimpl;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Get the stream buffer pool singleton.
///
/// You must have initialized the stream buffer pool with \c tpie::init
/// beforehand, or you will get a null pointer or an assertion error!
///////////////////////////////////////////////////////////////////////////////
stream_buffer_pool & the_stream_buffer_pool();

///////////////////////////////////////////////////////////////////////////////
/// \brief  Buffer manager for a single stream.
///
/// Buffers are provided via \c get_buffer.  You should call \c clean before
/// destroying.
///////////////////////////////////////////////////////////////////////////////
class stream_buffers {
public:
	typedef std::shared_ptr<compressor_buffer> buffer_t;

	const static memory_size_type OWN_BUFFERS = 1;

	stream_buffers(memory_size_type blockSize)
		: m_blockSize(blockSize)
		, m_ownBuffers(0)
	{
	}

	~stream_buffers() {
		if (!empty()) {
			log_debug() << "ERROR: ~stream_buffers: not empty!" << std::endl;
		}
	}

	static memory_size_type memory_usage(memory_size_type blockSize) {
		return blockSize;
	}

	buffer_t get_buffer(compressor_thread_lock & lock, stream_size_type blockNumber) {
		if (!(m_ownBuffers < OWN_BUFFERS || can_take_shared_buffer())) {
			// First, search for the buffer in the map.
			buffermapit target = m_buffers.find(blockNumber);
			if (target != m_buffers.end()) return target->second;

			// If not found, wait for a free buffer to become available.
			buffer_t b;
			while (true) {
				buffermapit i = m_buffers.begin();
				while (i != m_buffers.end() && !i->second.unique()) ++i;
				if (i == m_buffers.end()) {
					compressor().wait_for_request_done(lock);
					continue;
				} else {
					b.swap(i->second);
					m_buffers.erase(i);
					break;
				}
			}

			b->reset();
			m_buffers.insert(std::make_pair(blockNumber, b));
			clean();
			return b;
		} else {
			// First, search for the buffer in the map.
			std::pair<buffermapit, bool> res
				= m_buffers.insert(std::make_pair(blockNumber, buffer_t()));
			buffermapit & target = res.first;
			bool & inserted = res.second;
			if (!inserted) return target->second;

			// If not found, find a free buffer and place it in target->second.

			// We have now placed an empty shared_ptr in m_buffers
			// (an "insertion point"), and nobody is allowed to call clean()
			// on us before we insert something in that point.

			// target->second is the only buffer in the map with use_count() == 0.
			// If a buffer in the map has use_count() == 1 (that is, unique() == true),
			// that means only our map (and nobody else) refers to the buffer,
			// so it is free to be reused.
			buffermapit i = m_buffers.begin();
			while (i != m_buffers.end() && !i->second.unique()) ++i;

			if (i == m_buffers.end()) {
				// No free found: allocate new buffer.
				if (m_ownBuffers < OWN_BUFFERS) {
					target->second = allocate_own_buffer();
				} else if (can_take_shared_buffer()) {
					target->second = take_shared_buffer();
				} else {
					// This is a contradition of the very first check
					// in the beginning of the method.
					tp_assert(false, "get_buffer: Could not get a new buffer "
									 "contrary to previous checks");
				}
			} else {
				// Free found: reuse buffer.
				target->second.swap(i->second);
				m_buffers.erase(i);
			}

			// Bump use count before cleaning.
			buffer_t result = target->second;
			clean();
			result->reset();
			return result;
		}
	}

	bool empty() const {
		return m_buffers.empty();
	}

	void clean() {
		buffermapit i = m_buffers.begin();
		while (i != m_buffers.end()) {
			buffermapit j = i++;
			if (j->second.get() == 0) {
				// This item in the map represents an insertion point in get_buffer,
				// but in that case, get_buffer has the compressor lock,
				// and it shouldn't wait before inserting something
				throw exception("stream_buffers: j->second.get() == 0");
			} else if (j->second.unique()) {
				if (shared_buffers() > 0) {
					release_shared_buffer(j->second);
				} else {
					release_own_buffer(j->second);
				}
				m_buffers.erase(j);
			}
		}
	}

private:
	memory_size_type own_buffers() {
		return m_ownBuffers;
	}

	memory_size_type shared_buffers() {
		return m_buffers.size() - m_ownBuffers;
	}

	void release_shared_buffer(buffer_t & b) {
		the_stream_buffer_pool().release_shared_buffer(b);
	}

	void release_own_buffer(buffer_t & b) {
		--m_ownBuffers;
		the_stream_buffer_pool().release_own_buffer(b);
	}

	bool can_take_shared_buffer() {
		return the_stream_buffer_pool().can_take_shared_buffer();
	}

	buffer_t take_shared_buffer() {
		return the_stream_buffer_pool().take_shared_buffer();
	}

	buffer_t allocate_own_buffer() {
		++m_ownBuffers;
		return the_stream_buffer_pool().allocate_own_buffer();
	}

	compressor_thread & compressor() {
		return the_compressor_thread();
	}

	memory_size_type block_size() const {
		return m_blockSize;
	}

	memory_size_type m_blockSize;

	typedef std::map<stream_size_type, buffer_t> buffermap_t;
	typedef buffermap_t::iterator buffermapit;
	buffermap_t m_buffers;

	/** Number of own buffers currently allocated inside m_buffers. */
	memory_size_type m_ownBuffers;
};

} // namespace tpie

#endif // TPIE_COMPRESSED_BUFFER_H
