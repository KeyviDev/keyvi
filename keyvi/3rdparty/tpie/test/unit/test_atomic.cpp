// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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
#include <boost/thread.hpp>
#include <tpie/atomic.h>

#define TEST_ASSERT(cond) \
do { \
	if (!(cond)) { \
		tpie::log_error() << "Test failed on line " << __LINE__ << ": " #cond << std::endl; \
		return false; \
	} \
} while (0)

bool basic_test() {
	tpie::atomic_int i;
	TEST_ASSERT(i.fetch() == 0);
	TEST_ASSERT(i.fetch_and_add(40) == 0);
	TEST_ASSERT(i.fetch() == 40);
	TEST_ASSERT(i.add_and_fetch(30) == 70);
	TEST_ASSERT(i.fetch() == 70);
	TEST_ASSERT(i.fetch_and_sub(40) == 70);
	TEST_ASSERT(i.fetch() == 30);
	TEST_ASSERT(i.sub_and_fetch(30) == 0);
	TEST_ASSERT(i.fetch() == 0);
	i.add(40);
	TEST_ASSERT(i.add_and_fetch(30) == 70);
	TEST_ASSERT(i.fetch_and_sub(40) == 70);
	i.sub(30);
	TEST_ASSERT(i.fetch() == 0);
	return true;
}

class atomic_incrementer {
public:
	void operator()(tpie::atomic_int * myAtomic, size_t times) {
		for (size_t i = 0; i < times; ++i) myAtomic->add(1);
	}
};

bool parallel_test(size_t p, size_t n) {
	tpie::atomic_int myAtomic;
	boost::thread * threads = new boost::thread[p];
	for (size_t i = 0; i < p; ++i) {
		boost::thread t(atomic_incrementer(), &myAtomic, n);
		t.swap(threads[i]);
	}
	for (size_t i = 0; i < p; ++i) {
		threads[i].join();
	}
	delete[] threads;
	tpie::log_debug() << "Got " << myAtomic.fetch()
		<< ", expected " << p << " * " << n << " = " << (p*n) << std::endl;
	return myAtomic.fetch() == p*n;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic")
		.test(parallel_test, "parallel", "p", static_cast<size_t>(4), "n", static_cast<size_t>(1000000))
		;
}
