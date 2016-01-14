// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, 2012, The TPIE development team
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
#ifndef __TPIE_INTERNAL_STACK_VECTOR_BASE_H__
#define __TPIE_INTERNAL_STACK_VECTOR_BASE_H__

///////////////////////////////////////////////////////////////////////////////
/// \file internal_stack_vector_base.h
/// Generic base for internal stack and vector with known memory requirements.
///////////////////////////////////////////////////////////////////////////////
#include <tpie/array.h>
#include <tpie/util.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief A base class for a generic internal fixed size stack and vector.
///
/// \tparam T The type of items stored in the container.
/// \tparam child_t The subtype of the class (CRTP).
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename child_t>
class internal_stack_vector_base: public linear_memory_base<child_t> {
protected:
	/** Element storage. */
	array<T> m_elements;

	/** Number of elements pushed to the structure. */
	size_t m_size;
public:
	typedef T value_type;

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc tpie::linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {return array<T>::memory_coefficient();}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc tpie::linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {
		return array<T>::memory_overhead() - sizeof(array<T>) + sizeof(child_t);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct structure with given capacity.
	///
	/// If a zero capacity is given (the default), no elements may be pushed to
	/// the structure until the structure is <tt>resize</tt>d to a different
	/// capacity.
	///
	/// \param size  Capacity of the structure.
	///////////////////////////////////////////////////////////////////////////
	internal_stack_vector_base(size_t size=0, tpie::memory_bucket_ref b=memory_bucket_ref()):
		m_elements(size, b), m_size(0) {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Change the capacity of the structure and clear all elements.
	///
	/// \param size  New capacity of the structure.
	///////////////////////////////////////////////////////////////////////////
	void resize(size_t size=0) {m_elements.resize(size); m_size=0;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if no elements are currently pushed to the structure.
	///////////////////////////////////////////////////////////////////////////
	inline bool empty() const {return m_size==0;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the number of elements in the data structure.
	///////////////////////////////////////////////////////////////////////////
	inline size_t size() const {return m_size;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Clear the data structure of all elements.
	///////////////////////////////////////////////////////////////////////////
	inline void clear(){m_size=0;}
};

}
#endif //__TPIE_INTERNAL_STACK_VECTOR_BASE_H__
