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

#ifndef TPIE_COMPRESSED_STREAM_H
#define TPIE_COMPRESSED_STREAM_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/stream.h  Compressed stream public API.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/array.h>
#include <tpie/tpie_assert.h>
#include <tpie/tempname.h>
#include <tpie/file_base_crtp.h>
#include <tpie/file_stream_base.h>
#include <tpie/file_accessor/byte_stream_accessor.h>
#include <tpie/compressed/thread.h>
#include <tpie/compressed/buffer.h>
#include <tpie/compressed/request.h>
#include <tpie/compressed/stream_position.h>
#include <tpie/compressed/direction.h>

namespace tpie {

struct open {
	enum type {
		/** Open a file for reading only. */
		read_only =  00000001,
		/** Open a file for writing only.
		 * Content is truncated. */
		write_only = 00000002,
		/** Neither sequential access nor random access is intended.
		 * Corresponds to POSIX_FADV_NORMAL. */
		access_normal = 00000004,
		/** Random access is intended.
		 * Corresponds to POSIX_FADV_RANDOM and FILE_FLAG_RANDOM_ACCESS (Win32). */
		access_random = 00000010,
		/** Compress some blocks
		 * according to available resources (time, memory). */
		compression_normal = 00000020,
		/** Compress all blocks according to the preferred compression scheme
		 * which can be set using
		 * tpie::the_compressor_thread().set_preferred_compression(). */
		compression_all = 00000040,

		defaults = 0
	};

	friend inline open::type operator|(open::type a, open::type b)
	{ return (open::type) ((int) a | (int) b); }
	friend inline open::type operator&(open::type a, open::type b)
	{ return (open::type) ((int) a & (int) b); }
	friend inline open::type operator^(open::type a, open::type b)
	{ return (open::type) ((int) a ^ (int) b); }
	friend inline open::type operator~(open::type a)
	{ return (open::type) ~(int) a; }

	static type translate(access_type accessType, cache_hint cacheHint, compression_flags compressionFlags) {
		return (type) ((

			(accessType == access_read) ? read_only :
			(accessType == access_write) ? write_only :
			defaults) | (

			(cacheHint == tpie::access_normal) ? access_normal :
			(cacheHint == tpie::access_random) ? access_random :
			defaults) | (

			(compressionFlags == tpie::compression_normal) ? compression_normal :
			(compressionFlags == tpie::compression_all) ? compression_all :
			defaults));
	}

	static cache_hint translate_cache(open::type openFlags) {
		const open::type cacheFlags =
			openFlags & (open::access_normal | open::access_random);

		if (cacheFlags == open::access_normal)
			return tpie::access_normal;
		else if (cacheFlags == open::access_random)
			return tpie::access_random;
		else if (!cacheFlags)
			return tpie::access_sequential;
		else
			throw tpie::stream_exception("Invalid cache flags supplied");
	}

	static compression_flags translate_compression(open::type openFlags) {
		const open::type compressionFlags =
			openFlags & (open::compression_normal | open::compression_all);

		if (compressionFlags == open::compression_normal)
			return tpie::compression_normal;
		else if (compressionFlags == open::compression_all)
			return tpie::compression_all;
		else if (!compressionFlags)
			return tpie::compression_none;
		else
			throw tpie::stream_exception("Invalid compression flags supplied");
	}
};


///////////////////////////////////////////////////////////////////////////////
/// \brief  Base class containing the implementation details that are
/// independent of the item type.
///////////////////////////////////////////////////////////////////////////////
class compressed_stream_base {
public:
	typedef std::shared_ptr<compressor_buffer> buffer_t;

protected:
	struct seek_state {
		enum type {
			none,
			beginning,
			end,
			position
		};
	};

	compressed_stream_base(memory_size_type itemSize,
						   double blockFactor);

	// Non-virtual, protected destructor
	~compressed_stream_base();

	virtual void flush_block(compressor_thread_lock &) = 0;

	virtual void post_open() = 0;

	void open_inner(const std::string & path,
					open::type openFlags,
					memory_size_type userDataSize);

	compressor_thread & compressor() { return the_compressor_thread(); }

public:
	bool is_readable() const throw() { return m_canRead; }

	bool is_writable() const throw() { return m_canWrite; }

	static memory_size_type block_size(double blockFactor) throw ();

	static double calculate_block_factor(memory_size_type blockSize) throw ();

	static memory_size_type block_memory_usage(double blockFactor);

	memory_size_type block_items() const;

	memory_size_type block_size() const;

	template <typename TT>
	void read_user_data(TT & data) {
		if (sizeof(TT) != user_data_size())
			throw stream_exception("Wrong user data size");
		read_user_data(reinterpret_cast<void *>(&data), sizeof(TT));
	}

	memory_size_type read_user_data(void * data, memory_size_type count);

	template <typename TT>
	void write_user_data(const TT & data) {
		if (sizeof(TT) > max_user_data_size())
			throw stream_exception("Wrong user data size");
		write_user_data(reinterpret_cast<const void *>(&data), sizeof(TT));
	}

	void write_user_data(const void * data, memory_size_type count);

	memory_size_type user_data_size() const;

	memory_size_type max_user_data_size() const;

