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

#ifndef __TPIE_PIPELINING_PARALLEL_ALIGNED_ARRAY_H__
#define __TPIE_PIPELINING_PARALLEL_ALIGNED_ARRAY_H__

#include <tpie/types.h>

namespace tpie {

namespace pipelining {

namespace parallel_bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Aligned, uninitialized storage.
///
/// This class provides access to an array of items aligned to any boundary
/// (mostly useful for powers of two).
/// They are not constructed or destructed; only the memory resource is
/// handled.
/// This is used for the nodes that are instantiated once for each
/// parallel thread of pipeline computation. They should be stored in an array
/// aligned to a cache line, to avoid cache lock contention.
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t Align>
class aligned_array {
	// Compute the size of an item with alignment padding (round up to nearest
	// multiple of Align).
	static const size_t aligned_size = (sizeof(T)+Align-1)/Align*Align;

	uint8_t * m_data;
	size_t m_size;

	void dealloc() {
		delete[] m_data;
		m_size = 0;
	}

public:
	aligned_array() : m_data(0), m_size(0) {}

	~aligned_array() { realloc(0); }

	T * get(size_t idx) {
		const size_t addr = (size_t) m_data;

		// Find the aligned base of the array by rounding the pointer up to the
		// nearest multiple of Align.
		const size_t alignedBase = (addr + Align - 1)/Align*Align;

		// Find the address of the element.
		const size_t elmAddress = alignedBase + aligned_size * idx;

		return (T *) elmAddress;
	}

	void realloc(size_t elms) {
		dealloc();
		m_size = elms;
		// The buffer we get is not guaranteed to be aligned to any boundary.
		// Request Align extra bytes to ensure we can find an aligned buffer of
		// size aligned_size*elms.
		m_data = m_size ? new uint8_t[aligned_size * elms + Align] : 0;
	}

	size_t size() const { return m_size; }
};

} // namespace parallel_bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PARALLEL_ALIGNED_ARRAY_H__
