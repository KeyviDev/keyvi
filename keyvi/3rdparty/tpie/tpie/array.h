// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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
#ifndef __TPIE_ARRAY_H__
#define __TPIE_ARRAY_H__

///////////////////////////////////////////////////////////////////////////
/// \file array.h
/// Generic internal array with known memory requirements.
///////////////////////////////////////////////////////////////////////////
#include <tpie/util.h>
#include <boost/type_traits/is_pod.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/enable_if.hpp>
#include <tpie/memory.h>
#include <tpie/array_view_base.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \internal
/// \brief Shared implementation of array iterators.
///////////////////////////////////////////////////////////////////////////////
template <typename TT, bool forward>
class array_iter_base: public boost::iterator_facade<
	array_iter_base<TT, forward>,
	TT , boost::random_access_traversal_tag> {
private:
	template <typename, typename> friend class array;
	friend class boost::iterator_core_access;
	template <typename, bool> friend class array_iter_base;

	struct enabler {};
	explicit array_iter_base(TT * e): elm(e) {}

	inline TT & dereference() const {return * elm;}
	template <class U>
	inline bool equal(array_iter_base<U, forward> const& o) const {return elm == o.elm;}
	inline void increment() {elm += forward?1:-1;}
	inline void decrement() {elm += forward?-1:1;}
	inline void advance(size_t n) {if (forward) elm += n; else elm -= n;}
	inline ptrdiff_t distance_to(array_iter_base const & o) const {return o.elm - elm;}
	TT * elm;
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor.
	///////////////////////////////////////////////////////////////////////////
	array_iter_base(): elm(0) {};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy constructor.
	/// We use boost::enable_if to allow copying an iterator with a more
	/// specific item_type to an iterator with a more general item_type.
	///////////////////////////////////////////////////////////////////////////
	template <class U>
	array_iter_base(array_iter_base<U, forward> const& o, typename boost::enable_if<
			  boost::is_convertible<U*,TT*>, enabler>::type = enabler())
			: elm(o.elm) {}
};

#pragma pack(push, 1)
template <typename C>
struct trivial_same_size {
	char c[sizeof(C)];
};
#pragma pack(pop)

namespace bits {
	template <typename T, typename Allocator> struct allocator_usage;
} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A generic array with a fixed size.
///
/// This is almost the same as a real C-style T array but the memory management
/// is better.
///
/// \tparam T The type of element to contain.
/// \tparam alloc_t Allocator.
///////////////////////////////////////////////////////////////////////////////
// ASIDE about the C++ language: A type is said to be trivially constructible
//     if it has no user-defined constructors and all its members are trivially
//     constructible. `new T` will allocate memory for a T-element, and if T is
//     trivially constructible, the memory will be left uninitialized. This
//     goes for arrays (T[]) as well, meaning an array initialization of a
//     trivially constructible type takes practically no time.
//
// IMPLEMENTATION NOTE. We have three cases for how we want the item buffer
// `array::m_elements` to be initialized:
//
// 1. Default constructed. tpie_new_array<T> does this
//    allocation+initialization.
//
// 2. Copy constructed from a single element. Cannot be done with
//    tpie_new_array<T>.
//
// 3. Copy constructed from elements in another array. Cannot be done with
//    tpie_new_array<T> either.
//
// For cases 2 and 3, we must keep `new`-allocation separate from item
// initialization. Thus, for these cases we instead allocate an array of
// trivial_same_size<T>. This is a struct that has the same size as T, but is
// trivially constructible, meaning no memory initialization is done.
//
// Then, for case 2, we use std::uninitialized_fill, and for case 3 we use
// std::uninitialized_copy.
//
// Unfortunately, although we have the choice in case 1 of allocating a
// trivial_same_size<T> and then calling the default constructors afterwards,
// this turns out in some cases to be around 10% slower than allocating a
// T-array directly.
//
// Also, in order to have the same initialization semantics with regards to
// trivially constructible types as C++ new[], we need to check if the type is
// trivially constructible (using SFINAE/boost::enable_if/boost::type_traits)
// to avoid zero-initializing those (a speed penalty we cannot afford).
//
// Now it is clear that we must sometimes use tpie_new_array<T>, other times
// tpie_new_array<trivial_same_size<T> >. The TPIE memory manager checks that
// buffers allocated as one type are not deallocated as another type, so when
// the buffer is allocated as a trivial_same_size, we must remember this fact
// for later destruction and deallocation.
//
// We remember this fact in array::m_tss_used.
///////////////////////////////////////////////////////////////////////////////

