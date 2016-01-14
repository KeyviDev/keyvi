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
#include "common.h"
#include <tpie/internal_priority_queue.h>
#include <vector>
#include "priority_queue.h"

// Method coverage of tpie::internal_priority_queue
//
// Method                    Covered by unit test
// ctor(max_size)            basic
// ctor(max_size, IT, IT)    TODO
// clear                     TODO
// empty                     TODO
// get_array                 TODO
// insert                    TODO
// make_safe                 TODO
// pop                       basic
// pop_and_push              TODO
// push                      basic
// resize                    TODO
// size                      basic
// top                       basic
// unsafe_push               TODO

using namespace tpie;

bool basic_test() {
	size_t z = 104729;
	internal_priority_queue<uint64_t, bit_pertume_compare<std::greater<uint64_t> > > pq(z);
	return basic_pq_test(pq, z);
}

bool large_cycle(){
	size_t x = 524*1024*102;
	internal_priority_queue<uint64_t, bit_pertume_compare<std::greater<uint64_t> > > pq(x);
	return cyclic_pq_test(pq, x, 20000000);
}

class my_memory_test: public memory_test {
public:
	internal_priority_queue<int> * a;
	virtual void alloc() {a = tpie_new<internal_priority_queue<int> >(123456);}
	virtual void free() {tpie_delete(a);}
	virtual size_type claimed_size() {return static_cast<size_type>(internal_priority_queue<int>::memory_usage(123456));}
};

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic")
		.test(large_cycle, "large_cycle")
		.test(my_memory_test(), "memory");
}
