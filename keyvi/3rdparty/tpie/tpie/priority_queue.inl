// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2008, 2011, 2012, The TPIE development team
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

template<typename T, typename Comparator, typename OPQType>
priority_queue<T, Comparator, OPQType>::priority_queue(double f, float b, stream_size_type n) :
block_factor(b) { // constructor mem fraction
	assert(f<= 1.0 && f > 0);
	assert(b > 0.0);
	memory_size_type mm_avail = consecutive_memory_available();
	TP_LOG_DEBUG("priority_queue: Memory limit: " 
		<< mm_avail/1024/1024 << "mb("
		<< mm_avail << "bytes)" << "\n");
	mm_avail = static_cast<memory_size_type>(static_cast<double>(mm_avail)*f);
	init(mm_avail, n);
}

#ifndef DOXYGEN
template<typename T, typename Comparator, typename OPQType>
priority_queue<T, Comparator, OPQType>::priority_queue(memory_size_type mm_avail, float b, stream_size_type n) :
block_factor(b) { // constructor absolute mem
	assert(mm_avail <= get_memory_manager().limit() && mm_avail > 0);
	assert(b > 0.0);
	TP_LOG_DEBUG("priority_queue: Memory limit: " 
				 << mm_avail/1024/1024 << "mb("
				 << mm_avail << "bytes)" << "\n");
	init(mm_avail, n);
}
#endif


template<typename T, typename Comparator, typename OPQType>
memory_size_type priority_queue<T, Comparator, OPQType>::memory_usage(stream_size_type n, float) {
	if ( std::numeric_limits<memory_size_type>::max() / sizeof(T) < n)
		return std::numeric_limits<memory_size_type>::max();

	return n * sizeof(T);
}


