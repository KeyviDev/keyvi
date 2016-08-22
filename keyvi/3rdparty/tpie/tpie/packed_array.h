// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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
#ifndef __TPIE_PACKED_ARRAY_H__
#define __TPIE_PACKED_ARRAY_H__

#include <tpie/config.h>
#include <tpie/util.h>
#include <iterator>
#include <cassert>

///////////////////////////////////////////////////////////////////////////
/// \file packed_array.h
/// Packed array with known memory requirements.
///////////////////////////////////////////////////////////////////////////

namespace tpie {

/////////////////////////////////////////////////////////
/// \internal
/// \brief Base class for the iterators
/// \tparam CT CRTP child class
/// \tparam forward Is this a forward iterator?
/////////////////////////////////////////////////////////
template <typename CT, bool forward, typename RT>
class packed_array_iter_facade {
private:
	CT & self() {return *reinterpret_cast<CT*>(this);}
	template <typename, bool, typename> friend class packed_array_iter_facade;

public:
	const CT & self() const {return *reinterpret_cast<const CT*>(this);}

	typedef ptrdiff_t difference_type;
	typedef std::random_access_iterator_tag iterator_category;
	template <typename TT>
	bool operator==(const TT & o) const {return (self()-o) == 0;}
	template <typename TT>
	bool operator!=(const TT & o) const {return (self()-o) != 0;}
	CT & operator++() {self().index() += forward?1:-1; return self();}
	CT operator++(int) {CT x=self(); ++self(); return x;}
	CT & operator--() {self().index() += forward?-1:1; return self();}
	CT operator--(int) {CT x=self(); --self(); return x;}
	bool operator<(const CT & o)  const {return (self()-o) < 0;}
	bool operator>(const CT & o)  const {return (self()-o) > 0;}
	bool operator<=(const CT & o) const {return (self()-o) <= 0;}
	bool operator>=(const CT & o) const {return (self()-o) >= 0;}
	ptrdiff_t operator-(const CT & o) const {return forward ? (self().index() - o.index()) : (o.index() - self().index());}
	CT operator+(difference_type n) const {CT x=self(); return x += n;}
	CT operator-(difference_type n) const {CT x=self(); return x -= n;}
	CT & operator+=(difference_type n) {self().index() += (forward?n:-n); return self();}
	CT & operator-=(difference_type n) {self().index() += (forward?n:-n); return self();}
	RT operator[](difference_type n) {return *(self() + n);}
};

template <typename CT, bool f, typename RT>
CT operator+(ptrdiff_t n, const packed_array_iter_facade<CT, f, RT> & i) {
	CT tmp(i.self());
	tmp += n;
	return tmp;
}

///////////////////////////////////////////////////////////////////////////
/// \brief An array storring elements of type T using B
/// bits to  to store a element.
///
/// T must be either bool or int. B must devide the word size, (XXX why?)
/// in reality only 1, 2 or 4 seems usamle
///
/// \tparam T The type of elements to store in the array
/// \tparam B The number of bits used to store a single element
///////////////////////////////////////////////////////////////////////////
template <typename T, int B>
class packed_array: public linear_memory_base<packed_array<T, B> >{
public:
	typedef size_t storage_type;
private:
	// TODO is this necessary?
	static_assert(sizeof(storage_type) * 8 % B == 0, "Bad storrage");

