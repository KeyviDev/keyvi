// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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
#include <iostream>
#include <tpie/util.h>
#include <boost/iterator/iterator_facade.hpp>
#include <tpie/memory.h>
#include <tpie/packed_array.h>
#include <tpie/unittest.h>
#include <tpie/tpie_log.h>

typedef int test_t;

template <int B>
bool basic_test(const size_t elements) {
	typedef tpie::packed_array<test_t, B> Array;
	const test_t range = 1 << B;
	const test_t init = 43 % range;
	Array a(elements, init);
	test_t incrementBy = 0;
	for (size_t i = 0; i < elements; ++i) {
		if (*a.find(i) != init) {
			std::cerr << 'a' << i << " Got " << *a.find(i) << ", expected " << init << std::endl;
			return false; // XXX
		}
		a[i] = a[i] + incrementBy;
		++incrementBy;
	}
	test_t expect = init;
	incrementBy = 0;
	size_t n = 0;
	for (typename Array::iterator i = a.begin(); i != a.end(); ++i) {
		if (static_cast<size_t>(i-a.begin()) != n) {
			std::cerr << "Ptrdiff is wrong, got " << (i-a.begin()) << " expected " << n << std::endl;
			return false; // XXX
		}
		expect = (init + incrementBy) % range;

		if (*i != expect) {
			std::cerr << 'b' << (i-a.begin()) << " Got " << *i << ", expected " << expect << std::endl;
			return false; // XXX
		}

		*i = *i - incrementBy;

		++incrementBy;
		++n;
	}

	n = 0;
	for (typename Array::reverse_iterator i = a.rbegin(); i != a.rend(); ++i) {
		if (static_cast<size_t>(i-a.rbegin()) != n) {
			std::cerr << "Ptrdiff is wrong, got " << (i-a.rbegin()) << " expected " << n << std::endl;
			return false; // XXX
		}
		if (*i != init) {
			std::cerr << 'c' << (i-a.rbegin()) << " Got " << *i << ", expected " << init << std::endl;
			return false; // XXX
		}
		++n;
	}

	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(basic_test<1>, "basic1", "n", 0x243F6A)
		.test(basic_test<2>, "basic2", "n", 0x243F6A)
		.test(basic_test<4>, "basic4", "n", 0x243F6A);
}
