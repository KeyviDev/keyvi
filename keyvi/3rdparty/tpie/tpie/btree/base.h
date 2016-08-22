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
	typedef empty_augment value_type;
	
    template <typename T>
    empty_augment operator()(const T &) {return empty_augment();}
};

/**
 * \brief Functor used to extract the key from a value in case 
 * keys and values are the same
 */
struct identity_key {
	template <typename T>
	const T & operator()(const T & t) const noexcept {return t;}
};

/**
 * \brief Default < comparator for the btree
 */
struct default_comp {
	template <typename L, typename R>
	bool operator()(const L & l, const R & r) const noexcept {
		return l < r;
	}
};

struct empty_key {};

struct no_key {
	template <typename T>
	empty_key operator()(const T &) const noexcept {return empty_key{};};
};

namespace bbits {
template <int i>
struct int_opt {static const int O=i;};

static const int f_internal = 1;
static const int f_static = 2;
static const int f_unordered = 4;
static const int f_serialized = 8;

} //namespace bbits

template <typename T>
struct btree_comp {typedef T type;};

template <typename T>
struct btree_key {typedef T type;};

template <typename T>
struct btree_augment {typedef T type;};

template <int a_, int b_>
struct btree_fanout {
	static const int a = a_;
	static const int b = b_;
};

using btree_internal = bbits::int_opt<bbits::f_internal>;
using btree_external = bbits::int_opt<0>;

using btree_static = bbits::int_opt<bbits::f_static>;
using btree_dynamic = bbits::int_opt<0>;

using btree_unordered = bbits::int_opt<bbits::f_unordered>;
using btree_ordered = bbits::int_opt<0>;

using btree_serialized = bbits::int_opt<bbits::f_serialized>;
using btree_not_serialized = bbits::int_opt<0>;

namespace bbits {

//O = flags, a, b = B-tree parameters, C = comparator, K = key extractor, A = augmenter
template <int O_, int a_, int b_, typename C_, typename K_, typename A_>
struct Opt {
	static const int O=O_;
	static const int a=a_;
	static const int b=b_;
	typedef C_ C;
	typedef K_ K;
	typedef A_ A;
};

template <typename ... Opt>
struct OptComp {};

template <>
struct OptComp<> {
	typedef Opt<0, 0, 0, default_comp, identity_key, empty_augmenter> type;
};

template <int i, typename ... T>
struct OptComp<int_opt<i> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O | i, P::a, P::b, typename P::C, typename P::K, typename P::A> type;
};

template <typename C, typename ... T>
struct OptComp<btree_comp<C> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O, P::a, P::b, C, typename P::K, typename P::A> type;
};

template <typename K, typename ... T>
struct OptComp<btree_key<K> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O, P::a, P::b, typename P::C, K, typename P::A> type;
};

template <typename A, typename ... T>
struct OptComp<btree_augment<A> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O, P::a, P::b, typename P::C, typename P::K, A> type;
};

template <int a, int b, typename ... T>
struct OptComp<btree_fanout<a, b> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O, a, b, typename P::C, typename P::K, typename P::A> type;
};

} //namespace bbits



/**
 * Type that is useful for navigating a btree
 *
 * S is the type of the store used
 */
template <typename S>
class btree_node;


template <typename S>
class btree_iterator;

namespace bbits {

/**
 * \brief Augmented btree
 * 
 * The fanout and location of nodes is decided by the store type S
 * C is the item comparator type
 * A is the type of the augmentation computation fuctor
 */
template <typename T,
          typename O>
class tree;

/**
 * \brief Augmented btree builder
 *
 * The fanout and location of nodes is decided by the store type S
 * C is the item comparator type
 * A is the type of the augmentation computation fuctor
 */
template <typename T, typename O>
class builder;

template <typename T, typename A, std::size_t a, std::size_t b>
class internal_store;

template <typename T, typename A, std::size_t a, std::size_t b>
class external_store;

template <typename T, typename A, std::size_t a, std::size_t b>
class serialized_store;

struct enab {};

template <typename X, bool b>
struct Enable {};

template <>
struct Enable<enab, true> {typedef enab type;};

template <typename X, bool b>
using enable = typename Enable<X,b>::type;

template <typename T, typename O>
class tree_state {
public:
	static const bool is_internal = O::O & bbits::f_internal;
	static const bool is_static = O::O & bbits::f_static;
	static const bool is_ordered = ! (O::O & bbits::f_unordered);
	static const bool is_serialized = O::O & bbits::f_serialized;
	static_assert(!is_serialized || is_static, "Serialized B-tree cannot be dynamic.");
	
	typedef typename std::conditional<
		is_ordered,
		typename O::K,
		no_key>::type keyextract_type;

	typedef typename O::A augmenter_type;
	
	typedef T value_type;

	typedef typename std::decay<decltype(std::declval<augmenter_type>()(std::declval<value_type>()))>::type augment_type;

	typedef typename std::decay<decltype(std::declval<keyextract_type>()(std::declval<value_type>()))>::type key_type;

	struct key_augment {
		key_type key;
	};

	struct combined_augment
		: public key_augment
		, public augment_type {};


	struct combined_augmenter {
		template <typename N>
		combined_augment operator()(const N & node) {
			combined_augment ans;
			*static_cast<augment_type*>(&ans) = m_augmenter(node);

			static_cast<key_augment*>(&ans)->key =
				node.is_leaf()
				? m_key_extract(node.value(0))
				: static_cast<const key_augment*>(&node.get_combined_augmentation(0))->key;
			return ans;
		}

		combined_augmenter(
			augmenter_type a,
			keyextract_type key_extract)
			: m_key_extract(std::move(key_extract))
			, m_augmenter(std::move(a)) {}
 
		keyextract_type m_key_extract; 
		augmenter_type m_augmenter;
	};

	typedef typename std::conditional<
		is_internal,
		bbits::internal_store<value_type, combined_augment, O::a, O::b>,
		typename std::conditional<
			is_serialized,
			bbits::serialized_store<value_type, combined_augment, O::a, O::b>,
			bbits::external_store<value_type, combined_augment, O::a, O::b>
			>::type
		>::type store_type;
	
	typedef typename store_type::internal_type internal_type;
	typedef typename store_type::leaf_type leaf_type;
	
	key_type min_key(internal_type node, size_t i) const {
		return static_cast<const key_augment*>(&m_store.augment(node, i))->key;
	}

	key_type min_key(leaf_type node, size_t i) const {
		return m_augmenter.m_key_extract(m_store.get(node, i));
	}

	key_type min_key(T v) const {
		return m_augmenter.m_key_extract(v);
	}

	key_type min_key(internal_type v) const {
		return min_key(v, 0);
	}

	key_type min_key(leaf_type v) const {
		return min_key(v, 0);
	}

	const store_type & store() const {
		return m_store;
	}

	store_type & store() {
		return m_store;
	}

	static const augment_type & user_augment(const combined_augment & a) {
		return *static_cast<const augment_type *>(&a);
	}

	tree_state(store_type store, augmenter_type augmenter, keyextract_type keyextract)
		: m_augmenter(std::move(augmenter), std::move(keyextract))
		, m_store(std::move(store)) {}

	combined_augmenter m_augmenter;
	store_type m_store;
};


} //namespace bbits


} //namespace tpie
#endif /*_TPIE_BTREE_BASE_H_*/
