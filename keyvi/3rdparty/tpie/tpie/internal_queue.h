// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2010, 2011, The TPIE development team
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
#ifndef __TPIE_INTERNAL_QUEUE_H__
#define __TPIE_INTERNAL_QUEUE_H__

///////////////////////////////////////////////////////////////////////////////
/// \file internal_queue.h
/// Generic internal queue with known memory requirements.
///////////////////////////////////////////////////////////////////////////////
#include <tpie/array.h>
#include <tpie/util.h>
#include <tpie/tpie_assert.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief A generic internal circular queue
///
/// The queue supports a fixed number of unpopped elements between
/// calls to clear. The number of elements is given as an argument
/// to the constructor or to resize.
///
/// \tparam T The type of items stored in the queue
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class internal_queue: public linear_memory_base<internal_queue<T> > {
	array<T> m_elements;
	size_t m_first, m_last;
public:
	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {return array<T>::memory_coefficient();}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {return array<T>::memory_overhead() - sizeof(array<T>) + sizeof(internal_queue);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a queue.
	///
	/// \param size The number of pushes supported between calls to clear and
	/// resize.
	///////////////////////////////////////////////////////////////////////////
	internal_queue(size_t size=0): m_first(0), m_last(0) {m_elements.resize(size);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Resize the queue; all data is lost.
	///
	/// \param size The number of elements to contain.
	///////////////////////////////////////////////////////////////////////////
	void resize(size_t size=0) {m_elements.resize(size); m_first = m_last = 0;}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the item that has been in the queue for the longest time.
	///////////////////////////////////////////////////////////////////////////
	const T & front() const {tp_assert(!empty(), "front() on empty queue"); return m_elements[m_first % m_elements.size()];}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the last item pushed to the queue.
	///////////////////////////////////////////////////////////////////////////
	const T & back() const {return m_elements[(m_last-1) % m_elements.size()];}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Add an element to the front of the queue.
	///
	/// \param val The element to add.
	///////////////////////////////////////////////////////////////////////////
	inline void push(T val){m_elements[m_last++ % m_elements.size()] = val;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Remove an element from the back of the queue.
	///////////////////////////////////////////////////////////////////////////
	inline void pop(){++m_first;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if the queue is empty.
	/// \return true if the queue is empty, otherwise false.
	///////////////////////////////////////////////////////////////////////////
	inline bool empty() const {return m_first == m_last;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if the queue is empty.
	/// \return true if the queue is empty, otherwise false.
	///////////////////////////////////////////////////////////////////////////
	inline bool full() const {return m_last - m_first == m_elements.size();}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the number of elements in the queue.
	/// \return The number of elements in the queue.
	///////////////////////////////////////////////////////////////////////////
	inline size_t size() const { return m_last - m_first;}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Clear the queue of all elements.
	/// After this call, the queue again supports the number of pushes passed
	/// to the constructor or resize.
	///////////////////////////////////////////////////////////////////////////
	inline void clear(){m_first = m_last =0;}
}; // class internal_queue

} // namespace tpie
#endif //__TPIE_INTERNAL_QUEUE_H__

