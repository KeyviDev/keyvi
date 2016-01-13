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
#ifndef __TPIE_INTERNAL_VECTOR_H__
#define __TPIE_INTERNAL_VECTOR_H__

///////////////////////////////////////////////////////////////////////////
/// \file internal_vector.h
/// Generic internal vector with known memory requirements.
///////////////////////////////////////////////////////////////////////////
#include <tpie/array.h>
#include <tpie/util.h>
#include <tpie/internal_stack_vector_base.h>
namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief A generic internal vector.
///
/// \tparam T The type of items stored in the structure.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class internal_vector: public internal_stack_vector_base<T,internal_vector<T> > {
public:
	typedef internal_stack_vector_base<T,internal_vector<T> > parent_t;
	typedef typename array<T>::iterator iterator;
	typedef typename array<T>::const_iterator const_iterator;
	using parent_t::m_elements;
	using parent_t::m_size;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct structure with given capacity.
	/// \copydetails tpie::internal_stack_vector_base::internal_stack_vector_base
	///////////////////////////////////////////////////////////////////////////
	internal_vector(size_t size=0, tpie::memory_bucket_ref bucket = memory_bucket_ref())
					: parent_t(size, bucket){}

	
	internal_vector(tpie::memory_bucket_ref bucket): parent_t(0, bucket){}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Element access. No range checking is done.
	///////////////////////////////////////////////////////////////////////////
	inline T & operator[](size_t s){return m_elements[s];}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Element access. No range checking is done.
	///////////////////////////////////////////////////////////////////////////
	inline const T & operator[](size_t s)const {return m_elements[s];}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the first item pushed. Requires \c !empty().
	///////////////////////////////////////////////////////////////////////////
	inline T & front(){return m_elements[0];}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the first item pushed. Requires \c !empty().
	///////////////////////////////////////////////////////////////////////////
	inline const T & front()const {return m_elements[0];}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the last item pushed. Requires \c !empty().
	///////////////////////////////////////////////////////////////////////////
	inline T & back(){return m_elements[m_size-1];}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the last item pushed. Requires \c !empty().
	///////////////////////////////////////////////////////////////////////////
	inline const T & back()const {return m_elements[m_size-1];}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Add an element to the end of the vector.
	/// If size() is equal to the capacity (set in the constructor or in
	/// resize()), effects are undefined. resize() is not called implicitly.
	///
	/// Iterators are invalidated by this call.
	///
	/// \param val The element to add.
	///////////////////////////////////////////////////////////////////////////
	inline T & push_back(const T & val){m_elements[m_size++] = val; return back();}

	///////////////////////////////////////////////////////////////////////////
	/// \brief If an item was previously popped from this point in the
	/// structure, push it to the structure again; otherwise, push the default
	/// value.
	///
	/// Iterators are invalidated by this call.
	///
	/// \param val The element to add.
	///////////////////////////////////////////////////////////////////////////
	inline T & push_back(){++m_size; return back();}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Remove the last element from the vector.
	///
	/// Iterators are invalidated by this call.
	///////////////////////////////////////////////////////////////////////////
	inline void pop_back(){--m_size;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get an iterator to the beginning of the structure.
	///////////////////////////////////////////////////////////////////////////
	inline iterator begin(){ return m_elements.begin();}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get an iterator to the beginning of the structure.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator begin()const {return m_elements.begin();}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get an iterator to the end of the structure.
	///////////////////////////////////////////////////////////////////////////
	inline iterator end(){return m_elements.find(m_size);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get an iterator to the end of the structure.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator end()const {return m_elements.find(m_size);}
};

}
#endif //__TPIE_INTERNAL_VECTOR_H__
