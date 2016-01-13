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

#ifndef __TPIE_PIPELINING_STORE_H__
#define __TPIE_PIPELINING_STORE_H__
#include <tpie/memory.h>
namespace tpie {

namespace bits {
template <typename T> struct remove_pointer {typedef T type;};
template <typename T> struct remove_pointer<T*> {typedef T type;};

template <typename pred_t, typename specific_store_t>
class store_pred {
private:
	typedef typename specific_store_t::store_type store_type;
	pred_t pred;
public:
	typedef store_type first_argument_type;
	typedef store_type second_argument_type;
	typedef bool result_type;

	store_pred(pred_t pred): pred(pred) {}

	bool operator()(const store_type & lhs, const store_type & rhs) const {
		return pred(
			specific_store_t::store_as_element(lhs),
			specific_store_t::store_as_element(rhs));
	}

	bool operator()(const store_type & lhs, const store_type & rhs) {
		return pred(
			specific_store_t::store_as_element(lhs),
			specific_store_t::store_as_element(rhs));
	}
};
} //namespace bits

/**
 * \brief Plain old store
 *
 * We sort elements of type T, they are pushed to us a T, and we store them as T.
 */
struct plain_store {
	template <typename outer_t>
	struct element_type {
		typedef outer_t type;
	};

	template <typename element_t>
	struct specific {
		// Type of element we compare and store in file_stream
		typedef element_t element_type;
		// Type we store in internal sort buffer
		typedef element_t store_type;
		// Type we accept in push and return in pop
		typedef element_t outer_type;
		// Internal memory usage per element in internal sort
		static const size_t item_size = sizeof(element_t);

		element_type store_to_element(const store_type & e) {return e;}
		store_type element_to_store(const element_type & e) {return e;}
		static const element_type & store_as_element(const store_type & e) {return e;}
		store_type outer_to_store(const outer_type & e) {return e;}
		outer_type store_to_outer(const store_type & e) {return e;}
	};

	template <typename element_t>
	specific<element_t> get_specific() {return specific<element_t>();}
};

/**
 * \brief Sort elements in tpie pointers
 *
 * We sort elements of type T, they are pushed to us as T * and we store them as T*.
 * Pointers pushed must be allocated with tpie_new,
 * and the sorter takes ownership.
 * When returned, ownership of the new pointer is given to user.
 * The elements are tpie_delete'ed and tpie_new'ed.
 */
struct explicit_tpie_pointer_store {
	template <typename outer_t>
	struct element_type {
		typedef typename bits::remove_pointer<outer_t>::type type;
	};

	template <typename element_t>
	struct specific {
		typedef element_t element_type;
		typedef element_t * store_type;
		typedef element_t * outer_type;

		static const size_t item_size = sizeof(element_t) + sizeof(store_type);

		element_type store_to_element(const store_type e) {
			element_type ans=*e;
			tpie_delete(e);
			return ans;
		}
		store_type element_to_store(const element_type & e) {return tpie_new<element_type>(e);}
		static const element_type & store_as_element(const store_type e) {return *e;}
		store_type outer_to_store(const outer_type & e) {return e;}
		outer_type store_to_outer(const store_type & e) {return e;}
	};

	template <typename element_t>
	specific<element_t> get_specific() {return specific<element_t>();}
};

/**
 * \brief Sort elements in tpie unique pointers
 *
 * We sort elements of type T, they are pushed to us as tpie::unique_ptr<T> and we store them as tpie::unique_ptr<T>.
 */
struct explicit_tpie_unique_pointer_store {
	template <typename outer_t>
	struct element_type {
		typedef typename outer_t::element_type type;
	};

	template <typename element_t>
	struct specific {
		typedef element_t element_type;
		typedef tpie::unique_ptr<element_t> store_type;
		typedef tpie::unique_ptr<element_t> outer_type;

		static const size_t item_size = sizeof(element_t) + sizeof(store_type);

		element_type store_to_element(store_type e) {
			return *e;
		}
		store_type element_to_store(const element_type & e) {
			return outer_type(tpie_new<element_type>(e));
		}
		static const element_type & store_as_element(const store_type & e) {return *e;}
		store_type outer_to_store(outer_type e) {return std::move(e);}
		outer_type store_to_outer(store_type e) {return std::move(e);}
	};

	template <typename element_t>
	specific<element_t> get_specific() {return specific<element_t>();}
};

/**
 * \brief Sort elements using pointer indirection.
 *
 * We sort elements of type T, they are push to us a T. But internally they are storred as
 * T *.
 */
struct pointer_store {
	template <typename outer_t>
	struct element_type {
		typedef outer_t type;
	};

	template <typename element_t>
	struct specific {
		typedef element_t element_type;
		typedef element_t * store_type;
		typedef element_t outer_type;

		static const size_t item_size = sizeof(element_t) + sizeof(store_type);

		element_type store_to_element(const store_type e) {
			element_type ans=*e;
			tpie_delete(e);
			return ans;
		}
		store_type element_to_store(const element_type & e) {return tpie_new<element_type>(e);}
		static const element_type & store_as_element(const store_type e) {return *e;}
		store_type outer_to_store(const outer_type & e) {return tpie_new<element_type>(e);}
		outer_type store_to_outer(const store_type & e) {
			outer_type ans=*e;
			tpie_delete(e);
			return ans;
		}
	};

	template <typename element_t>
	specific<element_t> get_specific() {return specific<element_t>();}
};

namespace bits {

template <typename element_t, bool = (sizeof(element_t) > 256)>
struct dynamic_specific_selector: public plain_store::specific<element_t> {};

template <typename element_t>
struct dynamic_specific_selector<element_t, true>: public pointer_store::specific<element_t> {};

} //namespace bits

/**
 * \brief Fantastic store strategy.
 *
 * We sort elements of type T, they are push to us a T
 * If sizeof(T) is small, they are storred internally as T,
 * if on the other hand sizeof(T) is large they are sorted using pointer indirection.
 *
 * \note currently large means > 256 bytes, no experiments has been done to verify that this is sane.
 */
struct dynamic_store {
	template <typename outer_t>
	struct element_type {
		typedef outer_t type;
	};

	template <typename element_t>
	struct specific: public bits::dynamic_specific_selector<element_t> {};

	template <typename element_t>
	specific<element_t> get_specific() {return specific<element_t>();}
};


typedef dynamic_store default_store;

} //namespace tpie

#endif //__TPIE_PIPELINING_SPECIFIC_POINTER_H__
