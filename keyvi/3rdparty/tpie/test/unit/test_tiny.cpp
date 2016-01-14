// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2015 The TPIE development team
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
#include <iostream>
#include <random>
#include <tpie/tiny.h>
#include <set>

bool sort_test() {
	{
		std::vector<size_t> l1;
		std::vector<size_t> l2(l1);
		std::sort(l1.begin(), l1.end());
		tpie::tiny::sort(l2.begin(), l2.end());
		if (l1 != l2) return false;
	}

	{
		std::vector<size_t> l1{42};
		std::vector<size_t> l2(l1);
		std::sort(l1.begin(), l1.end());
		tpie::tiny::sort(l2.begin(), l2.end());
		if (l1 != l2) return false;
	}

	
	{
		std::vector<size_t> l1{11, 32};
		std::vector<size_t> l2(l1);
		std::sort(l1.begin(), l1.end(), std::greater<size_t>());
		tpie::tiny::sort(l2.begin(), l2.end(), std::greater<size_t>());
		if (l1 != l2) return false;
	}

	{
		std::mt19937 mt_rand(42);
		std::vector<size_t> l1;
		for (size_t i=0; i < 100; ++i) l1.push_back(mt_rand());
		std::vector<size_t> l2(l1);
		std::sort(l1.begin(), l1.end());
		tpie::tiny::sort(l2.begin(), l2.end());
		if (l1 != l2) return false;
	}
	return true;
}

template <typename T1, typename T2>
bool my_equal(T1 s1, T1 e1, T2 s2, T2 e2) {
	if (std::distance(s1, e1) != std::distance(s2, e2)) return false;
	return std::equal(s1, e1, s2);
}

bool set_test() {
	std::set<int> s1;
	tpie::tiny::set<int> s2;
	std::mt19937 mt_rand(42);

	TEST_ENSURE_EQUALITY(s1.empty(), s2.empty(), "empty");
	for (size_t i=0; i < 100; ++i) {
		size_t n=mt_rand();
		s1.insert(n);
		s2.insert(n);
	}

	TEST_ENSURE_EQUALITY(s1.empty(), s2.empty(), "");
	TEST_ENSURE_EQUALITY(s1.size(), s2.size(), "");
	TEST_ENSURE(my_equal(s1.begin(), s1.end(), s2.begin(), s2.end()), "begin");
	TEST_ENSURE(my_equal(s1.cbegin(), s1.cend(), s2.cbegin(), s2.cend()), "cbegin");
	TEST_ENSURE(my_equal(s1.rbegin(), s1.rend(), s2.rbegin(), s2.rend()), "rbegin");
	TEST_ENSURE(my_equal(s1.crbegin(), s1.crend(), s2.crbegin(), s2.crend()), "crbegin");
	TEST_ENSURE(s2.max_size() >= s2.size(), "max_size");
	TEST_ENSURE(s2.capacity() >= s2.size(), "capacity");
	// Make sure that all methods compile
	s2.reserve(0);
	s2.shrink_to_fit();
	tpie::unused(s2.key_comp());
	tpie::unused(s2.value_comp());
	tpie::unused(s2.get_allocator());

	s2.clear();
	TEST_ENSURE(s2.empty(), "empty");
	s2.insert({12, 11});
	TEST_ENSURE_EQUALITY(s2.count(11), 1, "count");
	TEST_ENSURE_EQUALITY(s2.count(10), 0, "count");
	s2.emplace(14);
	TEST_ENSURE_EQUALITY(s2.count(14), 1, "count");

	auto a1=s2.insert(16);
	TEST_ENSURE_EQUALITY(*a1.first, 16, "insert");
	TEST_ENSURE(a1.second, "insert");
	a1=s2.insert(16);
	TEST_ENSURE_EQUALITY(*a1.first, 16, "insert");
	TEST_ENSURE(!a1.second, "insert");
	TEST_ENSURE_EQUALITY(*s2.lower_bound(15), 16, "lower_bound");
	TEST_ENSURE_EQUALITY(*s2.lower_bound(14), 14, "lower_bound");
	TEST_ENSURE_EQUALITY(*s2.upper_bound(15), 16, "upper_bound");
	TEST_ENSURE_EQUALITY(*s2.upper_bound(14), 16, "upper_bound");
	auto a2=s2.equal_range(14);
	TEST_ENSURE_EQUALITY(*a2.first, 14, "equal_range");
	TEST_ENSURE_EQUALITY(*a2.second, 16, "equal_range");
	TEST_ENSURE_EQUALITY(*s2.find(14), 14, "find");
	TEST_ENSURE(s2.find(15) == s2.end(), "find");
	TEST_ENSURE_EQUALITY(s2.erase(11), 1, "erase");
	TEST_ENSURE_EQUALITY(s2.erase(10), 0, "erase");
	TEST_ENSURE_EQUALITY(s2.count(11), 0, "count");
	s2.erase(s2.find(14));
	TEST_ENSURE_EQUALITY(s2.count(14), 0, "count");

	tpie::tiny::set<int>::const_iterator i = s2.erase(s2.begin(), s2.end());
	tpie::tiny::set<int>::const_iterator j = s2.end();
	TEST_ENSURE(i == j, "erase");
	TEST_ENSURE(s2.empty(), "empty");
	return true;
}

