// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file pq_merge_heap.h Priority queue merge heap.
/// \sa \ref priority_queue.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_PQ_MERGE_HEAP_H_
#define _TPIE_PQ_MERGE_HEAP_H_

#include "tpie_log.h"
#include <cassert>
#include <tpie/memory.h>

namespace tpie{

///////////////////////////////////////////////////////////////////////////////
/// \class pq_merge_heap
/// \author Lars Hvam Petersen
///
/// pq_merge_heap
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename Comparator = std::less<T> >
class pq_merge_heap {
	public:
		typedef memory_size_type run_type;

		///////////////////////////////////////////////////////////////////////
		/// \brief Constructor.
		///
		/// \param elements Maximum allowed size of the heap.
		///////////////////////////////////////////////////////////////////////
		pq_merge_heap(memory_size_type elements);

		///////////////////////////////////////////////////////////////////////
		/// \brief Destructor.
		///////////////////////////////////////////////////////////////////////
		~pq_merge_heap();

		///////////////////////////////////////////////////////////////////////
		/// \brief Insert an element into the priority queue.
		///
		/// \param x The item.
		/// \param run Where it comes from.
		///////////////////////////////////////////////////////////////////////
		void push(const T& x, run_type run);

		///////////////////////////////////////////////////////////////////////
		/// \brief Remove the top element from the priority queue.
		///////////////////////////////////////////////////////////////////////
		void pop();

		///////////////////////////////////////////////////////////////////////
		/// \brief Remove the top element from the priority queue and insert
		/// another.
		///
		/// \param x The item.
		/// \param run Where it comes from.
		///////////////////////////////////////////////////////////////////////
		void pop_and_push(const T& x, run_type run);

		///////////////////////////////////////////////////////////////////////
		/// \brief See what's on the top of the priority queue.
		///
		/// \return Top element
		///////////////////////////////////////////////////////////////////////
		const T& top() const;

		///////////////////////////////////////////////////////////////////////
		/// \brief Return top element run number.
		///
		/// \return Top element run number.
		///////////////////////////////////////////////////////////////////////
		run_type top_run() const;

		///////////////////////////////////////////////////////////////////////
		/// \brief Returns the size of the queue.
		///
		/// \return Queue size.
		///////////////////////////////////////////////////////////////////////
		memory_size_type size() const;

		///////////////////////////////////////////////////////////////////////
		/// \brief Return true if queue is empty, otherwise false.
		///
		/// \return true if queue is empty, otherwise false.
		///////////////////////////////////////////////////////////////////////
		bool empty() const;

	private:
		void fixDown();
		void validate();
		void dump();

		memory_size_type m_size;
		Comparator comp_;

		T* heap;
		run_type* runs;
		memory_size_type maxsize;
};

#include "pq_merge_heap.inl"
}

#endif

