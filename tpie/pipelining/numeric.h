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

#ifndef __TPIE_PIPELINING_NUMERIC_H__
#define __TPIE_PIPELINING_NUMERIC_H__

#include <iostream>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename dest_t>
class linear_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	inline linear_t(TPIE_TRANSFERABLE(dest_t) dest, item_type factor, item_type term) : dest(TPIE_MOVE(dest)), factor(factor), term(term) {
		add_push_destination(this->dest);
		set_name("Linear transform", PRIORITY_INSIGNIFICANT);
	}
	inline void push(const item_type & item) {
		dest.push(item*factor+term);
	}
private:
	dest_t dest;
	item_type factor;
	item_type term;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that transforms the items by applying a linear 
/// function to them.
/// \param factor the factor that items should be multiplied by
/// \param term the term is added after the item is multipled by the factor
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_middle<factory_2<bits::linear_t, T, T> >
linear(T factor, T term) {
	return factory_2<bits::linear_t, T, T>(factor, term);
}

} // namespace pipelining

} // namespace tpie

#endif
