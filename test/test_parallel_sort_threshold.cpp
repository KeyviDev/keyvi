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

#include <tpie/tpie.h>
#include <tpie/unittest.h>
#include <tpie/parallel_sort.h>
#include <random>
#include <tpie/sysinfo.h>

typedef int test_t;

struct sort_timer {
	virtual ~sort_timer() {}
	virtual double run(std::vector<test_t> & items) = 0;
};

template <size_t stdSortThreshold>
struct sort_timer_impl : public sort_timer {
	~sort_timer_impl() {}
	double run(std::vector<test_t> & items) {
		tpie::test_time start=tpie::test_now();
		tpie::parallel_sort_impl<std::vector<test_t>::iterator, std::less<test_t>, false, stdSortThreshold > s(0);
		s(items.begin(), items.end());
		tpie::test_time end=tpie::test_now();
		return tpie::test_secs(start, end);
	}
};

template <size_t N, size_t Inc, size_t Rem>
struct inc;
template <size_t N, size_t Inc>
struct inc<N, Inc, 0> {
	static const size_t val = N;
};

template <size_t N, size_t Inc, size_t Rem>
struct inc {
	static const size_t val = inc<N+Inc, Inc, Rem-Inc>::val;
};

const size_t thresholdMax = 4*1024*1024;
const size_t thresholdInc = 16*1024;

template <size_t N>
struct get_sort_timer_calc {
	inline static sort_timer * calc(size_t stdSortThreshold) {
		const size_t next = inc<N, thresholdInc, thresholdMax-N>::val;
		if (stdSortThreshold < next || N == next) return new sort_timer_impl<N>();
		return get_sort_timer_calc<next>::calc(stdSortThreshold);
	}
};

sort_timer * get_sort_timer(size_t stdSortThreshold) {
	return get_sort_timer_calc<thresholdInc>::calc(stdSortThreshold);
};

// fill a vector with random numbers (the same random numbers each time).
void fill_data(std::vector<test_t> & data) {
	std::mt19937 rng; // default seed
	std::generate(data.begin(), data.end(), rng);
}

// search the interval [center-radius, center+radius] for the std::sort
// threshold that yields the best run-time when sorting as much data as `data'
// contains.
size_t find_threshold(size_t center, size_t radius, std::vector<test_t> & data) {
	size_t inc = radius/4;
	size_t lo = (center > radius) ? (center-radius) : 0;
	size_t hi = lo + 2*radius;

	size_t best = 0;
	double besttime = std::numeric_limits<double>::max();
	for (size_t attempt = lo; attempt <= hi; attempt += inc) {
		sort_timer * t = get_sort_timer(attempt);
		fill_data(data);
		std::cout << attempt << ' ' << std::flush;
		double dur = t->run(data);
		std::cout << dur << std::endl;
		if (attempt == lo || dur < besttime) {
			best = attempt;
			besttime = dur;
		}
	}
	std::cout << "Chose " << best << std::endl;
	return best;
}

int main(int argc, char ** argv) {
	// argument parsing
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " mb" << std::endl;
		return 1;
	}
	tpie::sysinfo si;
	std::cout << si << '\n';
	size_t mb;
	std::stringstream ss(argv[1]);
	ss >> mb;
	std::cout << si.custominfo("Data (MB)", mb) << std::endl;
	std::vector<test_t> data(mb*1024*1024/sizeof(test_t));

	// program
	tpie::tpie_init();
	size_t center = thresholdMax/2;
	size_t radius = thresholdMax/2;

	do {
		center = find_threshold(center, radius, data);
		radius = radius/2;
	} while (radius >= thresholdInc);
	tpie::tpie_finish();
	return 0;
}
