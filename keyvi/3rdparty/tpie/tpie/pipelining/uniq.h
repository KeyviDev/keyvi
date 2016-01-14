// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_UNIQ_H__
#define __TPIE_PIPELINING_UNIQ_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/file_stream.h>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename dest_t>
class count_consecutive_t : public node {
public:
	typedef uint64_t count_t;
	typedef typename push_type<dest_t>::type::first_type item_type;

	inline count_consecutive_t(dest_t dest)
		: dest(std::move(dest))
		, current_count(0)
	{
		add_push_destination(this->dest);
	}

	virtual void end() override {
		node::end();
		flush();
	}

	inline void push(const item_type & item) {
		if (current_count && item == item_buffer) {
			++current_count;
		} else {
			flush();
			item_buffer = item;
			current_count = 1;
		}
	}
private:
	inline void flush() {
		if (!current_count) return;
		dest.push(std::make_pair(item_buffer, current_count));
		current_count = 0;
	}
	dest_t dest;
	item_type item_buffer;
	count_t current_count;
};

class any_type {
public:
	template <typename T>
	inline any_type(const T &) {}
	template <typename T>
	inline any_type & operator=(const T &) {return *this;}
};

template <typename dest_t>
class extract_first_t : public node {
public:
	typedef std::pair<typename push_type<dest_t>::type, any_type> item_type;

	inline extract_first_t(dest_t dest) : dest(std::move(dest)) {
		add_push_destination(this->dest);
	}

	inline void push(const item_type & item) {
		dest.push(item.first);
	}
private:
	dest_t dest;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that removes duplicate items and create a phase
/// boundary
///////////////////////////////////////////////////////////////////////////////
inline pipe_middle<bits::pair_factory<factory<bits::count_consecutive_t>, factory<bits::extract_first_t> > >
pipeuniq() {
	return bits::pair_factory<factory<bits::count_consecutive_t>, factory<bits::extract_first_t> >
		(factory<bits::count_consecutive_t>(), factory<bits::extract_first_t>());
}

}

}

#endif
