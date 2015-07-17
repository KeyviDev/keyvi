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

#include <tpie/internal_stack.h>

using namespace tpie;

bool basic_test() {
	internal_stack<size_t> s(52);
	for(size_t i=0; i < 52; ++i)
		s.push((i * 104729) % 2251);
	for(int i=51; i >= 0; --i) {
		if (s.size() != (size_t)i+1) return false;
		if (s.top() != static_cast<size_t>((i * 104729) % 2251)) return false;
		s.pop();
	}
	if (!s.empty()) return false;
	return true;
}

class stack_memory_test: public memory_test {
public:
	internal_stack<int> * a;
	virtual void alloc() {a = tpie_new<internal_stack<int> >(123456);}
	virtual void free() {tpie_delete(a);}
	virtual size_type claimed_size() {return static_cast<size_type>(internal_stack<int>::memory_usage(123456));}
};

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic")
		.test(stack_memory_test(), "memory");
}
