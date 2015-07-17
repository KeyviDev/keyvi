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

#ifndef _TPIE_BLOCKS_BLOCK_H
#define _TPIE_BLOCKS_BLOCK_H

#include <tpie/tpie.h>
#include <tpie/array.h>

namespace tpie {

namespace blocks {

typedef array<char> block;

struct block_handle {
	block_handle() : position(0), size(0) {}
	block_handle(stream_size_type position, memory_size_type size) : position(position), size(size) {}

	stream_size_type position;
	memory_size_type size;

	bool operator==(const block_handle & other) const {
		return position == other.position && size == other.size;
	}
};

} // blocks namespace

} // tpie namespace

#endif // _TPIE_BLOCKS_BLOCK_H
