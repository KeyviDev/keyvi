// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2015 The TPIE development team
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

#ifndef __TPIE_PIPELINING_CHUNKER_H__
#define __TPIE_PIPELINING_CHUNKER_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/node_name.h>

namespace tpie {
namespace pipelining {
namespace bits {

template <typename dest_t>
class chunker_t: public node {
public:
	typedef typename push_type<dest_t>::type vector_type;
	typedef typename vector_type::value_type item_type;
private:
	const size_t maxSize;
	vector_type items;
	dest_t dest;
public:
	chunker_t(dest_t dest, size_t maxSize)
		: maxSize(maxSize)
		, dest(std::move(dest))
	{
			set_minimum_memory(sizeof(item_type) * maxSize);
			set_name("Chunker", PRIORITY_INSIGNIFICANT);
	}
	
	void flush() {
		dest.push(items);
		items.clear();
	}
	
	void begin() override {
		items.reserve(maxSize);
	}
	
	void push(const item_type & item) {
		if (items.size() == maxSize) flush();
		items.push_back(item);
	}

	void end() override {
		if (!items.empty()) flush();
		free_structure_memory(items);
	}
};

} //namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that gathers elements into a vector of some size.
/// \param maxSize the maximum size of the vector
///////////////////////////////////////////////////////////////////////////////
typedef pipe_middle<factory<bits::chunker_t, size_t> > chunker;

} //namespace pipelining
} //namespace terrastream

#endif //__TPIE_PIPELINING_CHUNKER_H__
