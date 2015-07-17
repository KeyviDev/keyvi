// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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

#ifndef _B_VECTOR_H
#define _B_VECTOR_H
///////////////////////////////////////////////////////////////////
/// \file b_vector.h
/// Definition of the  class b_vector.
///////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <string.h>

namespace tpie {

    ///////////////////////////////////////////////////////////////////
    /// The b_vector class stores an array of objects of a templated type T. 
    /// It has a fixed maximum size, or capacity, which is set during 
    /// construction (since instances of this class are created only by the 
    /// \ref tpie::ami::block< E,I,BTECOLL > class, the constructors are not 
    /// part of the public interface). The items stored can be  accessed through 
    /// the array operator. 
    ///
    /// The type T should have a default constructor, as well as copy
    /// constructor and assignment operator. 
    ///////////////////////////////////////////////////////////////////
    template<class T>
    class b_vector {

    protected:

   /** Pointer meant to point to the "current" element in the b_vector.*/
   T* p_;

   /** The capacity of the b_vector, .i.e, its maximal size.*/
   size_t capacity_;
	
    public:
      
  /** Typef to template parameter specifying the value type in the b_vector */
  typedef T value_type;

  /** Typef tp  pointer to the template parameter specifying the value type in the b_vector */
	typedef value_type* iterator;
  
	/** Typef to const pointer to template parameter specifying the value type in the b_vector */
	typedef const value_type* const_iterator;
	
  ///////////////////////////////////////////////////////////////////
  /// Constructor taking an array of elements and a capacity.
  ///////////////////////////////////////////////////////////////////
	b_vector(T* p, size_t cap): p_(p), capacity_(cap) {}
	
  ///////////////////////////////////////////////////////////////////
  ///Iterator pointing initially to the first element in the b_vector.
  ///////////////////////////////////////////////////////////////////
	iterator begin() { return p_; }
  
	///////////////////////////////////////////////////////////////////
	/// Const iterator pointing initially to the first element in the b_vector.
	///////////////////////////////////////////////////////////////////
	const_iterator begin() const { return p_; }
  
  ///////////////////////////////////////////////////////////////////
  /// Iterator pointing initially behind the last element in the b_vector.
  ///////////////////////////////////////////////////////////////////
	iterator end() { return p_ + capacity_; }

  ///////////////////////////////////////////////////////////////////
  /// Const iterator pointing initially behind the last element in the b_vector.
  ///////////////////////////////////////////////////////////////////
	const_iterator end() const { return p_ + capacity_; }
	
  ///////////////////////////////////////////////////////////////////
  /// Get a reference to the i'th element.
  ///////////////////////////////////////////////////////////////////
	T& operator[](size_t i) { return *(p_ + i); }
  
  ///////////////////////////////////////////////////////////////////
  /// Get a const reference to the i'th element.
  ///////////////////////////////////////////////////////////////////
	const T& operator[](size_t i) const { return *(p_ + i); }
	
  ///////////////////////////////////////////////////////////////////
  /// Return the capacity (i.e., maximum number of T elements) of this b_vector.
  ///////////////////////////////////////////////////////////////////
	size_t capacity() const { return capacity_; }
	
  ///////////////////////////////////////////////////////////////////
	/// Copy length elements from the source vector, starting with 
	/// element s_start, to this block, starting with element start. 
	/// Return the number of elements copied. Source can be *this.
  ///////////////////////////////////////////////////////////////////
	size_t copy(size_t start, size_t length,
		    const b_vector<T>& source, size_t s_start = 0);
	
  ///////////////////////////////////////////////////////////////////
	/// Copy length items from the array src to this vector, starting in position
	/// start. Return the number of items copied. 
  ///////////////////////////////////////////////////////////////////
	size_t copy(size_t start, size_t length, const T* source);
	
  ///////////////////////////////////////////////////////////////////
	/// Insert item t in position pos; all items from position pos onward
	/// are shifted one position higher; the last item is lost.
  ///////////////////////////////////////////////////////////////////
	void insert(const T& t, size_t pos);
	
  ///////////////////////////////////////////////////////////////////
	/// Erase the item in position pos and shift all items from position
	/// pos+1 onward one position lower; the last item becomes identical
	/// with the next to last item.
  ///////////////////////////////////////////////////////////////////
	void erase(size_t pos);
	
    };
    
    ////////////////////////////////
    ///////// **b_vector** /////////
    ////////////////////////////////
    
    //// *b_vector::copy* ////
    template<class T>
    size_t b_vector<T>::copy(size_t start, size_t length,
			     const b_vector<T>& source, size_t s_start) {
	
	// copy_length will store the actual number of items that can be copied.
	size_t copy_length = length;
	
	if (start < capacity_ && s_start < source.capacity()) {
	    // Check how much of length we can copy.
	    copy_length = (copy_length > capacity_ - start) ? 
		capacity_ - start: copy_length;
	    copy_length = (copy_length > source.capacity() - s_start) ? 
		source.capacity() - s_start: copy_length;
	    
	    memmove(&(*this)[start], &source[s_start], copy_length * sizeof(T));
	} else {
	    // start is too big. No copying.
	    copy_length = 0;
	}
	
	return copy_length;
    }
    
//// *b_vector::copy* ////
    template<class T>
    size_t b_vector<T>::copy(size_t start, size_t length, const T* source) {
	
	size_t copy_length = length;
	
	if (start < capacity_) {
	    // Check how much of length we can copy.
	    copy_length = (copy_length > capacity_ - start) ? 
		capacity_ - start: copy_length;
	    
	    memmove(&(*this)[start], source, copy_length * sizeof(T));
	} else
	    copy_length = 0;
	
	return copy_length;
    }
    
//// *b_vector::insert* ////
    template<class T>
    void b_vector<T>::insert(const T& t, size_t pos) {
	copy(pos + 1, capacity_ - pos - 1, *this, pos);
	copy(pos, 1, &t);
    }
    
//// *b_vector::erase* ////
    template<class T>
    void b_vector<T>::erase(size_t pos) {
	copy(pos, capacity_ - pos - 1, *this, pos + 1);
    }

} //  tpie namespace

#endif // _B_VECTOR_H
