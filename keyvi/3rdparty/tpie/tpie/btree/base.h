// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The TPIE development team
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

#ifndef _TPIE_BTREE_BASE_H_
#define _TPIE_BTREE_BASE_H_
#include <tpie/portability.h>
#include <functional>
namespace tpie {

/**
 * \brief Augmentation struct used in an un-augmented btree
 */
struct empty_augment {};

/**
 * \brief Functor used to augment an un-augmented btree
 */
struct empty_augmenter {
    template <typename T>
    empty_augment operator()(const T &) {return empty_augment();}
};

/**
 * \brief Functor used to extract the key from a value in case 
 * keys and values are the same
 */
template <typename T>
struct identity_key {
	typedef T value_type;
	
	T operator()(T & t) const {
		return t;
	}
};


/**
 * Type that is useful for navigating a btree
 *
 * S is the type of the store used
 */
template <typename S>
class btree_node;


template <typename S>
class btree_iterator;

/**
 * \brief Augmented btree
 * 
 * The fanout and location of nodes is decided by the store type S
 * C is the item comparator type
 * A is the type of the augmentation computation fuctor
 */
template <typename S,
          typename C=std::less<typename S::key_type>,
          typename A=empty_augmenter>
class btree;

} //namespace tpie
#endif /*_TPIE_BTREE_BASE_H_*/
