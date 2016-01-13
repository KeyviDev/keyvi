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
#include <tpie/parallel_sort.h>
#include <random>
#include <tpie/progress_indicator_arrow.h>
#include <tpie/dummy_progress.h>
#include <tpie/memory.h>
#include "../speed_regression/testtime.h"

//#define TPIE_TEST_PARALLEL_SORT
#ifdef TPIE_TEST_PARALLEL_SORT
	#include <parallel/algorithm>
#endif

static bool stdsort;

using namespace tpie;

template <bool Progress>
struct test_pi {
	struct type : public progress_indicator_arrow {
		type() : progress_indicator_arrow("Sorting", 1, tpie::log_info()) {
		}
	};
};

template <>
struct test_pi<false> {
	typedef dummy_progress_indicator type;
};

template<bool Progress, size_t min_size>
bool basic1(const size_t elements, typename progress_types<Progress>::base * pi) {
	typedef progress_types<Progress> P;

	const size_t stepevery = std::max(static_cast<size_t>(1), elements / 16);
	std::mt19937 prng(42);
	std::vector<int> v1(elements);
	std::vector<int> v2(elements);

	typename P::fp fp(pi);
	typename P::sub gen_p(fp, "Generate", TPIE_FSI, elements, "Generate");
	typename P::sub std_p(fp, "std::sort", TPIE_FSI, elements, "std::sort");
	typename P::sub par_p(fp, "parallel_sort", TPIE_FSI, elements, "parallel_sort");
	fp.init();

	gen_p.init(elements/stepevery);
	size_t nextstep = stepevery;
	for (size_t i = 0; i < elements; ++i) {
		if (i == nextstep) {
			gen_p.step();
			nextstep += stepevery;
		}
		if (stdsort)
			v1[i] = v2[i] = prng();
		else
			v1[i] = prng();
	}
	gen_p.done();

	{
		test_time start=test_now();
		parallel_sort_impl<std::vector<int>::iterator, std::less<int>, Progress, min_size > s(&par_p);
		s(v2.begin(), v2.end());
		test_time end=test_now();
		tpie::log_info() << "Parallel sort took " << test_millisecs(start, end) << std::endl;
	}

	std_p.init(1);
	if (stdsort) {
		test_time start=test_now();
		#ifdef TPIE_TEST_PARALLEL_SORT
		__gnu_parallel::sort(v1.begin(), v1.end());
		#else
		std::sort(v1.begin(), v1.end());
		#endif
		test_time end=test_now();
		tpie::log_info() << "std::sort took " << test_millisecs(start, end) << std::endl;
	}
	std_p.done();

	fp.done();

	if (stdsort && v1 != v2) {
		tpie::log_error() << "std::sort and parallel_sort disagree" << std::endl;
		return false;
	}
	return true;
}

void make_equal_elements_data(std::vector<int> & v) {
	for (size_t i = 0; i < v.size(); ++i) {
		if (i == v.size()-2) v[i] = 1;
		else if (i == v.size()-1) v[i] = 64;
		else v[i] = 42;
	}
}

void make_bad_case_data(std::vector<int> & v) {
	const size_t n = v.size()/8;
	for (size_t i = 0; i < v.size(); ++i) {
		const int el = (i % n && i != (8*n-1)) ? 42 : 36;
		v[i] = el;
	}
}

void make_random_data(std::vector<int> & v) {
	std::mt19937 rng;
	std::generate(v.begin(), v.end(), rng);
}

typedef void (* adversarial_generator) (std::vector<int> &);

template <adversarial_generator Generator>
struct adversarial {
bool operator()(const size_t n, const double seconds) {
	tpie::log_debug() << n << " elements" << std::endl;
	std::vector<int> v(n);
	size_t iterations;
	double dur;
	for (iterations = 1;; iterations += iterations) {
		tpie::log_debug() << iterations << "..." << std::endl;
		test_time t_begin = test_now();
		for (size_t i = 0; i < iterations; ++i) {
			Generator(v);
			std::sort(v.begin(), v.end());
		}
		test_time t_end = test_now();
		dur = test_millisecs(t_begin, t_end);
		if (dur > 1000*seconds) break;
	}
	tpie::log_info() << "Doing " << iterations << " iteration(s) of std::sort takes " << dur << std::endl;

	parallel_sort_impl<std::vector<int>::iterator, std::less<int>, false> s(0);
	test_time t_begin = test_now();
	for (size_t i = 0; i < iterations; ++i) {
		tpie::log_debug() << '.' << std::flush;
		Generator(v);
		s(v.begin(), v.end());
	}
	test_time t_end = test_now();
	tpie::log_debug() << std::endl;
	tpie::log_info() << "std: " << dur << " ours: " << test_millisecs(t_begin, t_end) << std::endl;
	if( dur*3 < test_millisecs(t_begin, t_end)) {tpie::log_error() << "Too slow" << std::endl; return false;}
	return true;
}
};

bool bad_case(const size_t elements, double seconds) {
	const size_t n = elements/8;
	return adversarial<make_bad_case_data>()(8*n, seconds);
}

