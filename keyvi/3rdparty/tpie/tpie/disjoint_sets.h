// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, The TPIE development team
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
#ifndef __TPIE_DISJOINT_SETS__
#define __TPIE_DISJOINT_SETS__
/////////////////////////////////////////////////////////////
/// \file disjoint_sets.h
/// Generic internal disjoint_sets (union find)
/// implementation
/////////////////////////////////////////////////////////////

#include <tpie/array.h>
#include <tpie/unused.h>
#include <tpie/util.h>

namespace tpie {

/////////////////////////////////////////////////////////////
/// \brief Internal memory union find implementation
///
/// The key space is the first n integers (from 0 to n-1).
/// n is given in the constructor.
///
/// \tparam value_t The type of values stored (must be an
/// integer type).
/////////////////////////////////////////////////////////////
template <typename value_t=size_type> 
class disjoint_sets: public linear_memory_base< disjoint_sets<value_t> > {
private:
	array<value_t> m_elements;
	value_t m_unused;
	size_type m_size;
public:
	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	/////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return array<value_t>::memory_coefficient();
	}

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	/////////////////////////////////////////////////////////
	static double memory_overhead() {
		return array<value_t>::memory_overhead() + sizeof(disjoint_sets) - sizeof(array<value_t>);
	}

	/////////////////////////////////////////////////////////
	/// \brief Construct a empty collection of disjoint sets
	///																  
	/// \param n The maximal number of sets to support
	/// \param u A value you guarentee not to use.
	/////////////////////////////////////////////////////////
	disjoint_sets(size_type n=0,
				  value_t u = default_unused<value_t>::v(),
				  tpie::memory_bucket_ref b = tpie::memory_bucket_ref())
		: m_elements(n, u, b), m_unused(u), m_size(0) {}	

	disjoint_sets(tpie::memory_bucket_ref b): m_elements(b), m_unused(default_unused<value_t>::v()), m_size(0) {}
	
	/////////////////////////////////////////////////////////
	/// \brief Make a singleton set
	///
	/// \param element The key of the singleton set to create
	/////////////////////////////////////////////////////////
	inline void make_set(value_t element) {m_elements[element] = element; ++m_size;}

	/////////////////////////////////////////////////////////
	/// \brief Check if a given element is a member of any set
	///
	/// This is the same as saying if make_set has ever been
	/// called with the given key
	/// \param element The key to check
	/////////////////////////////////////////////////////////
	inline bool is_set(value_t element) {return m_elements[element] != m_unused;}

	/////////////////////////////////////////////////////////
	/// \brief Union two sets given by their representatives
	///
	/// \param a The representative of one set
	/// \param b The representative of the other set
	/// \return The representative of the combined set
	/////////////////////////////////////////////////////////
	inline value_t link(value_t a, value_t b) {
		if (a == b) return a;
		--m_size;
		m_elements[b] = a;
		return a;
	}

	/////////////////////////////////////////////////////////
	/// \brief Find the representative of the set contaning
	/// a given element.
	///
	/// \param t The element of which to find the set representative
	/// \return The representative.
	/////////////////////////////////////////////////////////
	inline value_t find_set(value_t t) {
		// If t sits in depth d, we halve the depth of d/2 nodes in the tree,
		// including t.
		while (true) {
			// Set t to point to its grandparent.
			value_t x = m_elements[m_elements[t]];
			if (x == t) return t;
			m_elements[t] = x;
			t = x;
		}

		// The textbook implementation below is faster for some adversarial
		// inputs, but is cache inefficient on ordinary input.

		//value_t r = m_elements[t];
		//if (r == t)
		//	return r;
		//while (r != m_elements[r])
		//	r = m_elements[r];
		//while (t != r) {
		//	value_t next = m_elements[t];
		//	m_elements[t] = r;
		//	t = next;
		//}
		//return r;
	}

	/////////////////////////////////////////////////////////
	/// \brief Union the set containing a with the set
	/// containing b
	/// 
	/// \param a An element in one set
	/// \param b An element in another set (possible)
	/// \return The representative of the unioned set
	/////////////////////////////////////////////////////////
	inline value_t union_set(value_t a, value_t b) {
		return link(find_set(a), find_set(b));
	}

	/////////////////////////////////////////////////////////
	/// \brief Return the number of sets
	/////////////////////////////////////////////////////////
	inline size_type count_sets() {
		return m_size;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Clears all sets contained in the datastructure.
	///////////////////////////////////////////////////////////////////////////////
	void clear() {
		std::fill(m_elements.begin(), m_elements.end(), m_unused);
		m_size = 0;
	}

	/////////////////////////////////////////////////////////
	/// \brief Changes the size of the datastructure. 
	/// All elements are lost.
	/////////////////////////////////////////////////////////
	void resize(size_t size) {
		m_elements.resize(size, m_unused);
		m_size = 0;
	}
};

}
#endif //__TPIE_DISJOINT_SETS__