	const std::string & path() const;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening a named stream.
	///
	/// If compressionFlags is compression_none and the file does not already
	/// exist, no compression will be used when writing.
	/// If compressionFlags is compression_normal and the file does not already
	/// exist, compression will be used when writing.
	/// If the file already exists, the compression flags of the existing file
	/// are used instead.
	///////////////////////////////////////////////////////////////////////////
	void open(const std::string & path,
			  access_type accessType,
			  memory_size_type userDataSize = 0,
			  cache_hint cacheHint=access_sequential,
			  compression_flags compressionFlags=compression_none)
	{
		open(path, open::translate(accessType, cacheHint, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening an unnamed temporary stream.
	///////////////////////////////////////////////////////////////////////////
	void open(memory_size_type userDataSize,
			  cache_hint cacheHint=access_sequential,
			  compression_flags compressionFlags=compression_none) {
		open(open::translate(access_read_write, cacheHint, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening a temporary stream.
	///////////////////////////////////////////////////////////////////////////
	void open(temp_file & file,
			  access_type accessType,
			  memory_size_type userDataSize = 0,
			  cache_hint cacheHint=access_sequential,
			  compression_flags compressionFlags=compression_none) {
		open(file, open::translate(accessType, cacheHint, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening a named stream.
	///////////////////////////////////////////////////////////////////////////
	void open(const std::string & path, compression_flags compressionFlags) {
		const memory_size_type userDataSize = 0;
		open(path, open::translate(access_read_write, access_sequential, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening an unnamed temporary stream.
	///////////////////////////////////////////////////////////////////////////
	void open(compression_flags compressionFlags) {
		const memory_size_type userDataSize = 0;
		open(open::translate(access_read_write, access_sequential, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening a temporary stream.
	///////////////////////////////////////////////////////////////////////////
	void open(temp_file & file, compression_flags compressionFlags) {
		const memory_size_type userDataSize = 0;
		open(file, open::translate(access_read_write, access_sequential, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Open and possibly create a stream.
	///
	/// The stream is created if it does not exist and opened for reading
	/// and writing, but this can be changed with open::read_only or
	/// open::write_only; see below.
	///
	/// The flags supplied to openFlags should be a combination of the
	/// following from \c open::type, OR'ed together:
	///
	/// open::read_only
	///     Open for reading only, and fail if the stream does not exist.
	///
	/// open::write_only
	///     Open for writing only, and truncate the stream if it exists.
	///
	/// open::access_normal
	///     By default, POSIX_FADV_SEQUENTIAL is passed to the open syscall
	///     to indicate that the OS should optimize for sequential access;
	///     this flag disables that flag.
	///
	/// open::access_random
	///	    Pass POSIX_FADV_RANDOM to the open syscall to make the OS optimize
	///	    for random access.
	///
	/// open::compression_normal
	///     Create the stream in compression mode if it does not already exist,
	///     and compress written blocks according to available resources (for
	///     instance CPU time and memory).
	///
	/// open::compression_all
	///     Create the stream in compression mode if it does not already exist,
	///     and compress all written blocks using the preferred compression
	///     scheme, which can be set using
	///     tpie::the_compressor_thread().set_preferred_compression().
	///
	/// \param path  The path to the file to open
	/// \param openFlags  A bit-wise combination of the flags; see above.
	/// \param userDataSize  Required user data capacity in stream header.
	///////////////////////////////////////////////////////////////////////////
	void open(const std::string & path, open::type openFlags=open::defaults, memory_size_type userDataSize=0);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Open and create an unnamed temporary stream.
	///
	/// \param openFlags  A bit-wise combination of the flags; see above.
	/// \param userDataSize  Required user data capacity in stream header.
	///////////////////////////////////////////////////////////////////////////
	void open(open::type openFlags=open::defaults, memory_size_type userDataSize=0);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Open and possibly create a temporary stream.
	///
	/// \param file  The temporary file to open
	/// \param openFlags  A bit-wise combination of the flags; see above.
	/// \param userDataSize  Required user data capacity in stream header.
	///////////////////////////////////////////////////////////////////////////
	void open(temp_file & file, open::type openFlags=open::defaults, memory_size_type userDataSize=0);

	void close();

protected:
	void finish_requests(compressor_thread_lock & l);

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: use_compression()
	///
	/// TODO: Should probably investigate when this reports a useful value.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type last_block_read_offset(compressor_thread_lock & l);

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: use_compression()
	///
	/// TODO: Should probably investigate when this reports a useful value.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type current_file_size(compressor_thread_lock & l);

	bool use_compression() { return m_byteStreamAccessor.get_compressed(); }

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Reset cheap read/write counts to zero so that the next
	/// read/write operation will check stream state properly.
	///////////////////////////////////////////////////////////////////////////
	void uncache_read_writes() {
		m_cachedReads = m_cachedWrites = 0;
	}

public:
	bool is_open() const { return m_open; }

	stream_size_type size() const { return m_size; }

	stream_size_type file_size() const { return size(); }

	stream_size_type offset() const {
		switch (m_seekState) {
			case seek_state::none:
				return m_offset;
			case seek_state::beginning:
				return 0;
			case seek_state::end:
				return size();
			case seek_state::position:
				return m_nextPosition.offset();
		}
		tp_assert(false, "offset: Unreachable statement; m_seekState invalid");
		return 0; // suppress compiler warning
	}

protected:
	/** Whether the current block must be written out to disk before being ejected.
	 * Invariants:
	 * If m_bufferDirty is true and use_compression() is true,
	 * block_number() is either m_streamBlocks or m_streamBlocks - 1.
	 * If block_number() is m_streamBlocks, m_bufferDirty is true.
	 */
	bool m_bufferDirty;
	/** Number of items in a logical block. */
	memory_size_type m_blockItems;
	/** Size (in bytes) of a logical (uncompressed) block. */
	memory_size_type m_blockSize;
	/** Whether we are open for reading. */
	bool m_canRead;
	/** Whether we are open for writing. */
	bool m_canWrite;
	/** Whether we are open. */
	bool m_open;
	/** Size of a single item. itemSize * blockItems == blockSize. */
	memory_size_type m_itemSize;
	/** Number of cheap, unchecked reads we can do next. */
	memory_size_type m_cachedReads;
	/** Number of cheap, unchecked writes we can do next. */
	memory_size_type m_cachedWrites;
	/** The anonymous temporary file we have opened (when appropriate). */
	tpie::unique_ptr<temp_file> m_ownedTempFile;
	/** The temporary file we have opened (when appropriate).
	 * When m_ownedTempFile.get() != 0, m_tempFile == m_ownedTempFile.get(). */
	temp_file * m_tempFile;
	/** File accessor. */
	file_accessor::byte_stream_accessor<default_raw_file_accessor> m_byteStreamAccessor;
	/** Number of logical items in the stream. */
	stream_size_type m_size;
	/** Buffer manager for this entire stream. */
	stream_buffers m_buffers;
	/** Buffer holding the items of the block currently being read/written. */
	buffer_t m_buffer;

	/** The number of blocks written to the file.
	 * We must always have (m_streamBlocks+1) * m_blockItems <= m_size. */
	stream_size_type m_streamBlocks;

	/** When use_compression() is true:
	 * Read offset of the last block in the stream.
	 * Necessary to support seeking to the end. */
	stream_size_type m_lastBlockReadOffset;
	stream_size_type m_currentFileSize;

	/** Response from compressor thread; protected by compressor thread mutex. */
	compressor_response m_response;

	/** When use_compression() is true:
	 * Indicates whether m_response is the response to a write request.
	 * Used for knowing where to read next in read/read_back.
	 * */
	bool m_updateReadOffsetFromWrite = false;
	stream_size_type m_lastWriteBlockNumber;

	seek_state::type m_seekState;

	/** Position relating to the currently loaded buffer.
	 * readOffset is only valid during reading.
	 * Invariants:
	 *
	 * If use_compression() == false, readOffset == 0.
	 * If offset == 0, then readOffset == block_item_index() == block_number() == 0.
	 */
	stream_size_type m_readOffset;

	/** Offset of next item to read/write, relative to beginning of stream.
	 * Invariants:
	 *
	 * block_number() in [0, m_streamBlocks]
	 * offset in [0, size]
	 * block_item_index() in [0, m_blockSize)
	 * offset == block_number() * m_blockItems + block_item_index()
	 *
	 * block_item_index() <= offset.
	 *
	 * If block_number() == m_streamBlocks, we are in a block that has not yet
	 * been written to disk.
	 */
	stream_size_type m_offset;

	/** If seekState is `position`, seek to this position before reading/writing. */
	stream_position m_nextPosition;

	stream_size_type m_nextReadOffset;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Implementation helper that closes the stream if a method exits
/// by throwing an exception.
///
/// At every proper exit point from the method, commit() must be called.
/// If the method exits without committing, close() is called.
/// Care should be taken to ensure that the compressor lock is not held when
/// this object is destructed!
///////////////////////////////////////////////////////////////////////////////
class close_on_fail_guard {
public:
	close_on_fail_guard(compressed_stream_base * s)
		: m_committed(false)
		, m_stream(s)
	{
	}

	~close_on_fail_guard() {
		if (!m_committed) m_stream->close();
	}

	void commit() {
		m_committed = true;
	}

private:
	bool m_committed;
	compressed_stream_base * m_stream;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Compressed stream.
///
/// We assume that `T` is trivially copyable and that its copy constructor
/// and assignment operator never throws.
///
/// As a rule of thumb, when a `tpie::stream_exception` is thrown from a method,
/// the stream is left in the state it was in prior to the method call.
/// When a `tpie::exception` is thrown, the stream may have changed.
/// In particular, the stream may have been closed, and it is up to the caller
/// (if the exception is caught) to ensure that the stream is reopened as
/// necessary.
///
/// Several methods claim the `nothrow` guarantee even though the
/// implementation has `throw` statements.
/// In this case, there are two reasons an exception may be thrown:
/// A `tpie::exception` is thrown if some invariant in the stream has been
/// violated, and this is a bug we must fix in the compressed stream.
/// A `tpie::stream_exception` is thrown if the user has violated a
/// precondition (for instance by passing an invalid parameter).
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class file_stream : public compressed_stream_base {
	using compressed_stream_base::seek_state;

public:
	static const file_stream_base::offset_type beginning = file_stream_base::beginning;
	static const file_stream_base::offset_type end = file_stream_base::end;
	static const file_stream_base::offset_type current = file_stream_base::current;

	typedef T item_type;
	typedef file_stream_base::offset_type offset_type;

	file_stream(double blockFactor=1.0)
		: compressed_stream_base(sizeof(T), blockFactor)
		, m_bufferBegin(0)
		, m_bufferEnd(0)
		, m_nextItem(0)
	{
	}

	// This destructor would not need to be virtual if we could use
	// final (non-subclassable) classes, but that is a C++11 feature.
	virtual ~file_stream() {
		try {
			close();
		} catch (std::exception & e) {
			log_error() << "Someone threw an error in file_stream::~file_stream: " << e.what() << std::endl;
			abort();
		}
	}

	static memory_size_type memory_usage(double blockFactor=1.0) {
		// m_buffer is included in m_buffers memory usage
		return sizeof(file_stream)
			+ sizeof(temp_file) // m_ownedTempFile
			+ stream_buffers::memory_usage(block_size(blockFactor)) // m_buffers
			;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  For debugging: Describe the internal stream state in a string.
	///////////////////////////////////////////////////////////////////////////
	void describe(std::ostream & out) {
		if (!this->is_open()) {
			out << "[Closed stream]";
			return;
		}

		out << "[(" << m_byteStreamAccessor.path() << ") item " << offset()
			<< " of " << size();
		out << " (block " << block_number()
			<< " @ byte " << m_readOffset
			<< ", item " << block_item_index()
			<< ")";

		if (use_compression()) {
			out << ", compressed";
		} else {
			out << ", uncompressed";
		}

		switch (m_seekState) {
			case seek_state::none:
				break;
			case seek_state::beginning:
				out << ", seeking to beginning";
				break;
			case seek_state::end:
				out << ", seeking to end";
				break;
			case seek_state::position:
				out << ", seeking to position " << m_nextPosition.offset();
				out << " (block " << block_number(m_nextPosition.offset())
					<< " @ byte " << m_nextPosition.read_offset()
					<< ", item " << block_item_index(m_nextPosition.offset())
					<< ")";
				break;
		}

		if (m_bufferDirty)
			out << " dirty";

		if (m_seekState == seek_state::none) {
			if (can_read()) out << ", can read";
			else out << ", cannot read";
		}

		out << ", " << m_streamBlocks << " blocks";
		if (m_lastBlockReadOffset != std::numeric_limits<stream_size_type>::max())
			out << ", last block at " << m_lastBlockReadOffset;
		if (m_currentFileSize != std::numeric_limits<stream_size_type>::max())
			out << ", current file size " << m_currentFileSize;

		out << ']';
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  For debugging: Describe the internal stream state in a string.
	///////////////////////////////////////////////////////////////////////////
	std::string describe() {
		std::stringstream ss;
		describe(ss);
		return ss.str();
	}

	virtual void post_open() override {
		seek(0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: is_open()
	/// Precondition: offset == 0
	///////////////////////////////////////////////////////////////////////////
	void seek(stream_offset_type offset, offset_type whence=beginning) {
		tp_assert(is_open(), "seek: !is_open");
		uncache_read_writes();
		m_updateReadOffsetFromWrite = false;
		if (!use_compression()) {
			// Handle uncompressed case by delegating to set_position.
			switch (whence) {
			case beginning:
				break;
			case end:
				offset += size();
				break;
			case current:
				offset += this->offset();
				break;
			}
			set_position(stream_position(0, offset));
			return;
		}
		// Otherwise, we are in a compressed stream.
		if (offset != 0) throw stream_exception("Random seeks are not supported");
		switch (whence) {
		case beginning:
			if (m_buffer.get() != 0 && buffer_block_number() == 0) {
				// We are already reading or writing the first block.
				m_nextItem = m_bufferBegin;
				m_offset = m_readOffset = 0;
				m_seekState = seek_state::none;
			} else {
				// We need to load the first block on the next I/O.
				m_seekState = seek_state::beginning;
			}
			return;
		case end:
			if (m_buffer.get() == 0) {
				m_seekState = seek_state::end;
			} else if (m_offset == size()) {
				// no-op
				m_seekState = seek_state::none;
			} else if (// We are in the last block, and it has NOT YET been written to disk, or
					   buffer_block_number() == m_streamBlocks ||
					   // we are in the last block, and it has ALREADY been written to disk.
					   buffer_block_number()+1 == m_streamBlocks)
			{
				// If the last block is full,
				// block_item_index() reports 0 when it should report m_blockItems.
				// Compute blockItemIndex manually to handle this edge case.
				stream_size_type blockItemIndex =
					size() - buffer_block_number() * m_blockItems;
				memory_size_type cast = static_cast<memory_size_type>(blockItemIndex);
				tp_assert(blockItemIndex == cast, "seek: blockItemIndex out of bounds");
				m_nextItem = m_bufferBegin + cast;

				m_offset = size();
				m_seekState = seek_state::none;
			} else {
				m_seekState = seek_state::end;
			}
			return;
		case current:
			return;
		}
		tp_assert(false, "seek: Unknown whence");
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Truncate to given size.
	///
	/// Precondition: compression is disabled or offset is size() or 0.
	/// Blocks to take the compressor lock.
	///////////////////////////////////////////////////////////////////////////
	void truncate(stream_size_type offset) {
		tp_assert(is_open(), "truncate: !is_open");
		uncache_read_writes();
		if (offset == size())
			return;
		else if (offset == 0)
			truncate_zero();
		else if (!use_compression())
			truncate_uncompressed(offset);
		else
			throw stream_exception("Arbitrary truncate is not supported");
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Truncate to given stream position.
	///////////////////////////////////////////////////////////////////////////
	void truncate(const stream_position & pos) {
		tp_assert(is_open(), "truncate: !is_open");
		uncache_read_writes();
		if (pos.offset() == size())
			return;
		else if (pos.offset() == 0)
			truncate_zero();
		else if (!use_compression())
			truncate_uncompressed(pos.offset());
		else
			truncate_compressed(pos);
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Truncate to zero size.
	///////////////////////////////////////////////////////////////////////////
	void truncate_zero() {
		// No need to flush block
		m_buffer.reset();
		m_response.clear_block_info();
		m_updateReadOffsetFromWrite = false;
		compressor_thread_lock l(compressor());
		finish_requests(l);
		get_buffer(l, 0);
		m_size = 0;
		m_streamBlocks = 0;
		m_byteStreamAccessor.truncate(0);

		m_readOffset = 0;
		m_offset = 0;
		m_nextItem = m_bufferBegin;
		m_seekState = seek_state::none;
		uncache_read_writes();
	}

	void truncate_uncompressed(stream_size_type offset) {
		tp_assert(!use_compression(), "truncate_uncompressed called on compressed stream");

		stream_size_type currentOffset = this->offset();
		if (m_buffer.get() != 0
			&& block_number(offset) == buffer_block_number()
			&& buffer_block_number() == m_streamBlocks)
		{
			// We are truncating a final block that has not been written yet.
			m_size = offset;
			if (offset < m_offset) {
				m_offset = offset;
				memory_size_type blockItemIndex =
					static_cast<memory_size_type>(offset - m_streamBlocks * m_blockItems);
				m_nextItem = m_bufferBegin + blockItemIndex;
			}
			m_bufferDirty = true;
			m_seekState = seek_state::none;
			// No need to update m_streamBlocks
		} else {
			// We need to do a truncate on the file accessor.
			// Get rid of the current block first.
			compressor_thread_lock l(compressor());
			if (offset < buffer_block_number() * m_blockItems) {
				// No need to flush current block, since we are truncating it away.
			} else {
				// Changes to the current block may still be visible after the truncate.
				if (m_bufferDirty)
					flush_block(l);
			}
			m_buffer.reset();
			m_bufferDirty = false;
			finish_requests(l);
			m_byteStreamAccessor.truncate(offset);
			m_size = offset;
			m_streamBlocks = (offset + m_blockItems - 1) / m_blockItems;
		}
		seek(std::min(currentOffset, offset));

	}

	void truncate_compressed(const stream_position & pos) {
		tp_assert(use_compression(), "truncate_compressed called on uncompressed stream");

		stream_size_type offset = pos.offset();
		stream_position finalDestination = (offset < this->offset()) ? pos : get_position();

		if (m_buffer.get() == 0 || block_number(offset) != buffer_block_number()) {
			set_position(pos);
			perform_seek();
		}

		// We are truncating into the currently loaded block.
		if (buffer_block_number() < m_streamBlocks) {
			m_streamBlocks = buffer_block_number() + 1;
			m_lastBlockReadOffset = pos.read_offset();
			m_currentFileSize = std::numeric_limits<stream_size_type>::max();
			compressor_thread_lock l(compressor());
			m_response.clear_block_info();
			m_updateReadOffsetFromWrite = false;
		}
		m_size = offset;
		if (offset < m_offset) {
			m_offset = offset;
			memory_size_type blockItemIndex =
				static_cast<memory_size_type>(offset - m_streamBlocks * m_blockItems);
			m_nextItem = m_bufferBegin + blockItemIndex;
		}
		m_bufferDirty = true;

		set_position(finalDestination);
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Store the current stream position such that it may be found
	/// later on.
	///
	/// The stream_position object is violated if the stream is eventually
	/// truncated to before the current position.
	///
	/// The stream_position objects are plain old data, so they may themselves
	/// be written to streams.
	///
	/// Blocks to take the compressor lock.
	///////////////////////////////////////////////////////////////////////////
	stream_position get_position() {
		tp_assert(is_open(), "get_position: !is_open");
		if (!use_compression()) return stream_position(0, offset());
		switch (m_seekState) {
			case seek_state::position:
				// We just set_position, so we can just return what we got.
				return m_nextPosition;
			case seek_state::beginning:
				return stream_position(0, 0);
			case seek_state::none:
				if (buffer_block_number() != m_streamBlocks) {
					if (m_nextItem == m_bufferEnd)
						return stream_position(m_nextReadOffset, m_offset);
					else
						return stream_position(m_readOffset, m_offset);
				}
				// We are in a new block at the end of the stream.
				if (m_nextItem == m_bufferEnd) {
					tp_assert(m_bufferDirty, "At end of buffer, but bufferDirty is false?");
					// Make sure the position we get is not at the end of a block
					compressor_thread_lock lock(compressor());
					flush_block(lock);
					get_buffer(lock, m_streamBlocks);
					m_nextItem = m_bufferBegin;
				}
				break;
			case seek_state::end:
				// Figure out the size of the file below.
				break;
		}

		stream_size_type readOffset;
		stream_size_type blockNumber = block_number(offset());
		compressor_thread_lock l(compressor());
		if (size() % m_blockItems == 0)
			readOffset = current_file_size(l);
		else if (blockNumber == m_streamBlocks)
			readOffset = current_file_size(l);
		else if (blockNumber == m_streamBlocks - 1)
			readOffset = last_block_read_offset(l);
		else {
			tp_assert(false, "get_position: Invalid block_number");
			readOffset = 1111111111111111111ull; // avoid compiler warning
		}
		return stream_position(readOffset, offset());
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Seek to a position that was previously recalled with
	/// \c get_position.
	///////////////////////////////////////////////////////////////////////////
	void set_position(const stream_position & pos) {
		m_updateReadOffsetFromWrite = false;

		// If the code is correct, short circuiting is not necessary;
		// if the code is not correct, short circuiting might mask faults.
		/*
		if (pos == m_position) {
			m_seekState = seek_state::none;
			return;
		}
		*/

		if (pos == stream_position::end()) {
			seek(0, end);
			return;
		}

		if (!use_compression() && pos.read_offset() != 0)
			throw stream_exception("set_position: Invalid position, read_offset != 0");

		if (pos.offset() > size())
			throw stream_exception("set_position: Invalid position, offset > size");

		if (m_buffer.get() != 0
			&& block_number(pos.offset()) == buffer_block_number())
		{
			if (pos.read_offset() != m_readOffset) {
				// We don't always know the read offset of the current block
				// in m_readOffset, so let's assume that
				// pos.read_offset() is correct.
			}

			m_readOffset = pos.read_offset();
			m_offset = pos.offset();
			m_nextItem = m_bufferBegin + block_item_index();
			m_seekState = seek_state::none;
			return;
		}

		m_nextPosition = pos;
		m_seekState = seek_state::position;
		uncache_read_writes();
	}

	///////////////////////////////////////////////////////////////////////////
	/// Reads next item from stream if can_read() == true.
	///
	/// If can_read() == false, throws an end_of_stream_exception.
	///
	/// Blocks to take the compressor lock.
	///
	/// If a stream_exception is thrown, the stream is left in the state it was
	/// in before the call to read().
	///////////////////////////////////////////////////////////////////////////
	const T & read() {
		if (m_cachedReads > 0) {
			--m_cachedReads;
			++m_offset;
			return *m_nextItem++;
		}
		const T & res = peek();
		++m_offset;
		++m_nextItem;
		cache_read_writes();
		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Peeks next item from stream if can_read() == true.
	///
	/// If can_read() == false, throws an end_of_stream_exception.
	///
	/// Blocks to take the compressor lock.
	///
	/// If a stream_exception is thrown, the stream is left in the state it was
	/// in before the call to peek().
	///////////////////////////////////////////////////////////////////////////
	const T & peek() {
		if (m_cachedReads > 0) {
			return *m_nextItem;
		}
		if (m_seekState != seek_state::none) perform_seek();
		if (m_offset == m_size) throw end_of_stream_exception();
		if (m_nextItem == m_bufferEnd) {
			compressor_thread_lock l(compressor());
			if (this->m_bufferDirty)
				flush_block(l);
			// At this point, block_number() == buffer_block_number() + 1
			read_next_block(l, block_number());
		}
		return *m_nextItem;
	}

	void skip() {
		read();
	}

	void skip_back() {
		read_back();
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: is_open().
	///
	/// Reads min(b-a, size()-offset()) items into the range [a, b).
	/// If less than b-a items are read, throws an end_of_stream_exception.
	///////////////////////////////////////////////////////////////////////////
	template <typename IT>
	void read(IT const a, IT const b) {
		for (IT i = a; i != b; ++i) *i = read();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if the next call to read() will succeed or not.
	///////////////////////////////////////////////////////////////////////////
	bool can_read() {
		if (m_cachedReads > 0)
			return true;

		if (!this->m_open)
			return false;

		return offset() < size();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if the next call to read_back() will succeed or not.
	///////////////////////////////////////////////////////////////////////////
	bool can_read_back() {
		if (!this->m_open)
			return false;

		return offset() > 0;
	}

	const T & read_back() {
		if (m_seekState != seek_state::none) {
			if (offset() == 0) throw end_of_stream_exception();
			perform_seek(read_direction::backward);
		}
		if (m_nextItem == m_bufferBegin) {
			if (m_offset == 0) throw end_of_stream_exception();
			uncache_read_writes();
			compressor_thread_lock l(compressor());
			if (this->m_bufferDirty)
				flush_block(l);
			if (use_compression()) {
				read_previous_block(l, block_number() - 1);
			} else {
				read_next_block(l, block_number() - 1);
				m_nextItem = m_bufferEnd;
			}
		}
		++m_cachedReads;
		--m_offset;
		return *--m_nextItem;
	}

	void write(const T & item) {
		if (m_cachedWrites > 0) {
			*m_nextItem++ = item;
			++m_size;
			++m_offset;
			--m_cachedWrites;
			return;
		}

		if (m_seekState != seek_state::none) perform_seek();

		if (!use_compression()) {
			if (m_nextItem == m_bufferEnd) {
				compressor_thread_lock lock(compressor());
				if (m_bufferDirty) {
					m_updateReadOffsetFromWrite = true;
					flush_block(lock);
				}
				if (offset() == size()) {
					get_buffer(lock, m_streamBlocks);
					m_nextItem = m_bufferBegin;
				} else {
					read_next_block(lock, block_number());
				}
			}
			if (offset() == m_size) ++m_size;
			*m_nextItem++ = item;
			this->m_bufferDirty = true;
			++m_offset;
			cache_read_writes();
			return;
		}

		if (m_offset != size())
			throw stream_exception("Non-appending write attempted");

		if (m_nextItem == m_bufferEnd) {
			compressor_thread_lock l(compressor());
			if (m_bufferDirty) {
				m_updateReadOffsetFromWrite = true;
				flush_block(l);
			}
			get_buffer(l, m_streamBlocks);
			m_nextItem = m_bufferBegin;
		}

		*m_nextItem++ = item;
		this->m_bufferDirty = true;
		++m_size;
		++m_offset;

		cache_read_writes();
	}

	template <typename IT>
	void write(IT const a, IT const b) {
		for (IT i = a; i != b; ++i) write(*i);
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: seekState != none
	///
	/// Sets seekState to none.
	///
	/// If anything fails, the stream is closed by a close_on_fail_guard.
	///////////////////////////////////////////////////////////////////////////
	void perform_seek(read_direction::type dir=read_direction::forward) {
		if (!is_open()) throw stream_exception("Stream is not open");
		// This must be initialized before the compressor lock below,
		// so that it is destructed after we free the lock.
		close_on_fail_guard closeOnFail(this);

		tp_assert(!(m_seekState == seek_state::none), "perform_seek when seekState is none");

		uncache_read_writes();

		compressor_thread_lock l(compressor());
		
		m_updateReadOffsetFromWrite = false;

		if (this->m_bufferDirty)
			flush_block(l);

		m_buffer.reset();
		finish_requests(l);

		// Ensure that seek state beginning will take us to a read-only state
		if (m_seekState == seek_state::beginning && size() == 0) {
			m_seekState = seek_state::end;
		}

		// Ensure that seek state position will take us to a read-only state
		if (m_seekState == seek_state::position
			&& m_nextPosition.offset() == size())
		{
			m_seekState = seek_state::end;
		}

		if (m_seekState == seek_state::beginning) {
			// The (seek beginning && size() == 0) case is handled
			// by changing seekState to end.
			// Thus, we know for sure that size() != 0, and so the
			// read_next_block will not yield an end_of_stream_exception.
			tp_assert(!(size() == 0), "Seek beginning when size is zero");
			if (use_compression()) {
				m_nextReadOffset = 0;
			}
			read_next_block(l, 0);
			m_offset = 0;
			tp_assert(m_readOffset == 0, "perform_seek: Bad readOffset after reading first block");
		} else if (m_seekState == seek_state::position) {
			stream_size_type blockNumber = block_number(m_nextPosition.offset());
			memory_size_type blockItemIndex = block_item_index(m_nextPosition.offset());

			// This cannot happen in practice due to the implementation of
			// block_number and block_item_index, but it is an important
			// assumption in the following code.
			tp_assert(!(blockItemIndex >= m_blockItems), "perform_seek: Computed block item index >= blockItems");

			if (dir == read_direction::backward && blockItemIndex == 0 && blockNumber > 0) {
				if (use_compression()) {
					m_readOffset = m_nextPosition.read_offset();
					read_previous_block(l, blockNumber - 1);
					// sets m_nextItem = m_bufferEnd
				} else {
					read_next_block(l, blockNumber - 1);
					m_nextItem = m_bufferEnd;
				}
			} else {
				if (use_compression()) {
					m_nextReadOffset = m_nextPosition.read_offset();
				}
				read_next_block(l, blockNumber);
				m_nextItem = m_bufferBegin + blockItemIndex;
			}

			m_offset = m_nextPosition.offset();
		} else if (m_seekState == seek_state::end) {
			if (m_streamBlocks * m_blockItems == size() && dir == read_direction::forward) {
				// The last block in the stream is full,
				// so we can safely start a new empty one.
				get_buffer(l, m_streamBlocks);
				m_nextItem = m_bufferBegin;
				if (use_compression()) {
					m_readOffset = current_file_size(l);
				} else {
					m_readOffset = 0;
				}
				m_offset = size();
			} else {
				// The last block in the stream is non-full, or we are going to read_back.
				if (m_streamBlocks == 0) {
					// This cannot happen in practice,
					// since we short-circuit seek(end) when streamBlocks == 0.
					throw exception("Attempted seek to end when no blocks have been written");
				}
				memory_size_type blockItemIndex =
					static_cast<memory_size_type>(size() - (m_streamBlocks - 1) * m_blockItems);
				if (use_compression()) {
					m_nextReadOffset = last_block_read_offset(l);
				}
				read_next_block(l, m_streamBlocks - 1);
				m_nextItem = m_bufferBegin + blockItemIndex;
				m_offset = size();
			}
		} else {
			log_debug() << "Unknown seek state " << m_seekState << std::endl;
			tp_assert(false, "perform_seek: Unknown seek state");
		}

		m_seekState = seek_state::none;

		closeOnFail.commit();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Gets buffer for given block and sets bufferBegin and bufferEnd,
	/// and sets bufferDirty to false.
	///////////////////////////////////////////////////////////////////////////
	void get_buffer(compressor_thread_lock & l, stream_size_type blockNumber) {
		uncache_read_writes();
		buffer_t().swap(m_buffer);
		m_buffer = this->m_buffers.get_buffer(l, blockNumber);
		while (m_buffer->is_busy()) compressor().wait_for_request_done(l);
		m_bufferBegin = reinterpret_cast<T *>(m_buffer->get());
		m_bufferEnd = m_bufferBegin + block_items();
		this->m_bufferDirty = false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: m_bufferDirty == true.
	/// Postcondition: m_bufferDirty == false.
	///
	/// Does not get a new block buffer.
	///////////////////////////////////////////////////////////////////////////
	virtual void flush_block(compressor_thread_lock & lock) override {
		uncache_read_writes();
		stream_size_type blockNumber = buffer_block_number();
		stream_size_type writeOffset;
		if (!use_compression()) {
			// Uncompressed case
			writeOffset = blockNumber * m_blockSize;
		} else {
			// Compressed case
			if (blockNumber == m_streamBlocks) {
				// New block; no truncate
				writeOffset = std::numeric_limits<stream_size_type>::max();
			} else if (blockNumber == m_streamBlocks - 1) {
				// Block rewrite; truncate
				writeOffset = last_block_read_offset(lock);
				m_response.clear_block_info();
			} else {
				throw exception("flush_block: blockNumber not at end of stream");
			}
		}

		m_lastBlockReadOffset = std::numeric_limits<stream_size_type>::max();
		m_currentFileSize = std::numeric_limits<stream_size_type>::max();

		if (m_nextItem == NULL) throw exception("m_nextItem is NULL");
		if (m_bufferBegin == NULL) throw exception("m_bufferBegin is NULL");
		memory_size_type blockItems = m_blockItems;
		if (blockItems + blockNumber * m_blockItems > size()) {
			blockItems =
				static_cast<memory_size_type>(size() - blockNumber * m_blockItems);
		}
		m_buffer->set_size(blockItems * sizeof(T));
		m_buffer->set_state(compressor_buffer_state::writing);
		compressor_request r;
		r.set_write_request(m_buffer,
							&m_byteStreamAccessor,
							m_tempFile,
							writeOffset,
							blockItems,
							blockNumber,
							&m_response);
		compressor().request(r);
		m_bufferDirty = false;

		if (m_updateReadOffsetFromWrite) {
			m_lastWriteBlockNumber = blockNumber;
		}

		if (blockNumber == m_streamBlocks) {
			++m_streamBlocks;
		}
	}

	void maybe_update_read_offset(compressor_thread_lock & lock) {
		if (m_updateReadOffsetFromWrite && use_compression()) {
			while (!m_response.done()) {
				m_response.wait(lock);
			}
			if (m_response.has_block_info(m_lastWriteBlockNumber)) {
				m_readOffset = m_response.get_read_offset(m_lastWriteBlockNumber);
				m_nextReadOffset = m_readOffset + m_response.get_block_size(m_lastWriteBlockNumber);
			}
			m_updateReadOffsetFromWrite = false;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Reads next block according to nextReadOffset/nextBlockSize.
	///
	/// Updates m_readOffset with the new read offset.
	///////////////////////////////////////////////////////////////////////////
	void read_next_block(compressor_thread_lock & lock, stream_size_type blockNumber) {
		uncache_read_writes();
		get_buffer(lock, blockNumber);

		maybe_update_read_offset(lock);

		stream_size_type readOffset;
		if (m_buffer->get_state() == compressor_buffer_state::clean) {
			m_readOffset = m_buffer->get_read_offset();
			if (use_compression()) {
				tp_assert(m_readOffset == m_nextReadOffset,
						  "read_next_block: Buffer has wrong read offset");
				m_nextReadOffset = m_readOffset + m_buffer->get_block_size();
			}
		} else {
			if (use_compression()) {
				readOffset = m_nextReadOffset;
			} else {
				stream_size_type itemOffset = blockNumber * m_blockItems;
				readOffset = blockNumber * m_blockSize;
				memory_size_type blockSize =
					std::min(m_blockSize,
							 static_cast<memory_size_type>((size() - itemOffset) * m_itemSize));
				m_buffer->set_size(blockSize);
			}

			read_block(lock, readOffset, read_direction::forward);
			size_t blockItems = m_blockItems;
			if (size() - blockNumber * m_blockItems < blockItems) {
				blockItems = static_cast<size_t>(size() - blockNumber * m_blockItems);
			}
			size_t usableBlockSize = m_buffer->size() / sizeof(T) * sizeof(T);
			size_t expectedBlockSize = blockItems * sizeof(T);
			if (usableBlockSize != expectedBlockSize) {
				log_error() << "Expected " << expectedBlockSize << " (" << blockItems
					<< " items), got " << m_buffer->size() << " (rounded to "
					<< usableBlockSize << ')' << std::endl;
				throw exception("read_next_block: Bad buffer->get_size");
			}

			// Update m_readOffset, m_nextReadOffset
			if (use_compression()) {
				m_readOffset = readOffset;
				m_nextReadOffset = m_response.next_read_offset();
				if (m_readOffset != m_buffer->get_read_offset())
					throw exception("read_next_block: bad get_read_offset");
				if (m_nextReadOffset != m_readOffset + m_buffer->get_block_size())
					throw exception("read_next_block: bad get_block_size");
			} else {
				// Uncompressed case. The following is a no-op:
				//m_readOffset = 0;
				// nextReadOffset is not used.
			}
		}

		m_nextItem = m_bufferBegin;
	}

	void read_previous_block(compressor_thread_lock & lock, stream_size_type blockNumber) {
		uncache_read_writes();
		tp_assert(use_compression(), "read_previous_block: !use_compression");
		get_buffer(lock, blockNumber);

		maybe_update_read_offset(lock);

		if (m_buffer->get_state() == compressor_buffer_state::clean) {
			m_readOffset = m_buffer->get_read_offset();
			m_nextReadOffset = m_readOffset + m_buffer->get_block_size();
		} else {
			read_block(lock, m_readOffset, read_direction::backward);

			// This is backwards since we are reading backwards.
			// Confusing, I know.
			m_nextReadOffset = m_readOffset;
			m_readOffset = m_response.next_read_offset();

			if (m_readOffset != m_buffer->get_read_offset())
				throw exception("Bad buffer get_read_offset");
			if (m_nextReadOffset != m_readOffset + m_buffer->get_block_size())
				throw exception("Bad buffer get_block_size");
		}

		m_nextItem = m_bufferEnd;
	}

	void read_block(compressor_thread_lock & lock,
					stream_size_type readOffset,
					read_direction::type readDirection)
	{
		compressor_request r;
		r.set_read_request(m_buffer,
						   &m_byteStreamAccessor,
						   readOffset,
						   readDirection,
						   &m_response);
		m_buffer->transition_state(compressor_buffer_state::dirty,
								   compressor_buffer_state::reading);
		compressor().request(r);
		while (!m_response.done()) {
			m_response.wait(lock);
		}
	}

	stream_size_type block_number(stream_size_type offset) {
		return offset / m_blockItems;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the number of the block containing the next
	/// read/written item.
	///
	/// Precondition: m_buffer.get() != 0.
	/// Precondition: m_seekState == none
	///////////////////////////////////////////////////////////////////////////
	stream_size_type block_number() {
		return block_number(m_offset);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the number of the block currently loaded into m_buffer.
	///
	/// Precondition: m_buffer.get() != 0.
	/// Precondition: m_seekState == none
	///////////////////////////////////////////////////////////////////////////
	stream_size_type buffer_block_number() {
		stream_size_type blockNumber = block_number();
		if (m_nextItem == m_bufferEnd)
			return blockNumber - 1;
		else
			return blockNumber;
	}

	memory_size_type block_item_index(stream_size_type offset) {
		stream_size_type i = offset % m_blockItems;
		memory_size_type cast = static_cast<memory_size_type>(i);
		tp_assert(!(i != cast), "Block item index out of bounds");
		return cast;
	}

	memory_size_type block_item_index() {
		return block_item_index(m_offset);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute number of cheap, unchecked reads/writes we can do from
	/// now.
	///////////////////////////////////////////////////////////////////////////
	void cache_read_writes() {
		if (m_buffer.get() == 0 || m_seekState != seek_state::none) {
			m_cachedWrites = 0;
			m_cachedReads = 0;
		} else if (offset() == size()) {
			m_cachedWrites = m_bufferDirty ? m_bufferEnd - m_nextItem : 0;
			m_cachedReads = 0;
		} else {
			m_cachedWrites = 0;
			m_cachedReads = m_bufferEnd - m_nextItem;
			if (offset() + m_cachedReads > size()) {
				m_cachedReads =
					static_cast<memory_size_type>(size() - offset());
			}
		}
	}

private:
	/** Only when m_buffer.get() != 0: First item in writable buffer. */
	T * m_bufferBegin;
	/** Only when m_buffer.get() != 0: End of writable buffer. */
	T * m_bufferEnd;

	/** Next item in buffer to read/write. */
	T * m_nextItem;
};

} // namespace tpie

#endif // TPIE_COMPRESSED_STREAM_H
