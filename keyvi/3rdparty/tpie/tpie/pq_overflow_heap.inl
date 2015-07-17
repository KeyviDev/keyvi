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


template<typename T, typename Comparator>
pq_overflow_heap<T, Comparator>::pq_overflow_heap(memory_size_type m, Comparator c):
  comp(c), h(m, comp), maxsize(m) {}

template<typename T, typename Comparator>
inline void pq_overflow_heap<T, Comparator>::push(const T& x) {
#ifndef NDEBUG
	if(h.size() == maxsize) {
		TP_LOG_FATAL_ID("pq_overflow_heap: push error");
		exit(-1);
	}
#endif
	h.push(x);
}

template<typename T, typename Comparator>
inline void pq_overflow_heap<T, Comparator>::pop() {
	assert(!empty());
	h.pop();
}

template<typename T, typename Comparator>
inline const T& pq_overflow_heap<T, Comparator>::top() {
	assert(!empty());
	return h.top();
}

template<typename T, typename Comparator>
inline stream_size_type pq_overflow_heap<T, Comparator>::size() const {
	return h.size();
}

template<typename T, typename Comparator>
inline bool pq_overflow_heap<T, Comparator>::full() const {
	return maxsize == h.size();
}

template<typename T, typename Comparator>
inline T* pq_overflow_heap<T, Comparator>::sorted_array() {
	tpie::array<T> & a = h.get_array();
	std::sort(a.begin(), a.begin() + h.size(), comp);
	return a.get();
}

template<typename T, typename Comparator>
inline memory_size_type pq_overflow_heap<T, Comparator>::sorted_size() const{
	return maxsize;
}

template<typename T, typename Comparator>
inline void pq_overflow_heap<T, Comparator>::sorted_pop() {
	h.clear();
}

template<typename T, typename Comparator>
inline bool pq_overflow_heap<T, Comparator>::empty() const {
	return h.empty();
} 