bool stress_test() {
	std::mt19937 prng(42);
	for (size_t size_base = 1024;; size_base *= 2) {
		for (size_t size = size_base; size < size_base * 2; size += size_base / 4) {
			std::vector<size_t> v1(size);
			std::vector<size_t> v2(size);
			for (size_t i=0; i < size; ++i) {
				v1[i] = v2[i] = prng();
			}
			tpie::log_info() << size << " " << std::flush;

			double t1;
			double t2;
			{
				test_time start=test_now();
				std::sort(v1.begin(), v1.end());
				test_time end=test_now();
				tpie::log_info() << "std: " << (t1 = test_millisecs(start, end)) << std::flush;
			}
			{
				test_time start=test_now();
				parallel_sort_impl<std::vector<size_t>::iterator, std::less<size_t>, false, 524288/8 > s(0);
				s(v2.begin(), v2.end());
				test_time end=test_now();
				tpie::log_info() << " ours: " << (t2 = test_millisecs(start, end)) << std::endl;
			}
			if( t1*3 < t2  ) {tpie::log_error() << "Too slow" << std::endl; return false;}
		}
	}
	return false;
}

template <size_t fields>
struct padded_item {
	padded_item<fields-1> rest;

	int n;

	bool operator<(const padded_item<fields> & other) const {
		return n < other.n;
	}

	bool operator>(const padded_item<fields> & other) const {
		return n > other.n;
	}
};

template <>
struct padded_item<0> {
	int n;

	bool operator<(const padded_item<0> & other) const {
		return n < other.n;
	}

	bool operator>(const padded_item<0> & other) const {
		return n > other.n;
	}
};

template <typename item>
bool large_item_test(size_t mb) {
	size_t items = mb*1024*1024 / sizeof(item);
	std::mt19937 prng(42);
	tpie::log_info() << "Generating array of " << items << " items, each size " << sizeof(item) << std::endl;
	std::vector<item> arr(items);
	for (size_t i = 0; i < items; ++i) {
		arr[i].n = prng();
	}
	tpie::log_info() << "Invoke TPIE parallel sort" << std::endl;
	tpie::parallel_sort(arr.begin(), arr.end(), std::less<item>());
	tpie::log_info() << "Check that array is sorted" << std::endl;
	if (std::adjacent_find(arr.begin(), arr.end(), std::greater<item>()) != arr.end()) {
		tpie::log_error() << "Array not sorted" << std::endl;
		return false;
	} else {
		return true;
	}
}

template <size_t lo, size_t hi>
struct large_item_test_helper {
	static bool go(size_t mb, size_t itemSize) {
		if (itemSize == sizeof(padded_item<lo>)) {
			return large_item_test<padded_item<lo> >(mb);
		} else {
			return large_item_test_helper<lo+1, hi>::go(mb, itemSize);
		}
	}
};

template <size_t lo>
struct large_item_test_helper<lo, lo> {
	static bool go(size_t mb, size_t itemSize) {
		if (itemSize != sizeof(padded_item<lo>)) {
			tpie::log_warning() << "Item size too large; using " << sizeof(padded_item<lo>) << " instead." << std::endl;
		}
		return large_item_test<padded_item<lo> >(mb);
	}
};

bool large_item_test_chooser(size_t mb, size_t itemSize) {
	if (itemSize % sizeof(int) != 0) {
		itemSize -= itemSize % sizeof(int);
		if (itemSize >= sizeof(padded_item<0>)) {
			tpie::log_warning() << "Item size not a multiple of sizeof(int); using " << itemSize << " instead" << std::endl;
		}
	}
	if (itemSize < sizeof(padded_item<0>)) {
		itemSize = sizeof(padded_item<0>);
		tpie::log_warning() << "Item size too small; using " << itemSize << " instead" << std::endl;
	}
	return large_item_test_helper<0, 8>::go(mb, itemSize);
}

template <size_t stdsort_limit>
struct sort_tester {
	bool operator()(size_t n) {
		progress_indicator_arrow pi("Sort", n, tpie::log_info());
		return basic1<true, stdsort_limit>(n, &pi);
	}
};

int main(int argc, char **argv) {
	stdsort = true;
	return tpie::tests(argc, argv)
		.test(sort_tester<2>(), "basic1", "n", 1024*1024)
		.test(sort_tester<8>(), "basic2", "n", 8*8)
		.test(sort_tester<1024*1024>(), "general", "n", 24*1024*1024)
		.test(adversarial<make_equal_elements_data>(), "equal_elements", "n", 1234567, "seconds", 1.0)
		.test(bad_case, "bad_case", "n", 1024*1024, "seconds", 1.0)
		.test(adversarial<make_random_data>(), "general2", "n", 1024*1024, "seconds", 1.0)
		.test(stress_test, "stress_test")
		.test(large_item_test_chooser, "large_item", "mb", static_cast<size_t>(2048), "item-size", static_cast<size_t>(32))
		;
}