template<typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::init(memory_size_type mm_avail, stream_size_type n) { // init
#ifdef _WIN32
#ifndef _WIN64
	mm_avail = std::min(mm_avail, static_cast<memory_size_type>(1024*1024*512));
#endif //_WIN64
#endif //_WIN32
	if (memory_usage(n, block_factor) <= mm_avail) {
		// Internal allocation
		opq.reset(tpie_new<OPQType>(n));
		current_r = 0;
		m_size = 0; // total size of priority queue
		buffer_size = 0;
		buffer_start = 0;
		return;
	}	

	TP_LOG_DEBUG("m_for_queue: " 
		<< mm_avail << "\n");
	TP_LOG_DEBUG("memory before alloc: " 
				 << get_memory_manager().available() << "b" << "\n");
	{
		//Calculate M
		setting_m = mm_avail/sizeof(T);
		//Get stream memory usage
		memory_size_type usage = file_stream<T>::memory_usage(block_factor);
		TP_LOG_DEBUG("Memory used by file_stream: " << usage << "b\n");

		memory_size_type alloc_overhead = 0;


		//Compute overhead of the parameters
		const memory_size_type fanout_overhead = 2*sizeof(stream_size_type)// group state
			+ (usage+sizeof(file_stream<T>*)+alloc_overhead) //temporary streams
			+ (sizeof(T)+sizeof(group_type)); //mergeheap
		const memory_size_type sq_fanout_overhead = 3*sizeof(stream_size_type); //slot_state
		const memory_size_type heap_m_overhead = sizeof(T) //opg
			+ sizeof(T) //gbuffer0
			+ sizeof(T) //extra buffer for remove_group_buffer
			+ 2*sizeof(T); //mergebuffer
		const memory_size_type buffer_m_overhead = sizeof(T) + 2*sizeof(T); //buffer
		const memory_size_type extra_overhead =
			  2*(usage+sizeof(file_stream<T>*)+alloc_overhead) //temporary streams
			+ 2*(sizeof(T)+sizeof(group_type)); //mergeheap
		const memory_size_type additional_overhead = 16*1024; //Just leave a bit unused
		TP_LOG_DEBUG("fanout_overhead     " << fanout_overhead     << ",\n" <<
		             "sq_fanout_overhead  " << sq_fanout_overhead  << ",\n" <<
		             "heap_m_overhead     " << heap_m_overhead     << ",\n" <<
		             "buffer_m_overhead   " << buffer_m_overhead   << ",\n" <<
		             "extra_overhead      " << extra_overhead      << ",\n" <<
		             "additional_overhead " << additional_overhead << ".\n\n");

		//Check that there is enough space for the simple overhead
		if(mm_avail < extra_overhead+additional_overhead){
			throw priority_queue_error("Not enough memory available for priority queue");
		}

		//Setup the fanout, heap_m and buffer_m
		mm_avail-=additional_overhead+extra_overhead; //Subtract the extra space used
		setting_mmark = (mm_avail/16)/buffer_m_overhead; //Set the buffer size
		TP_LOG_DEBUG("mm_avail      " << mm_avail << ",\n" <<
		             "setting_mmark " << setting_mmark  << ".\n\n");

		mm_avail-=setting_mmark*buffer_m_overhead;
		setting_k = (mm_avail/2); 
		TP_LOG_DEBUG("mm_avail      " << mm_avail << ",\n" <<
		             "setting_k     " << setting_k  << ".\n\n");

		{
			//compute setting_k
			//some of these numbers get big which is the reason for all this
			//careful casting.
			stream_size_type squared_tmp =
				static_cast<stream_size_type>(fanout_overhead)
				*static_cast<stream_size_type>(fanout_overhead);

			squared_tmp +=
				static_cast<stream_size_type>(4*sq_fanout_overhead)
				*static_cast<stream_size_type>(setting_k);

			long double dsquared_tmp = static_cast<long double>(squared_tmp);

			const stream_size_type root_discriminant =
				static_cast<stream_size_type>(std::floor(std::sqrt(dsquared_tmp)));

			const stream_size_type nominator = root_discriminant-fanout_overhead;
			const stream_size_type denominator = 2*sq_fanout_overhead;
			setting_k = static_cast<memory_size_type>(nominator/denominator); //Set fanout

			// Don't open too many files
			setting_k = std::min(get_file_manager().available(), setting_k);

			// Performance degrades with more than around 250 open files
			setting_k = std::min(static_cast<memory_size_type>(250), setting_k);
		}

		mm_avail-=setting_k*heap_m_overhead+setting_k*setting_k*sq_fanout_overhead;
		setting_m = (mm_avail)/heap_m_overhead;
		TP_LOG_DEBUG("mm_avail      " << mm_avail << ",\n" <<
		             "setting_m     " << setting_m << ",\n" <<
		             "setting_k     " << setting_k << ".\n\n");

		//Check that minimum requirements on fanout and buffersizes are met
		const memory_size_type min_fanout=3;
		const memory_size_type min_heap_m=4;
		const memory_size_type min_buffer_m=2;
		if(setting_k<min_fanout || setting_m<min_heap_m || setting_mmark<min_buffer_m){
			TP_LOG_FATAL_ID("Priority queue: Not enough memory. Increase allowed memory.");
			throw exception("Priority queue: Not enough memory. Increase allowed memory.");
		}

		// this is assumed in empty_group.
		assert(2*setting_m > sizeof(file_stream<T>) + setting_k*(sizeof(T) + sizeof(size_type)
		                                                         + sizeof(file_stream<T>)));

	}

	current_r = 0;
	m_size = 0; // total size of priority queue
	buffer_size = 0;
	buffer_start = 0;

	TP_LOG_DEBUG("priority_queue" << "\n"
			<< "\tsetting_k: " << setting_k << "\n"
			<< "\tsetting_mmark: " << setting_mmark << "\n"
			<< "\tsetting_m: " << setting_m << "\n");

	assert(setting_k > 0);
	assert(current_r == 0);
	assert(setting_m > 0);
	assert(setting_mmark > 0);
	assert(setting_m > setting_mmark);
	if(setting_m < setting_mmark) {
		TP_LOG_FATAL_ID("wrong settings");
		throw exception("Priority queue: m < m'");
	}

	opq.reset(tpie_new<OPQType>(setting_m));
	assert(OPQType::sorted_factor == 1);

	// state arrays contain: start + size
	slot_state.resize(setting_k*setting_k*3);
	group_state.resize(setting_k*2);

	buffer.resize(setting_mmark);
	gbuffer0.resize(setting_m);
	mergebuffer.resize(setting_m*2);

	// clear memory
	for(memory_size_type i = 0; i<setting_k*setting_k; i++) {
		slot_state[i*3] = 0;
		slot_state[i*3+1] = 0;
		slot_state[i*3+2] = i;
	}
	slot_data_id = setting_k*setting_k+1;

	for(memory_size_type i = 0; i< setting_k*2; i++) {
		group_state[i] = 0;
	}

	std::stringstream ss;
	ss << tempname::tpie_name("pq_data");
	datafiles.resize(setting_k*setting_k);
	groupdatafiles.resize(setting_k);
	TP_LOG_DEBUG("memory after alloc: " 
				 << get_memory_manager().available() << "b" << "\n");
}

