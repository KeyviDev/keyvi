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
/// \file priority_queue.h
/// \brief External memory priority queue implementation.
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_PRIORITY_QUEUE_H_
#define _TPIE_PRIORITY_QUEUE_H_

#include <tpie/config.h>
#include "portability.h"
#include "tpie_log.h"
#include <cassert>
#include "pq_overflow_heap.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <string>
#include <cstring> // for memcpy
#include <sstream>
#include "pq_merge_heap.h"
#include <tpie/err.h>
#include <tpie/stream.h>
#include <tpie/array.h>
#include <boost/filesystem.hpp>

namespace tpie {

	struct priority_queue_error : public std::logic_error {
		priority_queue_error(const std::string& what) : std::logic_error(what)
		{ }
	};

///////////////////////////////////////////////////////////////////////////////
/// \class priority_queue
/// \brief External memory priority queue implementation. The top of the 
/// queue is the least element in the specified ordering.
///
/// Originally implemented by Lars Hvam Petersen for his Master's thesis
/// titled "External Priority Queues in Practice", June 2007.
/// This implementation, named "PQSequence3", is the fastest among the
/// priority queue implementations studied in the paper.
/// Inspiration: Sanders - Fast priority queues for cached memory (1999).
///
/// For an overview of the algorithm, refer to Sanders (1999) section 2 and
/// figure 1, or Lars Hvam's thesis, section 4.4.
///
/// In the debug log, the priority queue reports two values setting_k and
/// setting_m. The priority queue has a maximum capacity which is on the order
/// of setting_m * setting_k**setting_k elements (where ** denotes
/// exponentiation).
///
/// However, even with as little as 8 MB of memory, this maximum capacity in
/// practice exceeds 2**48, corresponding to a petabyte-sized dataset of 32-bit
/// integers.
///////////////////////////////////////////////////////////////////////////////

template<typename T, typename Comparator = std::less<T>, typename OPQType = pq_overflow_heap<T, Comparator> >
class priority_queue {
	typedef memory_size_type group_type;
	typedef memory_size_type slot_type;
public:
	static constexpr float default_blocksize = 0.0625; 
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///
	/// \param f Factor of memory that the priority queue is allowed to use.
	/// \param b Block factor
	///////////////////////////////////////////////////////////////////////////
	priority_queue(double f=1.0, float b=default_blocksize, stream_size_type n = std::numeric_limits<stream_size_type>::max());

#ifndef DOXYGEN
	// \param mmavail Number of bytes the priority queue is allowed to use.
	// \param b Block factor
	priority_queue(memory_size_type mm_avail, float b=default_blocksize, stream_size_type n = std::numeric_limits<stream_size_type>::max());
#endif

	/////////////////////////////////////////////////////////
    ///
    /// Compute the maximal amount of memory it makes sence
	/// to give a queue that will contain atmount n elements
    ///
    /////////////////////////////////////////////////////////
	static memory_size_type memory_usage(stream_size_type n, float b=default_blocksize);
	
    /////////////////////////////////////////////////////////
    ///
    /// Destructor
    ///
    /////////////////////////////////////////////////////////
    ~priority_queue();

    /////////////////////////////////////////////////////////
    ///
    /// Insert an element into the priority queue
    ///
    /// \param x The item
    ///
    /////////////////////////////////////////////////////////
    void push(const T& x);

    /////////////////////////////////////////////////////////
    ///
    /// Remove the top element from the priority queue
    ///
    /////////////////////////////////////////////////////////
    void pop();

    /////////////////////////////////////////////////////////
    ///
    /// See what's on the top of the priority queue
    ///
    /// \return the least element in the specified ordering
    ///
    /////////////////////////////////////////////////////////
    const T& top();

    /////////////////////////////////////////////////////////
    ///
    /// Returns the size of the queue
    ///
    /// \return Queue size
    ///
    /////////////////////////////////////////////////////////
    stream_size_type size() const;

    /////////////////////////////////////////////////////////
    ///
    /// Return true if queue is empty otherwise false
    ///
    /// \return Boolean - empty or not
    ///
    /////////////////////////////////////////////////////////
    bool empty() const;

    /////////////////////////////////////////////////////////
    ///
    /// Pop all elements with priority equal to that of the
    /// top element, and process each by invoking f's call
    /// operator on the element.
    ///
    /// \param f - assumed to have a call operator with parameter of type T.
    ///
    /// \return The argument f
    ///
    /////////////////////////////////////////////////////////
    template <typename F> F pop_equals(F f);

private:
    Comparator comp_;
    T dummy;

    T min;
    bool min_in_buffer;

	/** Overflow priority queue (for buffering inserted elements). Capacity m. */
	tpie::unique_ptr<OPQType> opq;

	/** Deletion buffer containing the m' top elements in the entire structure. */
	tpie::array<T> buffer;

	/** Group buffers contain at most m elements all less or equal to elements
	 * in the corresponding group slots. Elements in group buffers are *not*
	 * repeated in actual group slots. For efficiency, we keep group buffer 0
	 * in memory. */
	tpie::array<T> gbuffer0;

	/** Merge buffer of size 2*m. */
	tpie::array<T> mergebuffer;

	/** 3*(#slots) integers. Slot i contains its elements in cyclic ascending order,
	 * starting at index slot_state[3*i]. Slot i contains slot_state[3*i+1] elements.
	 * Its data is in data file index slot_state[3*i+2]. */
	tpie::array<memory_size_type> slot_state;

	/** 2*(#groups) integers. Group buffer i has its elements in cyclic ascending order,
	 * starting at index group_state[2*i]. Gbuffer i contains group_state[2*i+1] elements. */
	tpie::array<memory_size_type> group_state;

	/** k, the fanout of each group and the max fanout R. */
	memory_size_type setting_k;
	/** Number of groups in use. */
	memory_size_type current_r;
	/** m, the size of a slot and the size of the group buffers. */
	memory_size_type setting_m;
	/** m', the size of the deletion buffer. */
	memory_size_type setting_mmark;

    memory_size_type slot_data_id;

    stream_size_type m_size;
    memory_size_type buffer_size;
    memory_size_type buffer_start;

	float block_factor;

	void init(memory_size_type mm_avail, stream_size_type n = std::numeric_limits<stream_size_type>::max() );

    void             slot_start_set(slot_type slot, memory_size_type n);
    memory_size_type slot_start(slot_type slot) const;
    void             slot_size_set(slot_type slot, memory_size_type n);
    memory_size_type slot_size(slot_type slot) const;
    void             group_start_set(group_type group, memory_size_type n);
    memory_size_type group_start(group_type group) const;
    void             group_size_set(group_type group, memory_size_type n);
    memory_size_type group_size(group_type group) const;

    array<temp_file> datafiles;
    array<temp_file> groupdatafiles;

    temp_file & slot_data(slot_type slotid);
    void slot_data_set(slot_type slotid, memory_size_type n);
    temp_file & group_data(group_type groupid);
    memory_size_type slot_max_size(slot_type slotid);
    void write_slot(slot_type slotid, T* arr, memory_size_type len);
    slot_type free_slot(group_type group);
    void empty_group(group_type group);
    void fill_buffer();
    void fill_group_buffer(group_type group);
    void compact(slot_type slot);
    void validate();
    void remove_group_buffer(group_type group);
    void dump();
};

#include "priority_queue.inl"

    namespace ami {
		using tpie::priority_queue;
    }  //  ami namespace

}  //  tpie namespace

#endif
