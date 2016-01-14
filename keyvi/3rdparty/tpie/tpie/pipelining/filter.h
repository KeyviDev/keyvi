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

#ifndef __TPIE_PIPELINING_FILTER_H__
#define __TPIE_PIPELINING_FILTER_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/node_name.h>

namespace tpie {
namespace pipelining {
namespace bits {

template <typename F>
class filter_t {
public:
	template <typename dest_t>
	class type: public node {
	private:
		F functor;
		dest_t dest;
	public:
		typedef typename std::decay<typename unary_traits<F>::argument_type>::type item_type;
		type(dest_t dest, const F & functor):
			functor(functor), dest(std::move(dest)) {
			set_name(bits::extract_pipe_name(typeid(F).name()), PRIORITY_NO_NAME);
		}
		
		void push(const item_type & item) {
			if (functor(item))
				dest.push(item);
		}
	};
};

} //namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that keeps only elements where functor evaluates
/// to true.
/// \param functor The filter to use
///////////////////////////////////////////////////////////////////////////////
template <typename F>
pipe_middle<tempfactory<bits::filter_t<F>, F> > filter(const F & functor) {
	return tempfactory<bits::filter_t<F>, F >(functor);
}

} //namespace pipelining
} //namespace terrastream

#endif //__TPIE_PIPELINING_FILTER_H__
