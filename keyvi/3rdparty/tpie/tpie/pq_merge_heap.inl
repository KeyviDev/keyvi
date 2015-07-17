// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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


template <typename T, typename Comparator>
pq_merge_heap<T, Comparator>::pq_merge_heap(memory_size_type elements) {
	maxsize = elements;
	heap = tpie_new_array<T>(elements);
	runs = tpie_new_array<run_type>(elements);
	m_size = 0;
}

template <typename T, typename Comparator>
pq_merge_heap<T, Comparator>::~pq_merge_heap() {
  tpie_delete_array(heap, maxsize);
  tpie_delete_array(runs, maxsize);
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::push(const T& x, run_type run) {
	assert(m_size < maxsize);
	heap[m_size] = x;
	runs[m_size] = run;
	stream_offset_type parent;
	stream_offset_type child;
	child = m_size;
	parent = ( child - 1 ) / 2;
	m_size++;
	//cout << "pq_merge_heap: push" << endl;
	while(parent >= 0 && comp_(heap[child],heap[parent])) {
		//cout << "pq_merge_heap: fixup, child: " << heap[child] << " parent:" << heap[parent] << endl;
		std::swap(heap[child],heap[parent]);
		std::swap(runs[child],runs[parent]);
		child = parent;
		parent = ( child - 1 ) / 2;
	}
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::pop() {
	assert(m_size > 0);
	m_size--;
	if(m_size != 0) {
		heap[0] = heap[m_size];
		runs[0] = runs[m_size];
		fixDown();
	}
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::pop_and_push(const T& x, run_type run) {
	assert(m_size > 0);
	heap[0] = x;
	runs[0] = run;
	fixDown();
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator>
const T& pq_merge_heap<T, Comparator>::top() const {
	assert(m_size > 0);
	return heap[0];
}

template <typename T, typename Comparator>
typename pq_merge_heap<T, Comparator>::run_type
pq_merge_heap<T, Comparator>::top_run() const {
	assert(m_size > 0);
	return runs[0];
}

template <typename T, typename Comparator>
memory_size_type pq_merge_heap<T, Comparator>::size() const {
	return m_size;
}

template <typename T, typename Comparator>
bool pq_merge_heap<T, Comparator>::empty() const {
	return m_size == 0;
}

///////////////////////////////////////
// Private
///////////////////////////////////////

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::fixDown() {
	stream_offset_type parent, child1, child2;
	assert(m_size > 0);
	bool done = false;
	if(m_size == 1) done = true;
	parent = 0;
	child1 = parent * 2 + 1;
	child2 = parent * 2 + 2;

	//cout << "Loop Start, heap size:" << m_size<< endl;
	while(!done) {
		//cout << "pq_merge_heap: loop top" << child1 << " " << child2 << " " << m_size << endl;
		assert(m_size>0);
		if(child1 == static_cast<stream_offset_type>(m_size-1)) {
			//cout << "pq_merge_heap: fixdown 0" << endl;
			if(comp_(heap[child1],heap[parent])) {
				//cout << "pq_merge_heap: swap, fixdown 0 " << heap[child1] << " " << heap[parent] << endl;
				assert(child1 < static_cast<stream_offset_type>(maxsize));
				assert(parent < static_cast<stream_offset_type>(maxsize));
				std::swap(heap[child1],heap[parent]);
				std::swap(runs[child1],runs[parent]);
			}
			done = true;
			continue;
		} 
		if(child1 > static_cast<stream_offset_type>(m_size-1)) {
			//cout << "pq_merge_heap done 1" << endl;
			done = true;
			continue;
		}
		if(child2 > static_cast<stream_offset_type>(m_size-1)) {
			//cout << "pq_merge_heap done 2" << endl;
			done = true;
			continue;
		}
		//cout << "pq_merge_heap: loop step " << child1 << " " << child2 << " " << parent << " " << m_size << endl; 
		if(comp_(heap[child1],heap[child2]) && comp_(heap[child1],heap[parent])) {
			//cout << "pq_merge_heap: fixdown 1: " << heap[child1] << "(" << child1 << ") " << heap[parent] << "("<<parent << ")" << endl;
			assert(child1 < static_cast<stream_offset_type>(maxsize));
			assert(parent < static_cast<stream_offset_type>(maxsize));
			std::swap(heap[child1],heap[parent]);
			std::swap(runs[child1],runs[parent]);
			parent = child1;
		} else if(comp_(heap[child2],heap[parent])) {
			//cout << "pq_merge_heap: fixdown 2: " << heap[child2] << "(" << child2 << ") " << heap[parent] << "("<<parent << ")" << endl;
			assert(child2 < static_cast<stream_offset_type>(maxsize));
			assert(parent < static_cast<stream_offset_type>(maxsize));
			std::swap(heap[child2],heap[parent]);
			std::swap(runs[child2],runs[parent]);
			parent = child2;
		} else {
			done = true; 
			continue;
		}
		child1 = parent * 2 + 1;
		child2 = parent * 2 + 2;
	}
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::validate() {
#ifndef NDEBUG
#ifdef PQ_VALIDATE
	memory_size_type child1, child2;
	for(memory_size_type i = 0; i<m_size; i++) {
		child1 = i * 2 + 1;
		child2 = i * 2 + 2;
		if(child1<m_size) {
			if(comp_(heap[child1],heap[i])) dump();
			assert(!comp_(heap[child1],heap[i]));
		}
		if(child2<m_size) {
			if(comp_(heap[child2],heap[i])) dump();
			assert(!comp_(heap[child2],heap[i]));
		}
	}
#endif
#endif
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::dump() {
	TP_LOG_DEBUG("pq_merge_heap: "); 
	for(memory_size_type i = 0; i<m_size; i++) {
		TP_LOG_DEBUG(heap[i] << ", ");
	}
	TP_LOG_DEBUG("\n");
}
