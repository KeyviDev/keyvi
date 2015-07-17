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
/// \file stream_crtp.h  CRTP base of file::stream and file_stream
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_STREAM_CRTP_H__
#define __TPIE_STREAM_CRTP_H__

#include <tpie/types.h>
#include <tpie/exception.h>
#include <limits>
#include <cassert>

namespace tpie {

template <typename child_t>
class stream_crtp {
public:
	/** Type describing how we should interpret the offset supplied to seek. */
	enum offset_type {
		beginning,
		end,
		current
	};

	/////////////////////////////////////////////////////////////////////////
	/// \brief Moves the logical offset in the stream.
	///
	/// \param offset Where to move the logical offset to.
	/// \param whence Move the offset relative to what.
	/////////////////////////////////////////////////////////////////////////
	inline void seek(stream_offset_type offset, offset_type whence=beginning) throw(stream_exception) {
		assert(self().get_file().is_open());
		if (whence == end)
			offset += self().size();
		else if (whence == current) {
			// are we seeking into the current block?
			if (offset >= 0 || static_cast<stream_size_type>(-offset) <= m_index) {
				stream_size_type new_index = static_cast<stream_offset_type>(offset+m_index);

				if (new_index < self().get_file().block_items()) {
					self().update_vars();
					m_index = static_cast<memory_size_type>(new_index);
					return;
				}
			}

			offset += self().offset();
		}
		if (0 > offset || (stream_size_type)offset > self().size())
			throw io_exception("Tried to seek out of file");
		self().update_vars();
		stream_size_type b = static_cast<stream_size_type>(offset) / self().get_file().block_items();
		m_index = static_cast<memory_size_type>(offset - b* self().get_file().block_items());
		if (b == self().get_block().number) {
			m_nextBlock = std::numeric_limits<stream_size_type>::max();
			m_nextIndex = std::numeric_limits<memory_size_type>::max();
			assert(self().offset() == (stream_size_type)offset);
			return;
		}
		m_nextBlock = b;
		m_nextIndex = m_index;
		m_index = std::numeric_limits<memory_size_type>::max();
		assert(self().offset() == (stream_size_type)offset);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the current offset in the stream.
	///
	/// \returns The current offset in the stream
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type offset() const throw() {
		assert(self().get_file().is_open());
		if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
			return m_index + m_blockStartIndex;
		return m_nextIndex + m_nextBlock * self().get_file().block_items();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Check if we can read an item with read().
	///
	/// This is logically equivalent to:
	/// \code
	/// return offset() < size();
	/// \endcode
	/// but it might be faster.
	///
	/// \returns Whether or not we can read more items from the stream.
	/////////////////////////////////////////////////////////////////////////
	inline bool can_read() const throw() {
		assert(self().get_file().is_open());
		if (m_index < self().get_block().size ) return true;
		return offset() < self().size();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Check if we can read an item with read_back().
	///
	/// \returns Whether or not we can read an item with read_back().
	/////////////////////////////////////////////////////////////////////////
	inline bool can_read_back() const throw() {
		assert(self().get_file().is_open());
		if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
			return m_index > 0 || m_blockStartIndex > 0;
		else
			return m_nextIndex > 0 || m_nextBlock > 0;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of the file measured in items.
	///
	/// \returns The number of items in the file.
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type size() const throw() {
		// XXX update_vars changes internal state in a way that is not visible
		// through the class interface.
		// therefore, a const_cast is warranted.
		const_cast<child_t&>(self()).update_vars();
		return self().get_file().file_size();
	}

protected:
	inline void initialize() {
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Reads several items from the stream.
	///
	/// Implementation note: If your iterator type is efficiently copyable
	/// with std::copy, then this will also read efficiently from the
	/// internal TPIE buffer.
	///
	/// \tparam IT The type of Random Access Iterators used to supply the
	/// items.
	/// \param start Iterator to the first spot to write to.
	/// \param end Iterator past the last spot to write to.
	///
	/// \throws end_of_stream_exception If there are not enough elements in
	/// the stream to fill all the spots between start and end.
	///////////////////////////////////////////////////////////////////////////
	// Since this is called from file<T>::stream and file_stream<T>, we cannot
	// have the implementation in its own object.
	template <typename IT, typename Stream>
	static inline void read_array(Stream & stream, const IT & start, const IT & end) throw(stream_exception) {
		typedef typename Stream::item_type T;
		IT i = start;
		while (i != end) {
			if (stream.m_index >= stream.block_items()) {
				// check to make sure we have enough items in the stream
				stream_size_type offs = stream.offset();
				if (offs >= stream.size()
					|| offs + (end-i) > stream.size()) {

					throw end_of_stream_exception();
				}

				// fetch next block from disk
				stream.update_block();
			}

			T * src = reinterpret_cast<T*>(stream.get_block().data) + stream.m_index;

			// either read the rest of the block or until `end'
			memory_size_type count = std::min(stream.block_items()-stream.m_index, static_cast<memory_size_type>(end-i));

			std::copy(src, src + count, i);

			// advance output iterator
			i += count;

			// advance input position
			stream.m_index += count;
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	/// \brief Write several items to the stream.
	///
	/// Implementation note: If your iterator type is efficiently copyable
	/// with std::copy, then this will also write efficiently into the
	/// internal TPIE buffer.
	///
	/// \tparam IT The type of Random Access Iterators used to supply the
	/// items.
	/// \param start Iterator to the first item to write.
	/// \param end Iterator past the last item to write.
	/////////////////////////////////////////////////////////////////////////////
	// See read_array note above.
	template <typename IT, typename Stream>
	static inline void write_array(Stream & stream, const IT & start, const IT & end) throw(stream_exception) {
		typedef typename Stream::item_type T;
		IT i = start;
		while (i != end) {
			if (stream.m_index >= stream.block_items()) stream.update_block();

			size_t streamRemaining = end - i;
			size_t blockRemaining = stream.block_items()-stream.m_index;

			IT till = (blockRemaining < streamRemaining) ? (i + blockRemaining) : end;

			T * dest = reinterpret_cast<T*>(stream.get_block().data) + stream.m_index;

			std::copy(i, till, dest);

			stream.m_index += till - i;
			stream.write_update();
			i = till;
		}
	}

	///////////////////////////////////////////////////////////////////////
	/// \brief Fetch block from disk as indicated by m_nextBlock, writing old
	/// block to disk if needed.
	/// Update m_block, m_index, m_nextBlock and m_nextIndex. If
	/// m_nextBlock is maxint, use next block is the one numbered
	/// m_block->number+1. m_index is updated with the value of
	/// m_nextIndex.
	///////////////////////////////////////////////////////////////////////
	void update_block();

	/** Item index into the current block, or maxint if we don't have a
	 * block. */
	memory_size_type m_index;
	/** After a cross-block seek: Block index of next block, or maxint if
	 * the current block is good enough OR if we haven't read/written
	 * anything yet. */
	stream_size_type m_nextBlock;
	/** After a cross-block seek: Item index into next block. Otherwise,
	 * maxint as with m_nextBlock. */
	memory_size_type m_nextIndex;
	/** The file-level item index of the first item in the current block.
	 * When m_block is not the null block, this should be equal to
	 * m_block->number * block_items(). */
	stream_size_type m_blockStartIndex;

private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}
};

} // namespace tpie

#endif // __TPIE_STREAM_CRTP_H__