template <typename T, typename Comparator, typename OPQType>
priority_queue<T, Comparator, OPQType>::~priority_queue() { // destructor
	datafiles.resize(0); // unlink slots
	groupdatafiles.resize(0); // unlink groups 

	buffer.resize(0);
	gbuffer0.resize(0);
	mergebuffer.resize(0);
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::push(const T& x) {

	if(opq->full()) {
		// When the overflow priority queue (aka. insertion buffer) is full,
		// insert its contents into a new slot in group 0.
		//
		// To maintain the heap invariant
		//     deletion buffer <= group buffer 0 <= group 0 slots
		// we bubble lesser elements from insertion buffer down into
		// deletion buffer and group buffer 0.

		slot_type slot = free_slot(0); // (if group 0 is full, we recursively empty group i
		                               // by merging it into a slot in group i+1)

		assert(opq->sorted_size() == setting_m);
		T* arr = opq->sorted_array();

		// Bubble lesser elements down into deletion buffer
		if(buffer_size > 0) {

			// fetch insertion buffer
			memcpy(&mergebuffer[0], &arr[0], sizeof(T)*opq->sorted_size());

			// fetch deletion buffer
			memcpy(&mergebuffer[opq->sorted_size()], &buffer[buffer_start], sizeof(T)*buffer_size);

			// sort buffer elements
			std::sort(mergebuffer.get(), mergebuffer.get()+(buffer_size+opq->sorted_size()), comp_);

			// smaller elements go in deletion buffer
			memcpy(buffer.get()+buffer_start, mergebuffer.get(), sizeof(T)*buffer_size);

			// larger elements go in insertion buffer
			memcpy(&arr[0], mergebuffer.get()+buffer_size, sizeof(T)*opq->sorted_size());
		}

		// Bubble lesser elements down into group buffer 0
		if(group_size(0)> 0) {

			// Merge insertion buffer and group buffer 0
			assert(group_size(0)+opq->sorted_size() <= setting_m*2);
			memory_size_type j = 0;

			// fetch gbuffer0
			for(stream_size_type i = group_start(0); i < group_start(0)+group_size(0); i++) {
				mergebuffer[j] = gbuffer0[static_cast<memory_size_type>(i%setting_m)];
				++j;
			}

			// fetch insertion buffer
			memcpy(&mergebuffer[j], &arr[0], sizeof(T)*opq->sorted_size());

			// sort
			std::sort(mergebuffer.get(), mergebuffer.get()+(group_size(0)+opq->sorted_size()), comp_);

			// smaller elements go in gbuffer0
			memcpy(gbuffer0.get(), mergebuffer.get(), static_cast<size_t>(sizeof(T)*group_size(0)));
			group_start_set(0,0);

			// larger elements go in insertion buffer (actually a free group 0 slot)
			memcpy(&arr[0], &mergebuffer[group_size(0)], sizeof(T)*opq->sorted_size());
		}

		// move insertion buffer (which has elements larger than all of
		// gbuffer0 and deletion buffer) into a free group 0 slot

		write_slot(slot, arr, opq->sorted_size());
		opq->sorted_pop();

		// insertion buffer is now empty

	}

	// insertion buffer is non-full. insert element.
	opq->push(x);
	m_size++;
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::pop() {
	if(empty()) {
		throw priority_queue_error("pop() invoked on empty priority queue");
	}
	// Call top() to freshen deletion buffer (if empty) and min_in_buffer
	top();

	// The top element is in either the insertion buffer or the deletion buffer.
	if(min_in_buffer) {
		// Top element in deletion buffer
		buffer_size--;
		buffer_start++;
		if(buffer_size == 0) {
			buffer_start = 0;
		}
	} else {
		// Top element in insertion buffer
		opq->pop();
	}
	m_size--;
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator, typename OPQType>
const T& priority_queue<T, Comparator, OPQType>::top() {
	// If the deletion buffer is empty, refill it with elements from the group buffers
	if(buffer_size == 0 && opq->size() != m_size) {
		fill_buffer();
	}
	// The top element is in either the insertion buffer or the deletion buffer.
	if(buffer_size == 0 && opq->size() == 0) {
		throw priority_queue_error("top() invoked on empty priority queue");
	} else if(opq->size() == 0) {
		min=buffer[buffer_start];
		min_in_buffer = true;
	} else if(buffer_size == 0) {
		min=opq->top();
		min_in_buffer = false;
	} else if(comp_(buffer[buffer_start], opq->top())) { // compare
		min=buffer[buffer_start];
		min_in_buffer = true;
	} else {
		min=opq->top();
		min_in_buffer = false;
	}
#ifndef NDEBUG
	validate();
#endif
	return min;
}

template <typename T, typename Comparator, typename OPQType>
stream_size_type priority_queue<T, Comparator, OPQType>::size() const {
	return m_size;
}

template <typename T, typename Comparator, typename OPQType>
bool priority_queue<T, Comparator, OPQType>::empty() const {
	return m_size == 0;
}

template <typename T, typename Comparator, typename OPQType> template <typename F>
F priority_queue<T, Comparator, OPQType>::pop_equals(F f) {
	T a = top();
	f(a);
	pop();
	if(size() == 0) return f;
	T b = top();
	while(!(comp_(a, b))) { // compare
		f(b);
		pop();
		if(size() == 0) return f;
		b = top();
	}
	return f;
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::dump() {
	TP_LOG_DEBUG( "--------------------------------------------------------------" << "\n"
			<< "DUMP:\tTotal size: "
			<< m_size << ", OPQ size: "
			<< opq->size()
			<< ", OPQ top: ");
	if(opq->size()>0) {
		TP_LOG_DEBUG("" << opq->top());
	} else {
		TP_LOG_DEBUG("empty");
	}
	TP_LOG_DEBUG(", current_r: "
			<< current_r << "\n"
			<< "\tBuffer size: "
			<< buffer_size
			<< ", buffer start: "
			<< buffer_start
			<< "\n" << "\t");

	// output main buffer
	for(memory_size_type i = 0; i<setting_mmark; i++) {
		TP_LOG_DEBUG((i<buffer_start || buffer_start+buffer_size <=i ?"(":"") 
				<< buffer[i] 
				<< (i<buffer_start || buffer_start+buffer_size <=i ?")":"") 
				<< " ");
	}
	TP_LOG_DEBUG("\n");

	// output groups
	for(memory_size_type i =0; i<current_r; i++) {
		TP_LOG_DEBUG("GROUP " << i << " ------------------------------------------------------" << "\n");
		TP_LOG_DEBUG("\tGroup Buffer, size: "
				<< group_size(i) << ", start: "
				<< group_start(i) << "\n" << "\t\tBuffer(no ('s): ");

		if(i == 0) { // group buffer 0 is special
			TP_LOG_DEBUG("internal: ");
			memory_size_type k = 0;
			for(k = 0; k < setting_m; k++) {
				TP_LOG_DEBUG(gbuffer0[k] << " ");
			}
			TP_LOG_DEBUG("\n");
		} else {
			// output group buffer contents
			file_stream<T> instream(block_factor);
			instream.open(group_data(i));
			memory_size_type k = 0;
			if(group_size(i) > 0) {
				for(k = 0; k < setting_m; k++) {
					TP_LOG_DEBUG(instream.read() << " ");
				}
			}
			for(memory_size_type l = k; l < setting_m; l++) {
				TP_LOG_DEBUG("() ");
			}
			TP_LOG_DEBUG("\n");
		}

		// output slots
		for(memory_size_type j = i*setting_k; j<i*setting_k+setting_k; j++) {
			TP_LOG_DEBUG("\t\tSlot " << j << "(size: "
					<< slot_size(j)
					<< " start: " << slot_start(j) << "):");

			file_stream<T> instream(block_factor);
			instream.open(slot_data(j));
			stream_size_type k;
			for(k = 0; k < slot_start(j)+slot_size(j); k++) {
				TP_LOG_DEBUG((k>=slot_start(j)?"":"(") <<
						instream.read() <<
						(k>=slot_start(j)?"":")") << " ");
			}
			for(stream_size_type l = k; l < slot_max_size(j); l++) {
				TP_LOG_DEBUG("() ");
			}

			TP_LOG_DEBUG("\n");
		}
	}
	TP_LOG_DEBUG("--------------------------------------------------------------\n");
}

/////////////////////////////
// Private
/////////////////////////////

// Find a free slot in given group.
// If the group is full, call empty_group,
// which calls remove_group_buffer, which calls free_slot(0)
template <typename T, typename Comparator, typename OPQType>
typename priority_queue<T, Comparator, OPQType>::slot_type
priority_queue<T, Comparator, OPQType>::free_slot(group_type group) {

	slot_type i;
	if(group>=setting_k) {
		std::stringstream msg;
		msg << "Error, queue is full no free slots in invalid group " 
			<< group << ". Increase k.";
		TP_LOG_FATAL_ID(msg.str());
		throw exception(msg.str());
	}

	for(i = group*setting_k; i < group*setting_k+setting_k; i++) {
		if(slot_size(i) == 0) {
			// This slot is good
			break;
		}
	}

	if(i == group*setting_k+setting_k) {
		// All slots are occupied. Empty this group by merging slots into a
		// single free slot in group+1.

		empty_group(group);

		if(slot_size(group*setting_k) != 0) {
			return free_slot(group); // some group buffers might have been moved
		}
		return group*setting_k;
	}

	return i;
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::fill_buffer() {
	if(buffer_size !=0) {
		return;
	}
	if(current_r == 0) { // todo: check that this is ok
		return;
	}

	// refill group buffers, if needed
	for(memory_size_type i=0;i<current_r;i++) {
		if(group_size(i)<static_cast<stream_size_type>(setting_mmark)) {
			fill_group_buffer(i);
		}
		if(group_size(i) == 0 && i==current_r-1) {
			current_r--;
		}
	}

	// merge to buffer
	mergebuffer.resize(0);
#ifndef TPIE_NDEBUG
	std::cout << "memavail after mb free: "
			  << get_memory_manager().available() << "b" << std::endl;
#endif

	{
	pq_merge_heap<T, Comparator> heap(current_r);

	tpie::array<tpie::unique_ptr<file_stream<T> > > data(current_r);
	for(memory_size_type i = 0; i<current_r; i++) {
		data[i].reset(tpie_new<file_stream<T> >(block_factor));
		if(i == 0 && group_size(i)>0) {
			heap.push(gbuffer0[group_start(0)], 0);
		} else if(group_size(i)>0) {
			data[i]->open(group_data(i));
			data[i]->seek(group_start(i));
			heap.push(data[i]->read(), i);
		} else if(i > 0) {
			// dummy, well :o/
		}
	}

	while(!heap.empty() && buffer_size!=setting_mmark) {
		group_type current_group = heap.top_run();
		if(current_group!= 0 && data[current_group]->offset() == setting_m) {
			data[current_group]->seek(0);
		}
		buffer[(buffer_size+buffer_start)%setting_m] = heap.top();
		buffer_size++;

		assert(group_size(current_group) >= 1);
		group_size_set(current_group, group_size(current_group)-1);
		group_start_set(current_group, (group_start(current_group)+1)%setting_m);
		if(group_size(current_group) == 0) {
			heap.pop();
		} else {
			if(current_group == 0) {
				heap.pop_and_push(gbuffer0[group_start(0)], 0);
			} else {
				heap.pop_and_push(data[current_group]->read(), current_group);
			}
		}
	}
	} // destruct and deallocate `heap'
#ifndef TPIE_NDEBUG
	std::cout << "memavail before mb alloc: "
			  << get_memory_manager().available() << "b" << std::endl;
#endif
	mergebuffer.resize(setting_m*2);
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::fill_group_buffer(group_type group) {
	assert(group_size(group) < static_cast<stream_size_type>(setting_mmark));
	// max k + 1 open streams
	// 1 merge heap
	// opq still in action

	//get rid of mergebuffer so that we enough memory
	//for the heap and misc structures below
	//this array is reallocated below
	mergebuffer.resize(0);
#ifndef TPIE_NDEBUG
	std::cout << "memavail after mb free: "
			  << get_memory_manager().available() << "b" << std::endl;
#endif

	// merge
	{

		//group output stream, not used if group==0 in this case 
		//the in-memory gbuffer0 is used
		file_stream<T> out(block_factor);
		out.open(group_data(group));
		if(group > 0) {
			out.seek((group_start(group)+group_size(group))%setting_m);
		}

		//merge heap for the setting_k slots
		pq_merge_heap<T, Comparator> heap(setting_k);

		//Create streams for the non-empty slots and initialize
		//internal heap with one element per slot
		tpie::array<tpie::unique_ptr<file_stream<T> > > data(setting_k);
		for(memory_size_type i = 0; i<setting_k; i++) {

			data[i].reset(tpie_new<file_stream<T> >(block_factor));

			if(slot_size(group*setting_k+i)>0) {
				//slot is non-empry, opening stream
				slot_type slotid = group*setting_k+i;
				data[i]->open(slot_data(slotid));

				//seek to start of slot
				data[i]->seek(slot_start(slotid));

				//push first item of slot on the stream
				heap.push(data[i]->read(), slotid);
			}
		}

		//perform actual reading until group if full or all 
		//the slots are empty
		while(!heap.empty() && group_size(group)!=static_cast<stream_size_type>(setting_m)) {
			slot_type current_slot = heap.top_run();

			if(group == 0) {
				//use in-memory array for group 0
				gbuffer0[(group_start(0)+group_size(0))%setting_m] = heap.top();
			} else {
				//write to disk for group >0
				if(out.offset() == setting_m) {
					out.seek(0);
				}

				out.write(heap.top());
			}

			//increase group size
			group_size_set(group, group_size(group) + 1);

			//decrease slot size and increase starting index
			slot_start_set(current_slot, slot_start(current_slot)+1);
			slot_size_set(current_slot, slot_size(current_slot)-1);

			//pop from heap and insert next element (if any) from the slot
			if(slot_size(current_slot) == 0) {
				heap.pop();
			} else {
				heap.pop_and_push(data[current_slot-group*setting_k]->read(), current_slot);
			}
		}

	}

	//restore mergebuffer
#ifndef TPIE_NDEBUG
	std::cout << "memavail before mb alloc: "
			  << get_memory_manager().available() << "b" << std::endl;
#endif
	mergebuffer.resize(setting_m*2);;
}

// Memory usage:
// Deallocates mergebuffer : -2*setting_m
// Opens newstream         : sizeof(file_stream<T>)
// PQ merge heap           : setting_k * (sizeof T + sizeof size_type)
// Opens old streams       : setting_k * sizeof(file_stream<T>)
// Reallocates mergebuffer : +2*setting_m
// (no net heap usage since 2*setting_m > temporary heap usage)
template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::empty_group(group_type group) {
	if(group > setting_k) {
		TP_LOG_FATAL_ID("Error: Priority queue is full");
		throw exception("Priority queue is full");
	}

	// All slots are occupied. Empty this group by merging slots into a
	// single free slot in group+1.

	slot_type newslot = free_slot(group+1);

	assert(slot_size(newslot) == 0);
	slot_start_set(newslot, 0);
	if(current_r < newslot/setting_k+1) {
		// create a new group

		current_r = newslot/setting_k+1;
	}

	bool ret = false;

	mergebuffer.resize(0);
#ifndef TPIE_NDEBUG
	std::cout << "memavail after mb free: "
			  << get_memory_manager().available() << "b" << std::endl;
#endif
	{

		file_stream<T> newstream(block_factor);
		newstream.open(slot_data(newslot));
		pq_merge_heap<T, Comparator> heap(setting_k);

		// Open streams to slots in group `group', push top element to merge heap
		tpie::array<tpie::unique_ptr<file_stream<T> > > data(setting_k);
		for(memory_size_type i = 0; i<setting_k; i++) {
			data[i].reset(tpie_new<file_stream<T> >(block_factor));
			data[i]->open(slot_data(group*setting_k+i));
			if(slot_size(group*setting_k+i) == 0) {
				ret = true;
				break;
			}
			assert(slot_size(group*setting_k+i)>0);
			data[i]->seek(slot_start(group*setting_k+i));
			heap.push(data[i]->read(), group*setting_k+i);
		}

		while(!heap.empty() && !ret) {
			slot_type current_slot = heap.top_run();
			newstream.write(heap.top());
			slot_size_set(newslot,slot_size(newslot)+1);
			slot_start_set(current_slot, slot_start(current_slot)+1);
			slot_size_set(current_slot, slot_size(current_slot)-1);
			if(slot_size(current_slot) == 0) {
				heap.pop();
			} else {
				heap.pop_and_push(data[current_slot-group*setting_k]->read(), current_slot);
			}
		}
	}

#ifndef TPIE_NDEBUG
	std::cout << "memavail before mb alloc: "
			  << get_memory_manager().available() << "b" << std::endl;
#endif
	mergebuffer.resize(setting_m*2);;

	if(group_size(group+1) > 0 && !ret) {
		// Maintain heap invariant:
		//     group buffer i <= group i slots
		// If the group buffer of the group in which we inserted runs
		// was not empty before, we might now have violated a heap invariant
		// by inserting elements in a [group+1] slot that are less than elements
		// in group buffer [group+1].
		// Just remove the group buffer to ensure the invariant.
		remove_group_buffer(group+1); // todo, this might recurse?
	}
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::validate() {
#ifndef NDEBUG
#ifdef PQ_VALIDATE
	cout << "validate start" << "\n";
	// validate size
	stream_size_type size = 0;
	size = size + opq->size();
	size = size + buffer_size;
	for(stream_size_type i = 0; i<setting_k;i++) {
		size = size + group_size(i);
	}
	for(stream_size_type i = 0; i<setting_k*setting_k;i++) {
		size = size + slot_size(i);
	}
	if(m_size != size) {
		TP_LOG_FATAL_ID("Error: Validate: Size not ok");
		exit(-1);
	}

	// validate internal order in "nodes"
	if(buffer_size > 0) { // buffer
		T last = buffer[buffer_start];
		for(stream_size_type i = buffer_start; i<buffer_start+buffer_size;i++) {
			if(comp_(buffer[i],last)) {
				//Kasper and Mark: Should this exit(-1)?
				TP_LOG_WARNING_ID("Error: Buffer ordered validation failed");
			}
			last = buffer[i];
		}
	}
	// todo: validate gbuffer0
	for(stream_size_type i = 1; i < setting_k; i++) { // groups, nb: cyclic
		if(group_size(i) > 0) {
			file_stream<T> stream;
			stream.open(group_data(i));
			stream.seek(group_start(i));
			if(stream.offset() == setting_m) {
				stream.seek(0);
			}
			T last = stream.read();
			for(stream_size_type j = 1; j < group_size(i); j++) {
				if(stream.offset() == setting_m) {
					stream.seek(0);
				}
				T read = stream.read();
				if(comp_(read, last)) { // compare
					dump();
					TP_LOG_FATAL_ID("Error: Group buffer " << i << " order invalid (last: " << last <<
									", read: " << read << ")");
					exit(-1);
				} 
			}
			delete stream;
		}
	}
	for(stream_size_type i = 0; i < setting_k*setting_k; i++) { // slots
		if(slot_size(i) > 0){
			file_stream<T> stream;
			stream.open(slot_data(i));
			stream.seek(slot_start(i));
			T last = stream.read();
			for(stream_size_type j = 1; j < slot_size(i); j++) {
				T read = stream.read();
				if(comp_(read, last)) { // compare
					TP_LOG_FATAL_ID("Error: Slot " << i << " order invalid (last: " << last <<
									", read: " << read << ")");
					exit(-1);
				}
			}
		}
	}

	// validate heap properties
	if(buffer_size > 0) { // buffer --> group buffers
		T buf_max = buffer[buffer_start+buffer_size-1];
		for(stream_size_type i = 1; i < setting_k; i++) { // todo: gbuffer0
			if(group_size(i) > 0) {
				file_stream<T> stream;
				stream.open(group_data(i));
				stream.seek(group_start(i));
				if(stream->offset() == setting_m) {
					stream.seek(0);
				}
				T first = stream.read();
				if(comp_(first, buf_max)) { // compare
					dump();
					TP_LOG_FATAL_ID("Error: Heap property invalid, buffer -> group buffer " << i <<
									"(buffer: " << buf_max << ", first: " << first << ")");
					exit(-1);
				}
			}
		}
	}

	// todo: gbuffer0
	for(stream_size_type i = 1; i < setting_k; i++) { // group buffers --> slots
		if(group_size(i) > 0) {
			file_stream<T> stream;
			stream.open(group_data(i));
			stream.seek((group_start(i)+group_size(i)-1)%setting_m);
			T item_group = stream.read();
			//cout << "item_group: " << item_group << "\n";

			for(stream_size_type j = i*setting_k; j<i*setting_k+setting_k;j++) {
				if(slot_size(j) > 0) {
					file_stream<T> stream;
					stream.open(slot_data(j));
					stream.seek(slot_start(j));
					T item_slot = stream.read();
					
					if(comp_(item_slot, item_group)) { // compare
						dump();
						TP_LOG_FATAL_ID("Error: Heap property invalid, group buffer " << i <<
										" -> slot " << j << "(group: " << item_group <<
										", slot: " << item_slot << ")");
						exit(-1);
					}
				}
			}
		}
	}
	//cout << "validate end" << "\n";
#endif
#endif
}

// Empty a group buffer by inserting it into an empty group 0 slot.
// To maintain the invariant
//     group buffer 0 elements <= group 0 slot elements,
// merge the given group buffer with group buffer 0 before writing the slot out.
template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::remove_group_buffer(group_type group) {
#ifndef NDEBUG
	if(group == 0) {
		TP_LOG_FATAL_ID("Attempt to remove group buffer 0");
		throw exception("Attempt to remove group buffer 0");
	}
#endif

	// this is the easiest thing to do
	slot_type slot = free_slot(0);
	if(group_size(group) == 0) return;

	TP_LOG_DEBUG_ID("Remove group buffer " << group <<
					" of size " << group_size(group) <<
					" with available memory " << get_memory_manager().available());

	assert(group < setting_k);
	array<T> arr(static_cast<size_t>(group_size(group)));
	file_stream<T> data(block_factor);
	data.open(group_data(group));
	data.seek(group_start(group));
	memory_size_type size = group_size(group);
	if(group_start(group) + group_size(group) <= static_cast<stream_size_type>(setting_m)) {
		data.read(arr.begin(), arr.find(size));
	} else {
		// two reads
		memory_size_type first_read = setting_m - group_start(group);
		memory_size_type second_read = size - first_read;

		data.read(arr.begin(), arr.find(first_read));
		data.seek(0);
		data.read(arr.find(first_read), arr.find(first_read+second_read));
	}
	assert(group_size(group) > 0);

	// make sure that the new slot in group 0 is heap ordered with gbuffer0
	if(group > 0 && group_size(0) != 0) {
		memory_size_type j = 0;
		for(memory_size_type i = group_start(0); i < group_start(0)+group_size(0); i++) {
			mergebuffer[j] = gbuffer0[i%setting_m];
			++j;
		}
		memcpy(&mergebuffer[j], &arr[0], static_cast<size_t>(sizeof(T)*group_size(group)));
		std::sort(&mergebuffer[0], &mergebuffer[0]+(group_size(0)+group_size(group)), comp_);
		memcpy(&gbuffer0[0], &mergebuffer[0], static_cast<size_t>(sizeof(T)*group_size(0)));
		group_start_set(0,0);
		memcpy(&arr[0], &mergebuffer[group_size(0)], static_cast<size_t>(sizeof(T)*group_size(group)));
	}

	write_slot(slot, arr.get(), group_size(group));
	group_start_set(group, 0);
	group_size_set(group, 0);
}

//////////////////
// TPIE wrappers
template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::slot_start_set(slot_type slot, memory_size_type n) {
	slot_state[slot*3] = n;
}

template <typename T, typename Comparator, typename OPQType>
memory_size_type priority_queue<T, Comparator, OPQType>::slot_start(slot_type slot) const {
	return slot_state[slot*3];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::slot_size_set(slot_type slot, memory_size_type n) {
	assert(slot<setting_k*setting_k);
	slot_state[slot*3+1] = n;
}

template <typename T, typename Comparator, typename OPQType>
memory_size_type priority_queue<T, Comparator, OPQType>::slot_size(slot_type slot) const {
	return slot_state[slot*3+1];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::group_start_set(group_type group, memory_size_type n) {
	group_state[group*2] = n;
}

template <typename T, typename Comparator, typename OPQType>
memory_size_type priority_queue<T, Comparator, OPQType>::group_start(group_type group) const {
	return group_state[group*2];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::group_size_set(group_type group, memory_size_type n) {
	assert(group<setting_k);
	group_state[group*2+1] = n;
}

template <typename T, typename Comparator, typename OPQType>
memory_size_type priority_queue<T, Comparator, OPQType>::group_size(group_type group) const {
	return group_state[group*2+1];
}

template <typename T, typename Comparator, typename OPQType>
temp_file & priority_queue<T, Comparator, OPQType>::slot_data(slot_type slotid) {
	return datafiles[slot_state[slotid*3+2]];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::slot_data_set(slot_type slotid, memory_size_type n) {
	slot_state[slotid*3+2] = n;
}

template <typename T, typename Comparator, typename OPQType>
temp_file & priority_queue<T, Comparator, OPQType>::group_data(group_type groupid) {
	return groupdatafiles[groupid];
}

template <typename T, typename Comparator, typename OPQType>
memory_size_type priority_queue<T, Comparator, OPQType>::slot_max_size(slot_type slotid) {
	// todo, too many casts
	return setting_m
		*static_cast<memory_size_type>(pow((long double)setting_k,
										   (long double)(slotid/setting_k)));
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::write_slot(slot_type slotid, T* arr, memory_size_type len) {
	assert(len > 0);
	file_stream<T> data(block_factor);
	data.open(slot_data(slotid));
	data.write(arr+0, arr+len);
	slot_start_set(slotid, 0);
	slot_size_set(slotid, len);
	if(current_r == 0 && slotid < setting_k) {
		current_r = 1;
	}
}

/////////////////////
