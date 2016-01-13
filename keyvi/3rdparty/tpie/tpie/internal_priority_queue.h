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

#ifndef __TPIE_INTERNAL_PRIORITY_QUEUE_H__
#define __TPIE_INTERNAL_PRIORITY_QUEUE_H__
#include <tpie/array.h>
#include <algorithm>
#include <tpie/util.h>
namespace tpie {
///////////////////////////////////////////////////////////////////////////////
/// \file internal_priority_queue.h
/// \brief Simple heap based priority queue implementation.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \class internal_priority_queue
/// \author Lars Hvam Petersen, Jakob Truelsen
/// \brief Standard binary internal heap.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename comp_t = std::less<T> >
class internal_priority_queue: public linear_memory_base< internal_priority_queue<T, comp_t> > {
public:
	typedef memory_size_type size_type;

	///////////////////////////////////////////////////////////////////////////
    /// \brief Construct a priority queue.
    /// \param max_size Maximum size of queue.
	///////////////////////////////////////////////////////////////////////////
    internal_priority_queue(size_type max_size, comp_t c=comp_t(),
							memory_bucket_ref bucket = memory_bucket_ref())
		: pq(max_size, bucket), sz(0), comp(c) {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a priority queue with given elements.
	///////////////////////////////////////////////////////////////////////////
	template <typename IT>
	internal_priority_queue(size_type max_size, const IT & start, const IT & end,
							comp_t c=comp_t(),
							memory_bucket_ref bucket = memory_bucket_ref())
		: pq(max_size, bucket), sz(0), comp(c) {
		insert(start, end);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Insert some elements and run make_heap.
	///////////////////////////////////////////////////////////////////////////
	template <typename IT>
	void insert(const IT & start, const IT & end) {
		std::copy(start, end, pq.find(sz));
		sz += (end - start);
		make_safe();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Insert an element into the priority queue, possibly destroying
	/// ordering information.
    ///
    /// \param v The element that should be inserted.
	///////////////////////////////////////////////////////////////////////////
	void unsafe_push(const T & v) { 
		pq[sz++] = v;
	}

	void unsafe_push(T && v) {
		pq[sz++] = std::move(v);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Make the priority queue safe after a sequence of calls to
	/// unsafe_push.
	///////////////////////////////////////////////////////////////////////////
	void make_safe() {
		std::make_heap(pq.begin(), pq.find(sz), comp);
	}
  
	///////////////////////////////////////////////////////////////////////////
    /// \brief Is the queue empty?
    /// \return True if the queue is empty.
	///////////////////////////////////////////////////////////////////////////
    bool empty() const {return sz == 0;}

	///////////////////////////////////////////////////////////////////////////
    /// \brief Returns the size of the queue.
    /// \return Queue size.
	///////////////////////////////////////////////////////////////////////////
    size_type size() const {return sz;}

	///////////////////////////////////////////////////////////////////////////
    /// \brief Insert an element into the priority queue.
    ///
    /// \param v The element that should be inserted.
	///////////////////////////////////////////////////////////////////////////
    void push(const T & v) { 
		assert(size() < pq.size());
		pq[sz++] = v; 
		std::push_heap(pq.begin(), pq.find(sz), comp);
    }

	void push(T && v) {
		assert(size() < pq.size());
		pq[sz++] = std::move(v);
		std::push_heap(pq.begin(), pq.find(sz), comp);
    }

	///////////////////////////////////////////////////////////////////////////
    /// \brief Remove the minimum element from heap.
	///////////////////////////////////////////////////////////////////////////
    void pop() { 
		assert(!empty());
		std::pop_heap(pq.begin(), pq.find(sz), comp);
		--sz;
    }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Remove the minimum element and insert a new element.
	///////////////////////////////////////////////////////////////////////////
	void pop_and_push(const T & v) {
		assert(!empty());
		pq[0] = v;
		pop_and_push_heap(pq.begin(), pq.find(sz), comp);
	}

	void pop_and_push(T && v) {
		assert(!empty());
		pq[0] = std::move(v);
		pop_and_push_heap(pq.begin(), pq.find(sz), comp);
	}

	///////////////////////////////////////////////////////////////////////////
    /// \brief Return the minimum element.
    ///
    /// \return The minimum element.
	///////////////////////////////////////////////////////////////////////////
    const T & top() const {return pq[0];}

	T & top() {return pq[0];}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return tpie::array<T>::memory_coefficient();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {
		return tpie::array<T>::memory_overhead() - sizeof(tpie::array<T>) + sizeof(internal_priority_queue);
	}

	///////////////////////////////////////////////////////////////////////////
    /// \brief Return the underlying array.
	/// Make sure you know what you are doing.
    /// \return The underlying array.
	///////////////////////////////////////////////////////////////////////////
	tpie::array<T> & get_array() {
		return pq;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Clear the structure of all elements.
	///////////////////////////////////////////////////////////////////////////
	void clear() {sz=0;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Resize priority queue to given size.
	/// \param s New size of priority queue.
	///////////////////////////////////////////////////////////////////////////
	void resize(size_t s) {sz=0; pq.resize(s);}
private:	
	tpie::array<T> pq; 
    size_type sz;
	binary_argument_swap<comp_t> comp;
};

}  //  tpie namespace
#endif //__TPIE_INTERNAL_PRIORITY_QUEUE_H__
