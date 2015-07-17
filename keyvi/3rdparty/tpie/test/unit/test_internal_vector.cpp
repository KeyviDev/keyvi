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

#include <tpie/internal_vector.h>

using namespace tpie;

int item(size_t i) {
	return (static_cast<int>(i) * 104729) % 2251;
}

bool basic_test(size_t n = 52) {
	internal_vector<int> s(n);
	for(size_t i=0; i < n; ++i)
		s.push_back(item(i));
	for(size_t i=n; i--;) {
		if (s.size() != (size_t)i+1)
			return false;
		if (s.back() != item(i))
			return false;
		s.pop_back();
	}
	if (!s.empty()) return false;
	return true;
}

class vector_memory_test: public memory_test {
public:
	internal_vector<int> * a;
	virtual void alloc() {
		a = tpie_new< internal_vector<int> >(123456);
	}
	virtual void free() {
		tpie_delete(a);
	}
	virtual size_type claimed_size() {
		return static_cast<size_type>(internal_vector<int>::memory_usage(123456));
	}
};

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic", "size", 52)
		.test(basic_test, "medium", "size", 1000000)
		.test(basic_test, "large", "size", 100000000)
		.test(vector_memory_test(), "memory");
}


