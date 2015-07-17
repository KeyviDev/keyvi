// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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

#include <tpie/internal_queue.h>

using namespace tpie;

bool basic_test() {
	internal_queue<size_t> q(52);
	for(size_t i=0; i < 52; ++i)
		q.push((i * 104729) % 2251);
	if (q.empty()) return false;
	for(size_t i=0; i < 52; ++i) {
		if (q.size() != 52-i) return false;
		if (q.front() != ((size_t)i * 104729) % 2251) return false;
		q.pop();
	}
	if (!q.empty()) return false;
	return true;
}

class queue_memory_test: public memory_test {
public:
	internal_queue<size_t> * a;
	virtual void alloc() {a = tpie_new<internal_queue<size_t> >(123456);}
	virtual void free() {tpie_delete(a);}
	virtual size_type claimed_size() {return static_cast<size_type>(internal_queue<size_t>::memory_usage(123456));}
};

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic")
		.test(queue_memory_test(), "memory");
}


