// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
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
#include "common.h"
#include <tpie/priority_queue.h>
#include <vector>
#include "priority_queue.h"
#include "../test_portability.h"

using namespace tpie;
using namespace std;

bool basic_test() {
	//Lets hope the external pq has a small block factor!
	get_memory_manager().set_limit(32*1024*1024);
	ami::priority_queue<uint64_t, bit_pertume_compare< std::greater<uint64_t> > > pq(1.0);
#ifdef NDEBUG
	const uint64_t size = 350003;
#else // DEBUG
	const uint64_t size = 350;
#endif
	return basic_pq_test(pq, size);
}

bool medium_instance() {
	TPIE_OS_OFFSET iterations = 10000;
    get_memory_manager().set_limit(32*1024*1024);
	progress_indicator_arrow progress("Running test",iterations-1100, tpie::log_info());
    for(TPIE_OS_OFFSET it = 1100; it < iterations; it++)  {
		progress.step();
		ami::priority_queue<int, std::greater<int> > pq(0.75);
		std::priority_queue<int, vector<int>,std::less<int> > pq2;

		TPIE_OS_OFFSET elements = 71;
		TPIE_OS_SRANDOM(static_cast<unsigned int>(it));
		for(TPIE_OS_OFFSET i=0;i<elements;i++) {
			int src_int = TPIE_OS_RANDOM()%220;
			pq.push(src_int);
			pq2.push(src_int);
		}
		pq.pop();
		pq2.pop();
		pq.pop();
		pq2.pop();
		pq.pop();
		pq2.pop();
		TPIE_OS_OFFSET pop = 61; 
		for(TPIE_OS_OFFSET i=0;i<pop;i++) {
			if(!pq.empty()) {
				if(pq.top() != pq2.top()) {
					tpie::log_error() << "Pop " << i << " got: " << pq.top() << " expected " << pq2.top() << std::endl;
					return false;
				}
				pq.pop();
				pq2.pop();
			}
		}
		for(TPIE_OS_OFFSET i=0;i<elements;i++) {
			int src_int = TPIE_OS_RANDOM()%220;
			pq.push(src_int);
			pq2.push(src_int);
		}
		while(!pq.empty()) {
			if(pq.top() != pq2.top()) {
				tpie::log_error() <<  "Pop2, got: " << pq.top() << " expected " << pq2.top() << std::endl;
				return false;
			}
			pq.pop();
			pq2.pop();
		}
    }
	progress.done();
	return true;
}

bool large_cycle(){
	size_t x = 524*1024*1024;
	get_memory_manager().set_limit(x);
	ami::priority_queue<uint64_t, bit_pertume_compare< std::greater<uint64_t> > > pq(0.01);
	return cyclic_pq_test(pq, x / 10, 20000000);
}

template <bool crash_test>
bool large_instance(){
	const double PI = acos(-1.0);
	get_memory_manager().set_limit(500*1024*1024);
	double mem_frac = crash_test ? 1.0 : 0.04;

	ami::priority_queue<uint64_t, std::greater<uint64_t> > pq(mem_frac);
	std::priority_queue<uint64_t, vector<uint64_t>, std::less<uint64_t> > pq2;

	double cycle = crash_test ? 20000000000.0 : 50000000.0;
	const TPIE_OS_OFFSET iterations=500000000;
	progress_indicator_arrow progress("Running test",iterations, tpie::log_info());
	for (TPIE_OS_OFFSET j=0; j<iterations; j++) {
		progress.step();
		double i = static_cast<double>(j);
		double th = (cos(i*2.0*PI/cycle)+1.0)*(RAND_MAX/2);
		if (!crash_test && (uint64_t)pq.size() != (uint64_t)pq2.size()) return false;
		if (!crash_test && pq.empty() != pq2.empty()) return false;

		if (!crash_test && !pq.empty() && pq.top()!=pq2.top()) {
			tpie::log_error() << j << " Priority queues differ, got " << pq.top() << " but expected " 
							  << pq2.top() << std::endl;
			return false;
		}
		if (rand()<th) {
			uint64_t r = rand();
			pq.push(r);
			if (!crash_test) pq2.push(r);
		} else {
			if (pq.empty()) continue;
			pq.pop();
			if (!crash_test) pq2.pop();
		}
		// if(j%5000000==0) {
		// 	std::cout << "\rElements in pq: " << pq.size() << "                             " <<  std::endl;
		// 	std::cout << "Memory available: " << MM_manager.memory_available()/1024/1024 << " MB" << std::endl;
		// }
	}
	progress.done();
	return true;
}

class priority_queue_memory_test : public memory_test {
public:
	priority_queue_memory_test() {
		m_memory = std::min(ITEMS * sizeof(size_t) / 2, consecutive_memory_available());
	}

	priority_queue_memory_test(size_type memory_usage) :
		m_memory(memory_usage) {
		// Empty ctor
	}

	virtual ~priority_queue_memory_test() {
		// Empty dtor
	}