	///////////////////////////////////////////////////////////////////////////
	/// \brief Logical elements per physical element.
	///////////////////////////////////////////////////////////////////////////
	static size_t perword() {
		return sizeof(storage_type) * 8 / B;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Physical element index that contains a given logical element.
	///////////////////////////////////////////////////////////////////////////
	static size_t high(size_t index) {
		return index/perword();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Bit position where a given logical element resides.
	///////////////////////////////////////////////////////////////////////////
	static size_t low(size_t index) {
		return B*(index%perword());
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Number of physical elements needed to store m logical elements.
	///////////////////////////////////////////////////////////////////////////
	static size_t words(size_t m) {
		return (perword()-1+m)/perword();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Mask that will extract the lowest order logical element.
	///////////////////////////////////////////////////////////////////////////
	static storage_type mask() {
		return (1 << B)-1;
	}

	template <bool forward>
	class const_iter_base;

	template <bool forward>
	class iter_base;

	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief Iterator and reverse iterator will return an element
	/// of this type, it can be cast to a T, and assigned a T value
	/////////////////////////////////////////////////////////
	class iter_return_type {
	private:
		iter_return_type(storage_type * e, size_t i): elms(e), index(i) {}
		storage_type * elms;
		size_t index;
	public:
		template <typename, int> friend class packed_array;
		operator T() const {return static_cast<T>((elms[high(index)] >> low(index))&mask());}
	 	iter_return_type & operator=(const T b) {
			storage_type * p = elms+high(index);
			size_t i = low(index);
			*p = (*p & ~(mask()<<i)) | ((b & mask()) << i);
	 		return *this;
		}
	};

	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief Base class for iterator and reverse iterator
	/// \tparam forward Is this a forward iterator?
	/////////////////////////////////////////////////////////
	template <bool forward>
	class iter_base: public packed_array_iter_facade<iter_base<forward>, forward, iter_return_type> {
	private:
		iter_return_type elm;		
		
		friend class const_iter_base<forward>;
		friend class packed_array;
		template <typename, bool, typename> friend class packed_array_iter_facade;
		iter_base(storage_type * elms, size_t index): elm(elms, index) {};

		size_t & index() {return elm.index;}
		const size_t & index() const {return elm.index;}
	public:
		typedef iter_return_type value_type;
		typedef iter_return_type & reference;
		typedef iter_return_type * pointer;
		
		iter_return_type & operator*() {return elm;}
		iter_return_type * operator->() {return &elm;}
		iter_base & operator=(const iter_base & o) {elm.index = o.elm.index; elm.elms=o.elm.elms; return *this;}
		iter_base(iter_base const &o): elm(o.elm) {};
	};
	
	typedef T vssucks;
	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief Base class for const ierator and const reverse iterator
	/// \tparam forward Is this a forward iterator?
	/////////////////////////////////////////////////////////
	template <bool forward>
	class const_iter_base: public packed_array_iter_facade<const_iter_base<forward>, forward, vssucks> {
	private:
		const storage_type * elms;
		size_t idx;
		
		friend class packed_array;
		friend class boost::iterator_core_access;
		template <typename, bool, typename> friend class packed_array_iter_facade;
		const_iter_base(const storage_type * e, size_t i): elms(e), idx(i) {}

		size_t & index() {return idx;}
		const size_t & index() const {return idx;}
	public:
		typedef vssucks value_type;
		typedef vssucks reference;
		typedef vssucks * pointer;

		const_iter_base & operator=(const const_iter_base & o) {idx = o.idx; elms=o.elms; return *this;}
		vssucks operator*() const {return static_cast<T>(elms[high(idx)] >> low(idx) & mask());}
		const_iter_base(const_iter_base const& o): elms(o.elms), idx(o.idx) {}
		const_iter_base(iter_base<forward> const& o): elms(o.elm.elms), idx(o.elm.index) {}
	};		


	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief This type is returned by the [] operator.
	/// It can be cast to a T, and assigned a T value.
	/////////////////////////////////////////////////////////
	struct return_type{
	private:
		storage_type * p;
	 	size_t i;
		return_type(storage_type * p_, size_t i_): p(p_), i(i_) {}
		friend class packed_array;
	public:
	 	operator T() const {return static_cast<T>((*p >> i) & mask());}
	 	return_type & operator=(const T b) {
			*p = (*p & ~(mask()<<i)) | ((static_cast<const storage_type>(b) & mask()) << i);
	 		return *this;
		}
	 	return_type & operator=(const return_type & t){
	 		*this = (T) t;
	 		return *this;
	 	}
	};

	storage_type * m_elements;
	size_t m_size;
public:		
	/////////////////////////////////////////////////////////
	/// \brief Type of values containd in the array
	/////////////////////////////////////////////////////////
	typedef T value_type;
	
	/////////////////////////////////////////////////////////
	/// \brief Iterator over a const array
	/////////////////////////////////////////////////////////
	typedef const_iter_base<true> const_iterator;

	/////////////////////////////////////////////////////////
	/// \brief Reverse iterator over a const array
	/////////////////////////////////////////////////////////
	typedef const_iter_base<false> const_reverse_iterator;

	/////////////////////////////////////////////////////////
	/// \brief Iterator over an array
	/////////////////////////////////////////////////////////
	typedef iter_base<true> iterator;

	/////////////////////////////////////////////////////////
	/// \brief Reverse iterator over an array
	/////////////////////////////////////////////////////////
	typedef iter_base<false> reverse_iterator;

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	/////////////////////////////////////////////////////////
	static double memory_coefficient(){
		return B/8.0;
	}
	
	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	/////////////////////////////////////////////////////////
	static double memory_overhead() {
		return (double)sizeof(packed_array)+(double)sizeof(storage_type);
	}	

	/////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// The elements have undefined values
	/// \param s The number of elements in the array
	/////////////////////////////////////////////////////////
	packed_array(size_t s=0): m_elements(nullptr), m_size(0) {resize(s);}

	/////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array
	/// \param value Each entry of the array is initialized with this value
	/////////////////////////////////////////////////////////
	packed_array(size_t s, T value): m_elements(nullptr), m_size(0) {resize(s,value);}

	/////////////////////////////////////////////////////////
	/// \brief Construct a copy of another array
	/// \param other The array to copy
	/////////////////////////////////////////////////////////
	packed_array(const packed_array & a): m_elements(nullptr), m_size(0) {*this=a;}

	/////////////////////////////////////////////////////////
	/// \brief Move another aray into me
	/// \param other The array to copy
	/////////////////////////////////////////////////////////
	packed_array(packed_array && a): m_elements(a.m_elements), m_size(a.m_size) {
		a.m_elements = nullptr;
		a.m_size = 0;
	}
	
	/////////////////////////////////////////////////////////
	/// \brief Free up all memory used by the array
	/////////////////////////////////////////////////////////
	~packed_array() {resize(0);}

	/////////////////////////////////////////////////////////
	/// \brief Copy elements from another array into this.
	///
	/// Note this array is resized to the size of other
	/// \param other The array to copy from
	/// \return a reference to this array
	/////////////////////////////////////////////////////////
	packed_array & operator=(const packed_array & a) {
		resize(a.m_size);
		for(size_t i=0;i<words(m_size);++i)
			m_elements[i] = a.m_elements[i];
		return *this;
	}

	/////////////////////////////////////////////////////////
	/// \brief Move another array
	///
	/////////////////////////////////////////////////////////
	packed_array & operator=(packed_array && a) {
		resize(0);
		std::swap(m_elements, a.m_elements);
		std::swap(m_size, a.m_size);
		return *this;
	}


	/////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost, after resize the value of the entries
	/// is undefined
	/// \param s the new size of the array
	/////////////////////////////////////////////////////////
	void resize(size_t s) {
		if (s == m_size) return;
		tpie_delete_array(m_elements, words(m_size));
		m_size = s;
		m_elements = m_size?tpie_new_array<storage_type>(words(m_size)):nullptr;
	}

	/////////////////////////////////////////////////////////
	/// \brief Fill the entier array with the given value
	///
	/// \param elm the initialization element
	/////////////////////////////////////////////////////////
	void fill(T value) {
		storage_type x=0;
		for (size_t i=0; i < perword(); ++i) 
			x = (x << B) + ((storage_type)value & mask());
		for (size_t i=0; i < words(m_size); ++i)
			m_elements[i] = x;		
	}
	
	
	/////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost, after resize the value of the entries
	/// is initialized by a copy of elm
	/// \param s the new size of the array
	/// \param value the initialization element
	/////////////////////////////////////////////////////////
	void resize(size_t s, T value) {
		resize(s);
		fill(value);
	}

	/////////////////////////////////////////////////////////
	/// \brief Return the size of the array
	///
	/// \return the size of the array
	/////////////////////////////////////////////////////////
	size_t size() const {return m_size;}

	/////////////////////////////////////////////////////////
	/// \brief Check if the array is empty
	///
	/// \return true if and only if size is 0
	/////////////////////////////////////////////////////////
	bool empty() const {return m_size ==0;}

	/////////////////////////////////////////////////////////
	/// \brief Return an array entry
	///
	/// \param i the index of the entry to return
	/// \return the array entry
	/////////////////////////////////////////////////////////
	T operator[](size_t t)const {
		assert(t < m_size);
		return static_cast<T>((m_elements[high(t)] >> low(t))&mask());
	}	
	
	/////////////////////////////////////////////////////////
	/// \brief Return a object behaving like a reference to an array entry
	///
	/// \param i the index of the entry to return
	/// \return The object behaving like a reference
	/////////////////////////////////////////////////////////
	return_type operator[](size_t t) {
		assert(t < m_size);
		return return_type(m_elements+high(t), low(t));
	}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return an iterator to the i'th element
	/////////////////////////////////////////////////////////
	iterator find(size_type i) {return iterator(m_elements,i);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return a const iterator to the i'th element
	/////////////////////////////////////////////////////////
	const_iterator find(size_type i) const {return const_iterator(m_elements,i);}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the beginning of the array
	///
	/// \return an iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	iterator begin() {return iterator(m_elements,0);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the beginning of the array
	///
	/// \return a const iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	const_iterator begin() const {return const_iterator(m_elements,0);}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the end of the array
	///
	/// \return an iterator tho the end of the array
	/////////////////////////////////////////////////////////
	iterator end() {return iterator(m_elements,m_size);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the end of the array
	///
	/// \return a const iterator tho the end of the array
	/////////////////////////////////////////////////////////
	const_iterator end() const {return const_iterator(m_elements,m_size);}

	//We use m_elements -1 as basic for the reverse operators
	//To make sure that the index of rend is positive
	reverse_iterator rbegin() {return reverse_iterator(m_elements-1, perword()+m_size-1);}
	const_reverse_iterator rbegin() const {return const_reverse_iterator(m_elements-1, perword()+m_size-1);}
	reverse_iterator rend() {return reverse_iterator(m_elements-1, perword()-1);}
	const_reverse_iterator rend() const {return const_reverse_iterator(m_elements-1, perword()-1);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Swap two arrays.
	///////////////////////////////////////////////////////////////////////////
	void swap(packed_array & o) {
		std::swap(m_elements, o.m_elements);
		std::swap(m_size, o.m_size);
	}
};

} //namespace tpie

namespace std {
template <typename T, int B>
void swap(tpie::packed_array<T, B> & a, tpie::packed_array<T, B> & b) {
	a.swap(b);
}
} //namespace std
#endif //__TPIE_PACKED_ARRAY_H__
