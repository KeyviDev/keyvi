// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2014, The TPIE development team
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
/// \file block_collection Class to handle blocks of varying size on disk
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_BLOCKS_FREESPACE_COLLECTION_H
#define _TPIE_BLOCKS_FREESPACE_COLLECTION_H

#include <tpie/tpie.h>
#include <tpie/tpie_assert.h>
#include <tpie/blocks/block.h>
#include <tpie/stack.h>
#include <limits>

namespace tpie {

namespace blocks {

namespace bits {

class freespace_collection {
private:
	tpie::stack<block_handle> m_free;
	stream_size_type m_end;
	memory_size_type m_blockSize;
public:
	freespace_collection(const std::string & path, const memory_size_type blockSize) 
	: m_free(path)
	, m_end(0)
	, m_blockSize(blockSize)
	{
		if(m_free.size() > 0) { // when closed the top element of the stacked is an infinitely large block representing the end of the file
			m_end = m_free.pop().position;
		}
	}

	~freespace_collection() {
		// when closed the top element of the stacked is an infinitely large block representing the end of the file
		m_free.push(block_handle(m_end, std::numeric_limits<stream_size_type>::max()));
	}

	void free(block_handle handle) {
		tp_assert(handle.size == m_blockSize, "the size of the given handle is incorrect");
		m_free.push(handle);
	}

	block_handle alloc() {
		if(!m_free.empty())
			return m_free.pop();

		block_handle h(m_end, m_blockSize);
		m_end += m_blockSize;
		return h;
	}

	stream_size_type size() {
		return m_end;
	}
};

} // bits namespace

} // blocks namespace

} // tpie namespace

#endif // _TPIE_BLOCKS_FREESPACE_COLLECTION_H