template <typename T, typename Allocator = allocator<T> >
class array : public linear_memory_base<array<T> > {
public:
	/** \brief Iterator over a const array */
	typedef array_iter_base<T const, true> const_iterator;

	/** \brief Reverse iterator over a const array */
	typedef array_iter_base<T const, false> const_reverse_iterator;

	/** \brief Iterator over an array */
	typedef array_iter_base<T, true> iterator;

	/** \brief Reverse iterator over an array */
	typedef array_iter_base<T, false> reverse_iterator;

	/** \brief Type of values containd in the array */
	typedef T value_type;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the i'th element of the array.
	///
	/// \param idx The index of the element we want an iterator to.
	/// \return An iterator to the i'th element.
	///////////////////////////////////////////////////////////////////////////
	iterator find(size_t idx) throw () {
		assert(idx <= size());
		return get_iter(idx);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the i'th element of the array.
	///
	/// \param idx The index of the element we want an iterator to.
	/// \return A const iterator to the i'th element.
	///////////////////////////////////////////////////////////////////////////
	const_iterator find(size_t idx) const throw () {
		assert(idx <= size());
		return get_iter(idx);
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the element located at the given index.
	///
	/// \param i The index of the element returned.
	///////////////////////////////////////////////////////////////////////////
	T & at(size_t i) throw() {
		assert(i < size());
		return m_elements[i];
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc tpie::array_facade::at(size_t i)
	///////////////////////////////////////////////////////////////////////////
	const T & at(size_t i) const throw() {
		assert(i < size());
		return m_elements[i];
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy elements from another array into this.
	///
	/// Note: This array is resized to the size of other.
	///
	/// \param other The array to copy from.
	/// \return A reference to this array.
	///////////////////////////////////////////////////////////////////////////
	array & operator=(const array & other) {
		resize(other.size());
		for (size_t i=0; i < size(); ++i) m_elements[i] = other[i];
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy elements from another array with any allocator into this.
	///
	/// Note: This array is resized to the size of other.
	///
	/// \param other The array to copy from.
	/// \tparam OtherAllocator  The allocator used by the other array.
	/// \return A reference to this array.
	///////////////////////////////////////////////////////////////////////////
	template <typename OtherAllocator>
	array & operator=(const array<T, OtherAllocator> & other) {
		resize(other.size());
		for (size_t i=0; i < size(); ++i) m_elements[i] = other[i];
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if the array is empty.
	///
	/// \return True if and only if size is 0.
	///////////////////////////////////////////////////////////////////////////
	inline bool empty() const {return size() == 0;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a const reference to an array entry.
	///
	/// \param i The index of the entry to return.
	/// \return Const reference to the entry.
	///////////////////////////////////////////////////////////////////////////
	inline const T & operator[](size_t i) const {
		assert(i < size());
		return at(i);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a reference to an array entry.
	///
	/// \param i The index of the entry to return.
	/// \return Reference to the entry.
	///////////////////////////////////////////////////////////////////////////
	inline T & operator[](size_t i) {
		assert(i < size());
		return at(i);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Compare if the other array has the same elements in the same
	/// order as this.
	///
	/// \param other The array to compare against.
	/// \return True if they are equal otherwise false.
	///////////////////////////////////////////////////////////////////////////
	inline bool operator==(const array & other) const {
		if (size() != other.size()) return false;
		for (size_t i=0;i<size();++i) if (*get_iter(i) != *other.get_iter(i)) return false;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if two arrays differ.
	///
	/// \param other The array to compare against.
	/// \return False if they are equal; otherwise true.
	///////////////////////////////////////////////////////////////////////////
	inline bool operator!=(const array & other) const {
		if (size() != other.size()) return true;
		for (size_t i=0; i<size(); ++i) if (*get_iter(i) != *other.get_iter(i)) return true;
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the beginning of the array.
	///
	/// \return An iterator to the beginning of the array.
	///////////////////////////////////////////////////////////////////////////
	inline iterator begin() {return get_iter(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the beginning of the array.
	///
	/// \return A const iterator to the beginning of the array.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator begin() const {return get_iter(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the end of the array.
	///
	/// \return An iterator to the end of the array.
	///////////////////////////////////////////////////////////////////////////
	inline iterator end() {return get_iter(size());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the end of the array.
	///
	/// \return A const iterator to the end of the array.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator end() const {return get_iter(size());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the first element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline const T & front() const {return at(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the first element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline T & front() {return at(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the last element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline const T & back() const {return at(size()-1);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the last element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline T & back() {return at(size()-1);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Reverse iterator to beginning of reverse sequence.
	///////////////////////////////////////////////////////////////////////////
	inline reverse_iterator rbegin() {return get_rev_iter(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Const reverse iterator to beginning of reverse sequence.
	///////////////////////////////////////////////////////////////////////////
	inline const_reverse_iterator rbegin() const {return get_rev_iter(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Reverse iterator to end of reverse sequence.
	///////////////////////////////////////////////////////////////////////////
	inline reverse_iterator rend() {return get_rev_iter(size());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Const reverse iterator to end of reverse sequence.
	///////////////////////////////////////////////////////////////////////////
	inline const_reverse_iterator rend() const {return get_rev_iter(size());}

private:
	T * m_elements;
	size_t m_size;

	inline iterator get_iter(size_t idx) {
		return iterator(m_elements+idx);
	}

	inline const_iterator get_iter(size_t idx) const {
		return const_iterator(m_elements+idx);
	}

	inline reverse_iterator get_rev_iter(size_t idx) {
		return reverse_iterator(m_elements+m_size-idx-1);
	}

	inline const_reverse_iterator get_rev_iter(size_t idx) const {
		return const_reverse_iterator(m_elements+m_size-idx-1);
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// \copydoc tpie::linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return (double)sizeof(T);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc tpie::linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {return sizeof(array);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array.
	/// \param value Each entry of the array is initialized with this value.
	///////////////////////////////////////////////////////////////////////////
	array(size_type s, const T & value,
		  const Allocator & alloc=Allocator()
		): m_elements(0), m_size(0), m_tss_used(false), m_allocator(alloc)
		{resize(s, value);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array.
	///////////////////////////////////////////////////////////////////////////
	array(size_type s=0, const Allocator & alloc=
		  Allocator()): m_elements(0), m_size(0), m_tss_used(false),
						m_allocator(alloc) {resize(s);}

	/////////////////////////////////////////////////////////
	/// \brief Construct a copy of another array.
	/// \param other The array to copy.
	/////////////////////////////////////////////////////////
	array(const array & other): m_elements(0), m_size(other.m_size), m_tss_used(false), m_allocator(other.m_allocator) {
		if (other.size() == 0) return;
		alloc_copy(other.m_elements);
	}

	array(const array_view_base<T> & view)
		: m_elements(0)
		, m_size(view.size())
		, m_tss_used(false)
	{
		if (view.size() == 0) return;
		alloc_copy(&*view.begin());
	}

	array(const array_view_base<const T> & view)
		: m_elements(0)
		, m_size(view.size())
		, m_tss_used(false)
	{
		if (view.size() == 0) return;
		alloc_copy(&*view.begin());
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Free up all memory used by the array.
	///////////////////////////////////////////////////////////////////////////
	~array() {resize(0);}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Change the size of the array.
	///
	/// All elements are lost.
	///
	/// Memory manager MUST be initialized at this point unless s == 0.
	///
	/// \param size The new size of the array.
	/// \param elm The initialization element.
	///////////////////////////////////////////////////////////////////////////
	void resize(size_t size, const T & elm) {
		if (size != m_size) {
			destruct_and_dealloc();
			m_size = size;

			alloc_fill(elm);
		} else {
			std::fill(m_elements+0, m_elements+m_size, elm);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Swap two arrays.
	///////////////////////////////////////////////////////////////////////////
	void swap(array & other) {
		std::swap(m_allocator, other.m_allocator);
		std::swap(m_elements, other.m_elements);
		std::swap(m_size, other.m_size);
		std::swap(m_tss_used, other.m_tss_used);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost.
	///
	/// Memory manager MUST be initialized at this point unless s == 0.
	///
	/// \param s The new size of the array.
	///////////////////////////////////////////////////////////////////////////
	void resize(size_t s) {
		destruct_and_dealloc();
		m_size = s;
		alloc_dfl();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the size of the array.
	///
	/// \return The size of the array.
	///////////////////////////////////////////////////////////////////////////
	inline size_type size() const {return m_size;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a raw pointer to the array content.
	///////////////////////////////////////////////////////////////////////////
	inline T * get() {return m_elements;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a raw pointer to the array content.
	///////////////////////////////////////////////////////////////////////////
	inline const T * get() const {return m_elements;}

private:
	friend struct bits::allocator_usage<T, Allocator>;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Allocate m_size elements and copy construct contents with
	/// elements from other array.
	/// Precondition: m_elements == null pointer; m_size == no. of elements;
	/// Effect: Allocates the m_elements buffer.
	/// \param copy_from  Source elements in [copy_from, copy_from+m_size)
	///////////////////////////////////////////////////////////////////////////
	inline void alloc_copy(const T * copy_from) { bits::allocator_usage<T, Allocator>::alloc_copy(*this, copy_from); }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Allocate m_size elements and copy construct contents with
	/// element.
	/// Precondition: m_elements == null pointer; m_size == no. of elements;
	/// Effect: Allocates the m_elements buffer.
	/// \param elm  Element to pass to the copy constructor
	///////////////////////////////////////////////////////////////////////////
	inline void alloc_fill(const T & elm) { bits::allocator_usage<T, Allocator>::alloc_fill(*this, elm); }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Allocate m_size elements and default construct contents.
	/// Precondition: m_elements == null pointer; m_size == no. of elements;
	/// Effect: Allocates the m_elements buffer.
	///////////////////////////////////////////////////////////////////////////
	inline void alloc_dfl() { bits::allocator_usage<T, Allocator>::alloc_dfl(*this); }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Destruct and deallocate elements.
	/// Precondition: m_elements == pointer to buffer of m_size elements;
	/// Effect: m_elements == null pointer; does not modify m_size
	///////////////////////////////////////////////////////////////////////////
	inline void destruct_and_dealloc() { bits::allocator_usage<T, Allocator>::destruct_and_dealloc(*this); }

	/** Whether we allocated m_elements as a trivial_same_size<T> *.
	 * See the implementation note in the source for an explanation. */
	bool m_tss_used;

	Allocator m_allocator;
};

namespace bits {

template <typename T>
struct allocator_usage<T, allocator<T> > {
	static void alloc_copy(array<T, allocator<T> > & host, const T * copy_from) {
		host.m_elements = host.m_size ? reinterpret_cast<T*>(tpie_new_array<trivial_same_size<T> >(host.m_size)) : 0;
		host.m_tss_used = true;
		std::uninitialized_copy(copy_from+0, copy_from+host.m_size, host.m_elements+0);
	}

	static void alloc_fill(array<T, allocator<T> > & host, const T & elm) {
		host.m_elements = host.m_size ? reinterpret_cast<T*>(tpie_new_array<trivial_same_size<T> >(host.m_size)) : 0;
		host.m_tss_used = true;

		// call copy constructors manually
		std::uninitialized_fill(host.m_elements+0, host.m_elements+host.m_size, elm);
	}

	static void alloc_dfl(array<T, allocator<T> > & host) {
		host.m_elements = host.m_size ? tpie_new_array<T>(host.m_size) : 0;
		host.m_tss_used = false;
	}

	static void destruct_and_dealloc(array<T, allocator<T> > & host) {
		if (!host.m_tss_used) {
			// calls destructors
			tpie_delete_array(host.m_elements, host.m_size);
			return;
		}

		// call destructors manually
		for (size_t i = 0; i < host.m_size; ++i) {
			host.m_elements[i].~T();
		}
		tpie_delete_array(reinterpret_cast<trivial_same_size<T>*>(host.m_elements), host.m_size);
	}
};

template <typename T, typename Allocator>
struct allocator_usage {
	static void alloc_copy(array<T, Allocator> & host, const T * copy_from) {
		host.m_elements = host.m_size ? host.m_allocator.allocate(host.m_size) : 0;
		for (size_t i = 0; i < host.m_size; ++i) {
			host.m_allocator.construct(host.m_elements+i, copy_from[i]);
		}
	}

	static void alloc_fill(array<T, Allocator> & host, const T & elm) {
		host.m_elements = host.m_size ? host.m_allocator.allocate(host.m_size) : 0;
		for (size_t i = 0; i < host.m_size; ++i) {
			host.m_allocator.construct(host.m_elements+i, elm);
		}
	}

	static void alloc_dfl(array<T, Allocator> & host) {
		host.m_elements = host.m_size ? host.m_allocator.allocate(host.m_size) : 0;
		for (size_t i = 0; i < host.m_size; ++i) {
			host.m_allocator.construct(host.m_elements+i);
		}
	}

	static void destruct_and_dealloc(array<T, Allocator> & host) {
		for (size_t i = 0; i < host.m_size; ++i) {
			host.m_allocator.destroy(host.m_elements+i);
		}
		host.m_allocator.deallocate(host.m_elements, host.m_size);
	}
};

} // namespace bits

template <typename T>
std::ostream & operator<<(std::ostream & o, const array<T> & a) {
	o << "[";
	bool first=true;
	for(size_t i=0; i < a.size(); ++i) {
		if (first) first = false;
		else o << ", ";
		o << a[i];
	}
	return o << "]";
}

} // namespace tpie

namespace std {

///////////////////////////////////////////////////////////////////////////////
/// \brief Enable std::swapping two tpie::arrays.
///
/// Citing the C++11 draft N3242, dated 2011-02-28, 17.6.4.2.1, paragraph 1:
///
///    "The behavior of a C++ program is undefined if it adds declarations or
///     definitions to namespace std or to a namespace within namespace std
///     unless otherwise specified. A program may add a template specialization
///     for any standard library template to namespace std only if the
///     declaration depends on a user-defined type and the specialization meets
///     the standard library requirements for the original template and is not
///     explicitly prohibited."  [Footnote: "Any library code that instantiates
///     other library templates must be prepared to work adequately with any
///     user-supplied specialization that meets the minimum requirements of the
///     Standard."]
///
/// In other words, we are allowed to specialize std::swap.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
void swap(tpie::array<T> & a, tpie::array<T> & b) {
	a.swap(b);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief std::copy template specialization for tpie::array.
///////////////////////////////////////////////////////////////////////////////
template <typename TT1, bool forward1, typename TT2, bool forward2>
tpie::array_iter_base<TT2, forward2>
copy(tpie::array_iter_base<TT1, forward1> first,
     tpie::array_iter_base<TT1, forward1> last,
	 tpie::array_iter_base<TT2, forward2> d_first) {

	ptrdiff_t dist = copy(&*first, &*last, &*d_first) - &*d_first;
	return d_first + dist;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief std::copy template specialization for tpie::array as input.
///////////////////////////////////////////////////////////////////////////////
template <typename TT, bool forward, typename OutputIterator>
OutputIterator
copy(tpie::array_iter_base<TT, forward> first,
     tpie::array_iter_base<TT, forward> last,
	 OutputIterator d_first) {

	return copy(&*first, &*last, d_first);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief std::copy template specialization for tpie::array as output.
///////////////////////////////////////////////////////////////////////////////
template <typename TT, bool forward, typename InputIterator>
tpie::array_iter_base<TT, forward>
copy(InputIterator first,
	 InputIterator last,
	 tpie::array_iter_base<TT, forward> d_first) {

	ptrdiff_t dist = copy(first, last, &*d_first) - &*d_first;
	return d_first + dist;
}

} // namespace std

#endif //__TPIE_ARRAY_H__ 	
