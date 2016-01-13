// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, The TPIE development team
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
#ifndef __TPIE_TEST_UNIT_PRIORITY_QUEUE_H__
#define __TPIE_TEST_UNIT_PRIORITY_QUEUE_H__

#include <tpie/progress_indicator_arrow.h>
#include <random>
#include "common.h"
#include <vector>
#include <queue>

template <typename T>
bool cyclic_pq_test(T & pq, uint64_t size, uint64_t iterations) {
	tpie::progress_indicator_arrow progress("Running test", iterations, tpie::log_info());
	std::priority_queue<uint64_t, std::vector<uint64_t>, bit_pertume_compare<std::less<uint64_t> > > pq2;
	std::default_random_engine rnd;
	std::uniform_real_distribution<> urnd;

	for (uint64_t i=0;i<iterations;i++){
		progress.step();
		if (pq.size() != pq2.size()) {
			tpie::log_error() << "Size differs " << pq.size() << " " << pq2.size() << std::endl;
			return false;
		}
		if (pq.size() != 0 && pq.top() != pq2.top()){
			tpie::log_error() << "Top element differs " << pq.top() << " " << pq2.top() << std::endl;
			return false;
		}
		if ((size_t)pq.size() == (size_t)0 ||
			((size_t)pq.size() < (size_t)size && urnd(rnd) <= (cos(static_cast<double>(i) * 60.0 / static_cast<double>(size))+1.0)/2.0)) {
			uint64_t r = rnd();
			pq.push(r);
			pq2.push(r);
		} else {
			pq.pop();
			pq2.pop();
		}
	}
	return true;
}

template <typename T>
bool basic_pq_test(T & pq, uint64_t size) {
	std::default_random_engine rnd;
	// for(uint64_t i=0; i < size; ++i)
	// 	pq.push( (i*40849+37159)%size );
    // for(uint64_t i=0; i < 2473; ++i) {
	// 	if (pq.empty()) return false;
	// 	if (pq.top() != i) return false;
	// 	pq.pop();
    // }
    // for(uint64_t i=0; i < 2473; ++i)
	// 	pq.push((i*40849+37159)%2473);
	// for(uint64_t i=0; i < size; ++i) {
	// 	if (pq.empty()) return false;
	// 	if (pq.top() != i) return false;
	// 	pq.pop();
	// }
	// if (!pq.empty()) return false; 

	std::priority_queue<uint64_t, std::vector<uint64_t>, bit_pertume_compare<std::less<uint64_t> > > pq2;
	for (uint64_t i=0;i<size;i++){
		uint64_t r = rnd();
		pq.push(r);
		pq2.push(r);
	}
	while (!pq.empty()){
		if (pq.top()!=pq2.top()) return false;
		pq.pop();
		if (pq2.empty()) return false;
		pq2.pop();
	}
	if (!pq2.empty()) return false;
	return cyclic_pq_test(pq, size, size);
}

#endif //__TPIE_TEST_UNIT_PRIORITY_QUEUE_H__
