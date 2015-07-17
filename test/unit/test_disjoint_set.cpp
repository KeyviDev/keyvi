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
#include <tpie/disjoint_sets.h>
#include <iostream>
#include "test_timer.h"

// Method coverage of tpie::disjoint_sets
//
// Method         Covered by unit test
// ctor(n, u)     basic
// count_sets     basic
// find_set       basic
// is_set         basic
// link           TODO
// make_set       basic
// union_set      basic

using namespace tpie;
using namespace std;

#define DIE(msg) {tpie::log_error() << msg << std::endl; return false;}

bool basic_test() {
	disjoint_sets<int> s1(307);
	for (int i=0; i < 307; ++i) {
		if (s1.is_set(i)) DIE("is_set failed");
		s1.make_set(i);
		if (!s1.is_set(i)) DIE("is_set failed");
		if (s1.count_sets() != (size_t)i+1) DIE("count_sets faild");
	}

	for (int i=1; i < 307; ++i) {
		s1.union_set(i-1, i);
		if (s1.find_set(i-1) != s1.find_set(i)) DIE("find_set failed");
		if (s1.count_sets() != size_t(307-i)) DIE("count_sets failed");
 	}
	return true;
}

class disjointsets_memory_test: public memory_test {
public:
	disjoint_sets<int> * a;
	virtual void alloc() {a = tpie_new<disjoint_sets<int> >(123456);}
	virtual void free() {tpie_delete(a);}
	virtual size_type claimed_size() {return static_cast<size_type>(disjoint_sets<int>::memory_usage(123456));}
};

bool stress_test(int n) {
	test_timer t("disjoint_sets");
	for (int _ = 0; _ < 5; ++_) {
		t.start();
		disjoint_sets<int> s1(n);
		s1.make_set(0);
		for (int i = 1; i < n; ++i) {
			s1.make_set(i);
			s1.union_set(i, i-1);
		}
		for (int i = 0; i < n; ++i) {
			if (s1.find_set(0) != n-1) return false;
		}
		tpie::log_info() << fixed << setprecision(6);
		t.stop();
		t.output();
	}
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic")
		.test(disjointsets_memory_test(), "memory")
		.test(stress_test, "stress", "n", static_cast<int>(1024));
}
