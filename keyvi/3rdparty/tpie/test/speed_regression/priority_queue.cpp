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

#include "../app_config.h"

#include "blocksize_128KB.h"

#include <tpie/tpie.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include <tpie/priority_queue.h>
#include "testinfo.h"
#include <random>
#include <tpie/types.h>

using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;

const size_t mb_default=1;

void usage() {
	std::cout << "Parameters: [times] [mb]" << std::endl;
}

struct intgenerator {
	typedef tpie::uint64_t item_type;
	item_type i;
	inline intgenerator() : i(91493) {}
	inline item_type operator()() { return 104729*(i++); }
	inline void use(item_type & a, item_type & x) { a ^= x; }
};

std::mt19937 rng;
std::uniform_real_distribution<double> double_dist;
std::uniform_real_distribution<float> float_dist;

struct segmentgenerator {
	typedef std::pair<double, std::pair<double, float> > item_type;
	inline item_type operator()() { return std::make_pair(double_dist(rng), std::make_pair(double_dist(rng), float_dist(rng))); }
	inline void use(item_type & a, item_type & x) { a.first += x.second.first; }
};

template <typename Generator>
void test(Generator g, size_t mb, size_t times, float blockFactor = 0.125f) {
	typedef typename Generator::item_type test_t;
	test_t a = test_t();

	std::vector<const char *> names;
	names.resize(2);
	names[0] = "Push";
	names[1] = "Pop";

	tpie::test::stat s(names);
	TPIE_OS_OFFSET count=TPIE_OS_OFFSET(mb)*1024*1024/sizeof(test_t);
	for (size_t i=0; i < times; ++i) {
		
		test_realtime_t start;
		test_realtime_t end;
		getTestRealtime(start);
		{
			tpie::ami::priority_queue<test_t> pq(0.95f, blockFactor);
		
			for(TPIE_OS_OFFSET i=0; i < count; ++i) {
				test_t x = g();
				pq.push(x);
			}
			getTestRealtime(end);
			s(testRealtimeDiff(start,end));

			getTestRealtime(start);
			for(TPIE_OS_OFFSET i=0; i < count; ++i) {
				test_t x=pq.top();
				pq.pop();
				g.use(a, x);
			}
			getTestRealtime(end);
			s(testRealtimeDiff(start,end));
		}
	}
	if (a == g()) std::cout << "oh rly" << std::endl;
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t mb = mb_default;
	float blockFactor = 0.125;
	bool segments = false;

	int i;
	for (i = 1; i < argc; ++i) {
		std::string arg(argv[i]);
		if (arg == "-s") {
			segments = true;
		} else {
			break;
		}
	}
	if (i < argc) {
		if (std::string(argv[i]) == "0") {
			times = 0;
		} else {
			std::stringstream(argv[i]) >> times;
			if (!times) {
				usage();
				return EXIT_FAILURE;
			}
		}
		++i;
	}
	if (i < argc) {
		std::stringstream(argv[i]) >> mb;
		if (!mb) {
			usage();
			return EXIT_FAILURE;
		}
		++i;
	}
	if (i < argc) {
		if (!(std::stringstream(argv[i]) >> blockFactor) || blockFactor*2*1024*1024 < 8) {
			usage();
			return EXIT_FAILURE;
		}
		++i;
	}

	testinfo t("Priority queue speed test", 1024, mb, times);
	sysinfo().printinfo("Block factor", blockFactor);
	if (segments) {
		sysinfo().printinfo("Item type", "segments");
		::test(intgenerator(), mb, times, blockFactor);
	} else {
		sysinfo().printinfo("Item type", "64-bit integers");
		::test(segmentgenerator(), mb, times, blockFactor);
	}
	return EXIT_SUCCESS;
}
