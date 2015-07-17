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

#ifndef TPIE_COMPRESSED_STREAM_POSITION_H
#define TPIE_COMPRESSED_STREAM_POSITION_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/stream_position.h  Stream position indicator.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/compressed/predeclare.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief  POD object indicating the position of an item in a stream.
///
/// The object returned by stream_position::beginning is equal to any
/// position object returned by a stream that points to the beginning of the
/// stream.
///
/// On the other hand, the object returned by stream_position::end is special:
/// It is never returned by file_stream::get_position; instead, the object
/// tells set_position to perform the equivalent of seek(0, end).
/// The object resulting from the default constructor stream_position()
/// is also special and should never be passed to set_position.
///
/// The offset() method of an ordinary stream_position object indicates
/// the offset of the item in the stream that is pointed to.
/// The values stream_position().offset() and stream_position::end().offset()
/// are not well-defined and should not be relied on to have any particular
/// value; instead, check for equality to respectively stream_position() and
/// stream_position::end().
///
/// The less-than operator defines a total order and is derived from the values
/// returned by offset() with an implementation-defined tiebreaker in case
/// offset() are equal.
///
/// It is guaranteed that any ordinary position is ordered before
/// stream_position::end(), but the ordering of stream_position objects
/// with regards to the special values stream_position() and
/// stream_position::end() may otherwise be subject to change in the future.
///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION NOTES
//
// A stream position is the tuple `(read_offset, offset)`.
//
// For compressed streams, the stream block begins with a header at byte
// position `read_offset`.  After the block header, the items follow.
//
// For uncompressed streams, `read_offset` is zero and the item position
// in the stream is determined solely by `offset`.
//
// Thus, the first item in the stream has the position tuple `(0, 0)`.
///////////////////////////////////////////////////////////////////////////////
class stream_position {
private:
	friend class compressed_stream_base;
	template <typename T>
	friend class file_stream;

	uint64_t m_readOffset;
	uint64_t m_offset;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Convenience constructor returning a pointer to the beginning.
	///////////////////////////////////////////////////////////////////////////
	static stream_position beginning() {
		return stream_position(0, 0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Special-value constructor returning a pointer to the end.
	///////////////////////////////////////////////////////////////////////////
	static stream_position end() {
		return stream_position(
			std::numeric_limits<uint64_t>::max() - 1,
			std::numeric_limits<uint64_t>::max() - 1);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Default constructor resulting in a not-a-stream_position.
	///////////////////////////////////////////////////////////////////////////
	stream_position()
		: m_readOffset(std::numeric_limits<uint64_t>::max())
		, m_offset(std::numeric_limits<uint64_t>::max())
	{
	}

private:
	stream_position(stream_size_type readOffset,
					stream_size_type offset)
		: m_readOffset(readOffset)
		, m_offset(offset)
	{
	}

	stream_size_type read_offset() const {
		return m_readOffset;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  The stream offset of the item pointed to.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type offset() const {
		return m_offset;
	}

private:
	void advance_items(memory_size_type offset) {
		m_offset += offset;
	}

	void advance_item() {
		advance_items(1);
	}

public:
	bool operator==(const stream_position & other) const {
		return m_readOffset == other.m_readOffset
			&& m_offset == other.m_offset;
	}

	bool operator!=(const stream_position & other) const {
		return !(*this == other);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Total ordering of stream_position objects.
	///
	/// See the class documentation for more information.
	///////////////////////////////////////////////////////////////////////////
	bool operator<(const stream_position & other) const {
		return (m_offset != other.m_offset)
			? (m_offset < other.m_offset)
			: (m_readOffset < other.m_readOffset);
	}

	friend std::ostream & operator<<(std::ostream & s, const stream_position & p) {
		return s << "(" << p.read_offset() << "," << p.offset() << ")";
	}
};

} // namespace tpie

#endif // TPIE_COMPRESSED_STREAM_POSITION_H