bool map_test() {
	// We share most of the code with set, so we do not need to test much
	std::map<int, char> s1;
	tpie::tiny::map<int, char> s2;
	
	s1.emplace(12,'a');
	s2.emplace(12,'a');

	s1[14]='b';
	s2[14]='b';

	TEST_ENSURE_EQUALITY(s1.at(12) ,s2.at(12), "at");
	TEST_ENSURE_EQUALITY(s1.at(14), s2.at(14), "at");
	try {
		s2.at(10);
		return false;
	} catch(std::out_of_range) {}

	TEST_ENSURE_EQUALITY(s2.count(14), 1, "count");
	s2.erase(14);
	TEST_ENSURE_EQUALITY(s2.count(14), 0, "count");
	return true;
}

bool multiset_test() {
	tpie::tiny::multiset<int> s1({14, 17, 14, 10});
	TEST_ENSURE_EQUALITY(s1.count(14), 2, "count");
	TEST_ENSURE_EQUALITY(s1.count(17), 1, "count");
	TEST_ENSURE_EQUALITY(s1.count(10), 1, "count");
	TEST_ENSURE_EQUALITY(s1.count(9), 0, "count");
	TEST_ENSURE_EQUALITY(s1.size(), 4, "size");
	TEST_ENSURE_EQUALITY(s1.erase(14), 2, "erase");
	TEST_ENSURE_EQUALITY(s1.count(14), 0, "count");
	TEST_ENSURE_EQUALITY(s1.size(), 2, "size");
	return true;
}


bool multimap_test() {
	tpie::tiny::multimap<int, char> s1({{14, 'a'}, {17, 'b'} , {14, 'c'}, {10, 'd'}});
	TEST_ENSURE_EQUALITY(s1.count(14), 2, "count");
	TEST_ENSURE_EQUALITY(s1.count(17), 1, "count");
	TEST_ENSURE_EQUALITY(s1.count(10), 1, "count");
	TEST_ENSURE_EQUALITY(s1.count(9), 0, "count");
	TEST_ENSURE_EQUALITY(s1.size(), 4, "size");
	TEST_ENSURE_EQUALITY(s1.erase(14), 2, "erase");
	TEST_ENSURE_EQUALITY(s1.count(14), 0, "count");
	TEST_ENSURE_EQUALITY(s1.size(), 2, "size");
	return true;
}


int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(sort_test, "sort")
		.test(set_test, "set")
		.test(map_test, "map")
		.test(multiset_test, "multiset")
		.test(multimap_test, "multimap")
		;
}

