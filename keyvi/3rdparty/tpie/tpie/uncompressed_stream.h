// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2011, The TPIE development team
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
#ifndef TPIE_UNCOMPRESSED_STREAM_H
#define TPIE_UNCOMPRESSED_STREAM_H
#include <tpie/tempname.h>
#include <tpie/file.h>
#include <tpie/memory.h>
#include <tpie/file_stream_base.h>
///////////////////////////////////////////////////////////////////////////////
/// \file uncompressed_stream.h
/// \brief Simple class acting both as a tpie::file and a
/// tpie::file::stream.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {


///////////////////////////////////////////////////////////////////////////////
/// \brief Simple class acting both as \ref file and a file::stream.
///
/// A uncompressed_stream basically supports every operation a \ref file or a
/// file::stream supports. This is used to access a file I/O-efficiently, and
/// is the direct replacement of the old ami::stream.
///
/// \tparam T The type of items stored in the stream.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class uncompressed_stream: public file_stream_base {
public:
	/** The type of the items stored in the stream */
	typedef T item_type;

	/////////////////////////////////////////////////////////////////////////
	/// \brief Construct a new uncompressed_stream.
	/// 
	/// \copydetails tpie::file::file(double blockFactor, file_accessor::file_accessor * fileAccessor)
	/////////////////////////////////////////////////////////////////////////
	uncompressed_stream(double blockFactor=1.0,
					   file_accessor::file_accessor * fileAccessor=NULL):
		file_stream_base(sizeof(item_type), blockFactor, fileAccessor) {};

	
	/////////////////////////////////////////////////////////////////////////
	/// \copybrief file<T>::stream::write(const item_type & item)
	/// \copydetails file<T>::stream::write(const item_type & item)
	/// \sa file<T>::stream::write(const item_type & item)
	/////////////////////////////////////////////////////////////////////////
	inline void write(const item_type & item) throw(stream_exception) {
		assert(m_open);
#ifndef NDEBUG
		if (!is_writable())
			throw io_exception("Cannot write to read only stream");
#endif
		if (m_index >= m_blockItems) update_block();
		reinterpret_cast<item_type*>(m_block.data)[m_index++] = item;
		write_update();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copybrief stream_crtp::write_array
	/// \copydetails stream_crtp::write_array
	/// \sa file<T>::stream::write(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	template <typename IT>
	inline void write(const IT & start, const IT & end) throw(stream_exception) {
		assert(m_open);
		write_array(*this, start, end);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copybrief file<T>::stream::read()
	/// \copydetails file<T>::stream::read()
	/// \sa file<T>::stream::read()
	/////////////////////////////////////////////////////////////////////////
	inline const item_type & read() throw(stream_exception) {
		assert(m_open);
		const item_type & x = peek();
		++m_index;
		return x;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copybrief stream_crtp::read_array
	/// \copydetails stream_crtp::read_array
	/// \sa file<T>::stream::read(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	template <typename IT>
	inline void read(const IT & start, const IT & end) throw(stream_exception) {
		assert(m_open);
		read_array(*this, start, end);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copybrief file<T>::stream::read_back()
	/// \copydetails file<T>::stream::read_back()
	/// \sa file<T>::stream::read_back()
	/////////////////////////////////////////////////////////////////////////
	inline const item_type & read_back() throw(stream_exception) {
		assert(m_open);
		skip_back();
		return peek();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get next item from stream without advancing the position.
	///////////////////////////////////////////////////////////////////////////
	const item_type & peek() {
		assert(m_open);
		if (m_index >= m_block.size) {
			update_block();
			if (offset() >= size()) {
				throw end_of_stream_exception();
			}
		}
		return reinterpret_cast<item_type*>(m_block.data)[m_index];
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Advance the stream position to the next item.
	///////////////////////////////////////////////////////////////////////////
	void skip() {
		seek(1, current);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Advance the stream position to the previous item.
	///////////////////////////////////////////////////////////////////////////
	void skip_back() {
		seek(-1, current);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the amount of memory used by a single uncompressed_stream.
	///
	/// \param blockFactor The block factor you pass to open.
	/// \param includeDefaultFileAccessor Unless you are supplying your own
	/// file accessor to open, leave this to be true.
	/// \returns The amount of memory maximally used by the count file_streams.
	///////////////////////////////////////////////////////////////////////////
	inline static memory_size_type memory_usage(
		float blockFactor=1.0,
		bool includeDefaultFileAccessor=true) throw() {
		// TODO
		memory_size_type x = sizeof(uncompressed_stream);
		x += block_memory_usage(blockFactor); // allocated in constructor
		if (includeDefaultFileAccessor)
			x += default_file_accessor::memory_usage();
		return x;
	}

	void swap(uncompressed_stream<T> & other) {
		file_stream_base::swap(other);
	}

	friend struct stream_item_array_operations;
};

} // namespace tpie

namespace std {

///////////////////////////////////////////////////////////////////////////////
/// \brief Enable std::swapping two tpie::file_streams.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
void swap(tpie::uncompressed_stream<T> & a, tpie::uncompressed_stream<T> & b) {
	a.swap(b);
}

} // namespace std

#endif // TPIE_UNCOMPRESSED_STREAM_H
