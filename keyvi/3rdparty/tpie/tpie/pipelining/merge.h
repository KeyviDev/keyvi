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

#ifndef __TPIE_PIPELINING_MERGE_H__
#define __TPIE_PIPELINING_MERGE_H__

#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/node.h>

namespace tpie {

namespace pipelining {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \class merge_t
/// \brief Merge a pull pipeline into a push pipeline
///
/// Currently, it is not very well defined what constitutes a merge.
///////////////////////////////////////////////////////////////////////////////

template <typename fact_t>
class merge_t {
public:
	typedef typename fact_t::constructed_type pull_t;

	template <typename dest_t>
	class type : public node {
	public:
		typedef typename push_type<dest_t>::type item_type;

		type(dest_t dest, fact_t fact) : dest(std::move(dest)), with(fact.construct()) {
			add_push_destination(this->dest);
			add_pull_source(with);
		}

		inline void push(const item_type & item) {
			dest.push(item);
			dest.push(with.pull());
		}

		dest_t dest;
		pull_t with;
	};
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// A node that merges a pull pipeline into a push pipeline. It pulls an items
/// for each item pushed to it.
///////////////////////////////////////////////////////////////////////////////
template <typename pull_t>
inline pipe_middle<factory<bits::merge_t<pull_t>::template type, pull_t> >
merge(pullpipe_begin<pull_t> with) {
	return {std::move(with.factory)};
}

} // namespace pipelining

} // namespace tpie

#endif
