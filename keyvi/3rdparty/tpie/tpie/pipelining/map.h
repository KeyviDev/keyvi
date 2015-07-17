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

#ifndef __TPIE_PIPELINING_MAP_H__
#define __TPIE_PIPELINING_MAP_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/node_name.h>
#ifdef TPIE_CPP_DECLTYPE
#include <type_traits>
#else
#include <boost/functional.hpp>
#endif

namespace tpie {
namespace pipelining {
namespace bits {

#ifdef TPIE_CPP_DECLTYPE

template <typename T>
struct unary_traits: public unary_traits<decltype(&T::operator()) > {};

template <typename C, typename R, typename A>
struct unary_traits<R(C::*)(A)> {
	typedef A argument_type;
	typedef R return_type;
};

template <typename C, typename R, typename A>
struct unary_traits<R(C::*)(A) const > {
	typedef A argument_type;
	typedef R return_type;
};

template <typename R, typename A>
struct unary_traits<R(*)(A)> {
	typedef A argument_type;
	typedef R return_type;
};
#endif //TPIE_CPP_DECLTYPE

template <typename F>
class map_t {
public:
	template <typename dest_t>
	class type: public node {
	private:
		F functor;
		dest_t dest;
	public:
#ifdef TPIE_CPP_DECLTYPE
		typedef typename std::decay<typename unary_traits<F>::argument_type>::type item_type;
#else
		typedef typename boost::template unary_traits<F>::argument_type item_type;
#endif	
		type(TPIE_TRANSFERABLE(dest_t) dest, const F & functor):
			functor(functor), dest(TPIE_MOVE(dest)) {
			set_name(bits::extract_pipe_name(typeid(F).name()), PRIORITY_NO_NAME);
		}
		
		void push(const item_type & item) {
			dest.push(functor(item));
		}
	};
};

} //namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that applies to given functor to items in
/// the stream.
/// \param f The functor that should be applied to items
///////////////////////////////////////////////////////////////////////////////
template <typename F>
pipe_middle<tempfactory_1<bits::map_t<F>, F> > map(const F & functor) {
	return tempfactory_1<bits::map_t<F>, F >(functor);
}

} //namespace pipelining
} //namespace terrastream

#endif //__TPIE_PIPELINING_MAP_H__
