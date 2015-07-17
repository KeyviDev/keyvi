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
/// \file file_base_crtp.h  CRTP base of file_base
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_FILE_BASE_CRTP_H__
#define __TPIE_FILE_BASE_CRTP_H__

#include <tpie/types.h>
#include <tpie/tpie.h>
#include <tpie/exception.h>
#include <tpie/memory.h>
#include <tpie/access_type.h>
#include <tpie/cache_hint.h>
#include <tpie/stream_header.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/tempname.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class of classes that access files.
///
/// Inheriting classes may wish to override open_inner() and close(), e.g. to
/// initialize and deinitialize block buffers. open_inner() in the inheriting
/// class will not be called twice in a row without an intervening call to
/// close(). The default implementation of open_inner() passes the open on to
/// the file accessor and sets some attributes.
///////////////////////////////////////////////////////////////////////////////
template <typename child_t>
class file_base_crtp {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// Check if we can read from the file.
	///
	/// \returns True if we can read from the file.
	////////////////////////////////////////////////////////////////////////////////
	bool is_readable() const throw() {
		return m_canRead;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// Check if we can write to the file.
	///
	/// \returns True if we can write to the file.
	////////////////////////////////////////////////////////////////////////////////
	bool is_writable() const throw() {
		return m_canWrite;
	}


	////////////////////////////////////////////////////////////////////////////////
	/// Calculate the block size in bytes used by a stream.
	///
	/// We have block_size(calculate_block_factor(b)) ~= b.
	///
	/// \param blockFactor Factor of the global block size to use.
	/// \returns Size in bytes.
	////////////////////////////////////////////////////////////////////////////////
	static inline memory_size_type block_size(double blockFactor) throw () {
		return static_cast<memory_size_type>(get_block_size() * blockFactor);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Find the block factor that would result in the given block size
	/// measured in bytes.
	///
	/// We have calculate_block_factor(block_size(f)) ~= f.
	///
	/// \param blockSize The sought block size.
	/// \returns The block factor needed to achieve this block size.
	///////////////////////////////////////////////////////////////////////////
	static inline double calculate_block_factor(memory_size_type blockSize) throw () {
		return (double)blockSize / (double)block_size(1.0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Amount of memory used by a single block given the block factor.
	///////////////////////////////////////////////////////////////////////////
	static inline memory_size_type block_memory_usage(double blockFactor) {
		return block_size(blockFactor);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the number of items per block.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type block_items() const {
		return m_blockItems;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of a block in bytes.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type block_size() const {
		return m_blockSize;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Read the user data associated with the file.
	///
	/// \param data Where to store the user data.
	/// \tparam TT The type of user data. sizeof(TT) must be less than or equal
	/// to the maximum user data size of the stream. TT must be trivially
	/// copyable.
	///////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void read_user_data(TT & data) throw(stream_exception) {
		assert(m_open);
		if (sizeof(TT) != user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->read_user_data(reinterpret_cast<void*>(&data), sizeof(TT));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Read variable length user data associated with the file.
	///
	/// \param data The buffer in which to write data.
	/// \param count The size of the buffer.
	/// \returns Number of bytes of user data actually read.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type read_user_data(void * data, memory_size_type count) {
		assert(m_open);
		return m_fileAccessor->read_user_data(data, count);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Write user data to the stream.
	///
	/// \param data The user data to store in the stream.
	/// \tparam TT The type of user data. sizeof(TT) must be less than or equal
	/// to the maximum user data size of the stream. TT must be trivially
	/// copyable.
	///////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void write_user_data(const TT & data) throw(stream_exception) {
		assert(m_open);
		if (sizeof(TT) > max_user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->write_user_data(reinterpret_cast<const void*>(&data), sizeof(TT));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Write variable length user data associated with the file.
	///
	/// Throws a stream_exception if the size of the user data exceeds the
	/// maximum user data size of the stream.
	///
	/// \param data The buffer from which to read data.
	/// \param count The size of the user data.
	///////////////////////////////////////////////////////////////////////////
	void write_user_data(const void * data, memory_size_type count) {
		assert(m_open);
		m_fileAccessor->write_user_data(data, count);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get current user data size.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type user_data_size() const {
		assert(m_open);
		return m_fileAccessor->user_data_size();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get maximum user data size.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type max_user_data_size() const {
		assert(m_open);
		return m_fileAccessor->max_user_data_size();
	}


	/////////////////////////////////////////////////////////////////////////
	/// \brief The path of the file opened or the empty string.
	///
	/// \returns The path of the currently opened file.
	/////////////////////////////////////////////////////////////////////////
	inline const std::string & path() const throw() {
		assert(m_open);
		return m_fileAccessor->path();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Open a file.
	///
	/// \param path The path of the file to open.
	/// \param accessType The mode of operation.
	/// \param userDataSize The size of the user data we want to store in the
	/// file.
	/////////////////////////////////////////////////////////////////////////
	inline void open(const std::string & path,
					 access_type accessType=access_read_write,
					 memory_size_type userDataSize=0,
					 cache_hint cacheHint=access_sequential) throw (stream_exception) {
		self().close();
		self().open_inner(path, accessType, userDataSize, cacheHint);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Open an anonymous temporary file. The temporary file is deleted
	/// when this file is closed.
	/////////////////////////////////////////////////////////////////////////
	inline void open(memory_size_type userDataSize=0,
					 cache_hint cacheHint=access_sequential) throw (stream_exception) {
		self().close();
		m_ownedTempFile.reset(tpie_new<temp_file>());
		m_tempFile=m_ownedTempFile.get();
		self().open_inner(m_tempFile->path(), access_read_write, userDataSize, cacheHint);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open a temporary file. The temporary file is not deleted when
	/// this file is closed, so several tpie::file objects may use the same
	/// temporary file consecutively.
	///////////////////////////////////////////////////////////////////////////
	inline void open(temp_file & file,
					 access_type accessType=access_read_write,
					 memory_size_type userDataSize=0,
					 cache_hint cacheHint=access_sequential) throw (stream_exception) {
		self().close();
		m_tempFile=&file;
		self().open_inner(m_tempFile->path(), accessType, userDataSize, cacheHint);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Close the file.
	///
	/// Note all streams into the file must be freed before you call close.
	/////////////////////////////////////////////////////////////////////////
	inline void close() throw(stream_exception) {
		if (m_open) m_fileAccessor->close();
		m_open = false;
		m_tempFile = NULL;
		m_ownedTempFile.reset();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if file is open.
	///////////////////////////////////////////////////////////////////////////
	inline bool is_open() const {
		return m_open;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of the file measured in items.
	/// If there are streams of this file that have extended the stream length
	/// but have not yet flushed these writes, we might report an incorrect
	/// size.
	///
	/// \returns The number of items in the file.
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type file_size() const throw() {
		return m_size;
	}

protected:
	inline void open_inner(const std::string & path,
						   access_type accessType,
						   memory_size_type userDataSize,
						   cache_hint cacheHint) throw(stream_exception) {
		m_canRead = accessType == access_read || accessType == access_read_write;
		m_canWrite = accessType == access_write || accessType == access_read_write;
		const bool preferCompression = false;
		m_fileAccessor->open(path, m_canRead, m_canWrite, m_itemSize,
							 m_blockSize, userDataSize, cacheHint,
							 preferCompression);
		if (m_fileAccessor->get_compressed()) {
			m_fileAccessor->close();
			throw stream_exception("Tried to open compressed stream as non-compressed");
		}
		m_size = m_fileAccessor->size();
		m_open = true;
	}


	file_base_crtp(memory_size_type itemSize, double blockFactor,
				   file_accessor::file_accessor * fileAccessor);


	template <typename BT>
	void read_block(BT & b, stream_size_type block);
	void get_block_check(stream_size_type block);

	memory_size_type m_blockItems;
	memory_size_type m_blockSize;
	bool m_canRead;
	bool m_canWrite;
	bool m_open;
	memory_size_type m_itemSize;
	file_accessor::file_accessor * m_fileAccessor;
	tpie::auto_ptr<temp_file> m_ownedTempFile;
	temp_file * m_tempFile;
	stream_size_type m_size;

private:
	child_t & self() {return *static_cast<child_t *>(this);}
	const child_t & self() const {return *static_cast<const child_t *>(this);}
};

} // namespace tpie

#endif // __TPIE_FILE_BASE_CRTP_H__
