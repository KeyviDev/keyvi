// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2015, The TPIE development team
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

#ifndef __TPIE_PIPELINING_PIPE_CONTAINER_H__
#define __TPIE_PIPELINING_PIPE_CONTAINER_H__

#include <type_traits>

namespace tpie {
namespace pipelining {

template <typename T>
T & move_if_movable(typename std::remove_reference<T>::type & t,
			typename std::enable_if<!std::is_move_constructible<T>::value>::type* = 0) {
	return t;
}

template <typename T>
T && move_if_movable(typename std::remove_reference<T>::type & t,
			typename std::enable_if<std::is_move_constructible<T>::value>::type* = 0) {
	return static_cast<T&&>(t);
}

template <typename T>
T & move_if_movable_rvalue(typename std::remove_reference<T>::type & t,
			typename std::enable_if<
				!std::is_move_constructible<T>::value ||
				!std::is_rvalue_reference<T>::value
			>::type* = 0) {
 	return t;
}

template <typename T>
T && move_if_movable_rvalue(typename std::remove_reference<T>::type & t,
			typename std::enable_if<
				std::is_move_constructible<T>::value &&
				std::is_rvalue_reference<T>::value
			>::type* = 0) {
	return static_cast<T&&>(t);
}


template <typename ... TT>
class container {};

template <typename T, typename ... TT>
struct container<T, TT...> : public container<TT...> {
	container(const container &) = delete;
	container() = delete;

	template <typename T2, typename ... TT2>
	container(T2 && t, TT2 && ... tt)
		: container<TT...>(std::forward<TT2>(tt)...)
		, v(move_if_movable_rvalue<T2&&>(t)) {}

	container(container && o)
		: container<TT...>(std::forward<container<TT...> >(o))
		, v(move_if_movable<T>(o.v)) {}

	T v;
};

namespace bits {
template <int i, typename T, typename ...TT>
struct get_impl {
	typedef typename get_impl<i-1, TT...>::type type;
	static type & get(container<T, TT...> & cc) {return get_impl<i-1, TT...>::get(cc);}
	static const type & get(const container<T, TT...> & cc) {return get_impl<i-1, TT...>::get(cc);}
};

template <typename T, typename ...TT>
struct get_impl<0, T, TT...> {
	typedef T type;
	static type & get(container<T, TT...> & cc) {return cc.v;}
	static const type & get(const container<T, TT...> & cc) {return cc.v;}
};
} //namespace bits


template <int i, typename ...T>
typename bits::get_impl<i, T...>::type & get(container<T...> & c) {
	return bits::get_impl<i, T...>::get(c);
}

template <int i, typename ...T>
const typename bits::get_impl<i, T...>::type & get(const container<T...> & c) {
	return bits::get_impl<i, T...>::get(c);
}

namespace bits {
template<int ... S>
struct dispatch {
	template <typename F, typename ... T1, typename ... T2>
	static F run_move(container<T1...> & t, T2 &&... o) {
		return F(std::move(o)..., move_if_movable<T1>(get<S>(t))...);
	}

	template <typename F, typename ... T1, typename ... T2>
	static F run_copy(container<T1...> & t, T2 &&... o) {
		return F(std::move(o)..., get<S>(t)...);
	}
};

template<int N, int ...S>
struct dispatch_gen : dispatch_gen<N-1, N-1, S...> { };

template<int ...S>
struct dispatch_gen<0, S...> {
	typedef dispatch<S...> type;
};

} //namespace bits


template <typename F, typename ... T1, typename ... T2>
F container_construct(container<T1...> & cont, T2 && ... a) {
	return bits::dispatch_gen<sizeof...(T1)>::type::template run_move<F>(cont, std::move(a)...);
}

template <typename F, typename ... T1, typename ... T2>
F container_construct_copy(container<T1...> & cont, T2 && ... a) {
	return bits::dispatch_gen<sizeof...(T1)>::type::template run_copy<F>(cont, std::move(a)...);
}


} //namespace pipelining
} //namespace tpie
#endif //__TPIE_PIPELINING_PIPE_CONTAINER_H__
