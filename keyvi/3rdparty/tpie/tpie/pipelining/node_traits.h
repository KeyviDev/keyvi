// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
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

#ifndef TPIE_PIPELINING_NODE_TRAITS_H
#define TPIE_PIPELINING_NODE_TRAITS_H

#include <type_traits>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename T>
struct remove {
	typedef T type;
};

template <typename T>
struct remove<const T> {
	typedef typename remove<T>::type type;
};

template <typename T>
struct remove<T &> {
	typedef typename remove<T>::type type;
};

template <typename T>
struct remove<T &&> {
	typedef typename remove<T>::type type;
};

template <typename T>
struct push_traits {};

template <typename ClassType, typename ArgType>
struct push_traits< void(ClassType::*)(ArgType) > {
	typedef ArgType type;
};

template <typename T, bool>
struct push_type_help {
	typedef typename push_traits<decltype(&T::push)>::type type_;
	typedef typename remove<type_>::type type;
};

template <typename T>
struct push_type_help<T, true> {
	typedef typename T::item_type type;
};

template <typename T>
struct pull_traits {};

template <typename ClassType, typename ArgType>
struct pull_traits< ArgType(ClassType::*)() > {
	typedef ArgType type;
};

template <typename T, bool>
struct pull_type_help {
	typedef typename pull_traits<decltype(&T::pull)>::type type_;
	typedef typename remove<type_>::type type;
};

template <typename T>
struct pull_type_help<T, true> {
	typedef typename T::item_type type;
};

template <typename T>
struct has_itemtype {
	typedef char yes[1];
	typedef char no[2];

	template <typename C>
	static yes& test(typename C::item_type*);

	template <typename>
	static no& test(...);
	//static_assert(sizeof(test<T>(nullptr)) == sizeof(yes), "WTF");
	static const bool value = sizeof(test<T>(nullptr)) == sizeof(yes);
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// Class to deduce the item_type of a node of type T.
/// typedef item_type to T::item_type if that exists
/// otherwise typedef it to the type of whatever paramater push takes
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct push_type {
	typedef typename bits::remove<T>::type node_type;
	typedef typename bits::push_type_help<node_type, bits::has_itemtype<T>::value>::type type;
};

template <typename T>
struct pull_type {
	typedef typename bits::remove<T>::type node_type;
	typedef typename bits::pull_type_help<node_type, bits::has_itemtype<T>::value>::type type;
};

} // namespace pipelining

} // namespace tpie

#endif // TPIE_PIPELINING_NODE_TRAITS_H
