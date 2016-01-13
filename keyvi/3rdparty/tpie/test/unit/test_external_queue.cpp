// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2011, 2013, 2015, The TPIE development team
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
#include <queue>
#include <tpie/queue.h>
#include <cmath>
#include <random>

using namespace tpie;

inline uint64_t element(size_t i) {
	return 179424673 * i + 15485863;
}

#if 0
std::ostream & debug = std::cout;
#else
std::ostream debug(0); // bit bucket
#endif

bool queue_test(const size_t elements = 2*1024*1024/sizeof(uint64_t)) {
	const size_t maxpush = 64;
	queue<uint64_t> q1;
	std::queue<uint64_t> q2;

	size_t i = 0;

	// number of elements currently in the queue
	uint64_t l = 0;

	while (i < elements) {
		// First, push a random number of elements to our queue and std::queue.
		uint64_t push = 1 + (element(i) % (maxpush - 1));
		for (uint64_t j = 0; i < elements && j < push; ++i, ++j) {
			debug << "Push " << i << " " << element(i) << " " << std::endl;
			q1.push(element(i));
			q2.push(element(i));
		}

		l += push;

		// Next, pop a random number of elements.
		uint64_t pop = 1 + (element(i) % (l - 1));
		for (uint64_t j = 0; i < elements && j < pop; ++i, ++j) {
			debug << "Pop " << i << std::endl;

			uint64_t el = q2.front();
			q2.pop();

			uint64_t got = q1.front();

			// our queue implementation also returns an element in pop()
			if (got != q1.pop()) {
				tpie::log_info() << "pop() doesn't agree with front() on element " << i << std::endl;
				return false;
			}

			if (el != got) {
				tpie::log_info() << "front() returned incorrect element " << i << std::endl;
				return false;
			}

			debug << "Got " << el << std::endl;
		}

		l -= pop;
	}

	return true;
}

bool basic_test() {
	return queue_test();
}

bool empty_size_test() {
	queue<uint64_t> q;
	bool result = true;
	TEST_ENSURE(q.empty(), "Wrong empty");
	TEST_ENSURE_EQUALITY(0, q.size(), "Wrong size");
	q.push(0);
	TEST_ENSURE(!q.empty(), "Wrong empty");
	TEST_ENSURE_EQUALITY(1, q.size(), "Wrong size");
	q.push(0);
	TEST_ENSURE(!q.empty(), "Wrong empty");
	TEST_ENSURE_EQUALITY(2, q.size(), "Wrong size");
	for (size_t i = 0; i < 1000; ++i) q.push(42);
	TEST_ENSURE(!q.empty(), "Wrong empty");
	TEST_ENSURE_EQUALITY(1002, q.size(), "Wrong size");
	for (size_t i = 0; i < 1000000; ++i) q.push(42);
	TEST_ENSURE(!q.empty(), "Wrong empty");
	TEST_ENSURE_EQUALITY(1001002, q.size(), "Wrong size");
	for (size_t i = 0; i < 1000000; ++i) q.pop();
	TEST_ENSURE(!q.empty(), "Wrong empty");
	TEST_ENSURE_EQUALITY(1002, q.size(), "Wrong size");
	for (size_t i = 0; i < 1000; ++i) q.pop();
	TEST_ENSURE(!q.empty(), "Wrong empty");
	TEST_ENSURE_EQUALITY(2, q.size(), "Wrong size");
	q.pop();
	TEST_ENSURE(!q.empty(), "Wrong empty");
	TEST_ENSURE_EQUALITY(1, q.size(), "Wrong size");
	q.pop();
	TEST_ENSURE_EQUALITY(0, q.size(), "Wrong size");
	TEST_ENSURE(q.empty(), "Wrong empty");
	return result;
}

double pop_probability(uint64_t i, uint64_t max_size) {
	double factor = 3.14159f / max_size;
	return (1-cos(factor * i))/2;
}

bool large_test() {
	std::mt19937 rng(42);
	std::uniform_real_distribution<double> dist;

	queue<uint64_t> q1;
	std::queue<uint64_t> q2;

	for(uint64_t i = 0; i < 1024*1024*10; ++i) {
		TEST_ENSURE_EQUALITY(q1.size(), q2.size(), "size() does not agree with STL queue");
		TEST_ENSURE_EQUALITY(q1.empty(), q1.empty(), "empty() does not agree with STL queue");

		if(!q1.empty())
			TEST_ENSURE_EQUALITY(q1.front(), q2.front(), "front() does not agree with STL queue. Size " << q1.size());

		if(!q1.empty() && dist(rng) <= pop_probability(i, 1024*1024)) {
			TEST_ENSURE_EQUALITY(q1.pop(), q2.front(), "pop() does not agree with STL queue");
			q2.pop();
		}
		else {
			uint64_t el = element(i);
			q1.push(el);
			q2.push(el);
		}
	}

	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv, 32)
		.test(basic_test, "basic")
		.test(empty_size_test, "empty_size")
		.test(queue_test, "sized", "n", static_cast<size_t>(32*1024*1024/sizeof(uint64_t)))
		.test(large_test, "large")
		;
}