	virtual void alloc() {
		m_pq = tpie_new<ami::priority_queue<size_t> >(m_memory);
	}

	static inline size_t ITEM(size_t i) {return i*98927 % 104639;}
	static const size_t ITEMS = 16*1024*1024;

	virtual void use() {
		progress_indicator_arrow progress("Priority queue test",ITEMS*2, tpie::log_info());
		for (size_t i = 0; i < ITEMS; ++i) {
			progress.step();
			m_pq->push(ITEM(i));
		}
		for (size_t i = 0; i < ITEMS; ++i) {
			progress.step();
			m_pq->pop();
		}
	}

	virtual void free() {
		tpie_delete<ami::priority_queue<size_t> >(m_pq);
		m_pq = 0;
	}

	virtual size_type claimed_size() {
		return m_memory;
	}

private:
	ami::priority_queue<size_t> * m_pq;
	size_type m_memory;
};

bool memory_test() {
	priority_queue_memory_test tester;
	return tester();
}


bool overflow_test() {
	tpie::priority_queue<char> pq;
	char c = 0;
	const uint64_t limit = 100ull + (1ull << 32);
	const uint64_t every = 1<<16;
	const uint64_t steps = limit/every;

	progress_indicator_arrow pi("PQ test", 2*steps, tpie::log_info());
	pi.init(2*steps);
	uint64_t progress_tracker = every;

	for (uint64_t i = 0; i < limit; ++i) {
		pq.push(c++);

		if (!--progress_tracker) {
			progress_tracker = every;
			pi.step();
		}
	}

	for (uint64_t i = 0; i < limit; ++i) {
		pq.pop();

		if (!--progress_tracker) {
			progress_tracker = every;
			pi.step();
		}
	}

	pi.done();
	return true;
}


template <uint64_t prime, typename T>
bool very_large_test() {
	try {
		const uint64_t generator=104729;
		const uint64_t base=((prime + 7)/8);
		const uint64_t size=base+base+prime;
		uint64_t nn=0;
		uint64_t ext=0;
		ami::priority_queue<uint64_t> pq;
		tpie::progress_indicator_arrow pi("Test", size+size);
		pi.init(size+size);
		for (uint64_t i=0; i < prime; ++i) {
			if (i % 8 == 0) {
				pq.push(nn++);
				pq.push(nn++);
				if (pq.empty()) {std::cerr << "Was empty" << std::endl; return false;}
				T elm=pq.top();
				if (elm != T(ext) ) {std::cerr << "Expected " << T(ext) << " got " << elm << std::endl; return false;}
				++ext;
				pq.pop();
				pi.step(3);
			}
			pq.push( base+base + (i * generator) % prime);
			pi.step();
		}
		for (uint64_t i=0; i < base+prime; ++i) {
			if (pq.empty()) {std::cerr << "Was empty" << std::endl; return false;}
			T elm=pq.top();
			if (elm != T(ext) ) {std::cerr << "Expected " << ext << " got " << elm << std::endl; return false;}
			++ext;
			pq.pop();
			pi.step();
		}
		pi.done();
		if (!pq.empty()) {std::cerr << "Was not empty" << std::endl; return false;}
		return true;
	} catch(std::exception e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return false;
	}
}

template <typename T>
bool parameter_test(double kb, double blockSizeKB) {
	float blockFact = file<T>::calculate_block_factor(static_cast<memory_size_type>(blockSizeKB*1024.0));
	memory_size_type mmAvail = static_cast<memory_size_type>(kb*1024.0);
	log_debug() << "blockFact = " << blockFact << "\nmmAvail = " << mmAvail << endl;
	{
		tpie::priority_queue<T> pq(mmAvail, blockFact);
		pq.push(T());
	}
	return true;
}

template <typename T>
bool remove_group_buffer_test(memory_size_type mmAvail, float blockFact, stream_size_type items, stream_size_type iterations) {
	log_debug() << "blockFact = " << blockFact << "\nmmAvail = " << mmAvail << endl;
	ami::priority_queue<uint64_t, bit_pertume_compare< std::greater<uint64_t> > > pq(mmAvail, blockFact);
	return cyclic_pq_test(pq, items, iterations);
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv, 128)
		.test(basic_test, "basic")
		.test(medium_instance, "medium")
		.test(large_instance<false>, "large")
		.test(large_cycle, "large_cycle")
		.test(memory_test, "memory")
		.test(very_large_test<4294967311, uint64_t>, "very_large")
		.test(overflow_test, "overflow")
		.test(parameter_test<uint64_t>, "parameters", "kb", 50000.0, "bs_kb", 128.0)
		.test(remove_group_buffer_test<uint64_t>, "remove_group_buffer",
			  "mmavail", static_cast<memory_size_type>(23552),
			  "blockfact", 0.000244141f,
			  "items", static_cast<stream_size_type>(5000),
			  "iterations", static_cast<stream_size_type>(100000))
		;
}
