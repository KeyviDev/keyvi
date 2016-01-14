// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

#ifndef __ARRAY_VIEW_BASE_H__
#define  __ARRAY_VIEW_BASE_H__

///////////////////////////////////////////////////////////////////////////////
/// \file array_view_base.h  Base class for array_view. This is needed so that
/// a tpie::array can be constructed from an array_view_base and an array_view
/// can be constructed from a tpie::array.
///////////////////////////////////////////////////////////////////////////////

#include <boost/iterator/iterator_facade.hpp>
#include <cassert>
namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class for array_view. This is needed so that a tpie::array can
/// be constructed from an array_view_base and an array_view can be constructed
/// from a tpie::array.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class array_view_base {
private:
	T * m_start;
	T * m_end;
public:
	class iterator: public boost::iterator_facade<iterator, T, boost::random_access_traversal_tag> {
	private:
		friend class array_view_base;
		friend class boost::iterator_core_access;
		explicit iterator(T * e): elm(e) {}
		inline T & dereference() const {return * elm;}
		inline bool equal(iterator const& o) const {return elm == o.elm;}
		inline void increment() {++elm;}
		inline void decrement() {--elm;}
		inline void advance(size_t n) {elm += n;}
		inline ptrdiff_t distance_to(iterator const & o) const {return o.elm - elm;}
		T * elm;
	public:
		iterator(): elm(0) {};
	};


	///////////////////////////////////////////////////////////////////////////
	/// \brief Pointer constructor. The structure will produce the elements in
	/// the memory range [start, end).
	/// \param start Pointer to first element of the array_view.
	/// \param end Pointer past the last element of the array_view.
	///////////////////////////////////////////////////////////////////////////
	array_view_base(T * start, T * end): m_start(start), m_end(end) {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Type of values contained in the array.
	///////////////////////////////////////////////////////////////////////////
	typedef T value_type;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the i'th element of the array.
	///
	/// \param i The index of the element we want an iterator to.
	/// \return An iterator to the i'th element.
	///////////////////////////////////////////////////////////////////////////
	iterator find(size_t idx) const throw () {
		assert(idx <= size());
		return iterator(m_start + idx);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the element located at the given index.
	///
	/// \param i The index of the element returned.
	///////////////////////////////////////////////////////////////////////////
	T & at(size_t i) const throw() {
		assert(i < size());
		return *find(i);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if the array is empty.
	///
	/// \return True if and only if size is 0.
	///////////////////////////////////////////////////////////////////////////
	inline bool empty() const {return m_end == m_start;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get number of elements in the array.
	/// \return Number of elements in the array.
	///////////////////////////////////////////////////////////////////////////
	inline size_t size() const {return m_end - m_start;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a reference to an array entry.
	///
	/// \param i The index of the entry to return.
	/// \return Reference to the entry.
	///////////////////////////////////////////////////////////////////////////
	inline T & operator[](size_t i) const {
		assert(i < size());
		return at(i);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if the other array has the same elements in the same order
	/// as this.
	///
	/// \param other The array to compare against.
	/// \return True if they are equal, otherwise false.
	///////////////////////////////////////////////////////////////////////////
	inline bool operator==(const array_view_base & other) const {
 		if (size() != other.size()) return false;
		for (size_t i=0; i < size(); ++i) if (at(i) != other.at(i)) return false;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if the two arrays differ.
	///
	/// \param other The array to compare against.
	/// \return false If they are equal otherwise true.
	///////////////////////////////////////////////////////////////////////////
	inline bool operator!=(const array_view_base & other) const {
		if (size() != other.size()) return true;
		for (size_t i=0; i< size(); ++i) if (at(i) != other.at(i)) return true;
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the beginning of the array.
	///
	/// \return An iterator to the beginning of the array.
	///////////////////////////////////////////////////////////////////////////
	inline iterator begin() const {return iterator(m_start);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the end of the array.
	///
	/// \return An iterator to the end of the array.
	///////////////////////////////////////////////////////////////////////////
	inline iterator end() const {return iterator(m_end);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the first element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline T & front() const {return *m_start;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the last element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline T & back() const {return *(m_end-1);}
};

} //namespace tpie

#endif //__ARRAY_VIEW_BASE_H__
