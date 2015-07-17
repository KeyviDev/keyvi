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

#include "blocksize_2MB.h"

#include <tpie/tpie.h>
#include <tpie/pipelining/sort.h>
#include <tpie/prime.h>
#include <vector>
#include "stat.h"
#include "testinfo.h"
#include "testtime.h"

using namespace tpie;
using namespace tpie::test;
using namespace tpie::pipelining;

struct item {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;

	item & operator=(uint32_t v) {
		a = v;
		return *this;
	}

	item & operator=(uint64_t v) {
		a = static_cast<uint32_t>(v);
		return *this;
	}

	std::pair<uint32_t, uint32_t> repr() const {
		const uint32_t mask = 0x55555555u;
		return std::make_pair(a & mask, a & ~mask);
	}

	bool operator<(const item & other) const {
		return repr() < other.repr();
	}
};

typedef item test_t;

struct testparams {
	stream_size_type elements;
	size_t times;
	memory_size_type runLength;
	size_t fanout;
	memory_size_type memoryLimit;
};

void parse_args(int argc, char ** argv, testparams & p) {
	int i = 1;
	while (i < argc) {
		std::string arg(argv[i]);
		if (arg == "-r") {
			std::stringstream ss(argv[i+1]);
			ss >> p.runLength;
			i += 2;
		} else if (arg == "-f") {
			std::stringstream ss(argv[i+1]);
			ss >> p.fanout;
			i += 2;
		} else if (arg == "-m") {
			std::stringstream ss(argv[i+1]);
			ss >> p.memoryLimit;
			i += 2;
		} else if (arg == "-h" || arg == "--help") {
			std::cout << "Usage: " << argv[0] << " [-r runLength] [-f fanout] [-m memoryLimit] <elements> <times>" << std::endl;
			exit(1);
		} else break;
	}
	if (i < argc) {
		std::stringstream ss(argv[i]);
		ss >> p.elements;
		++i;
	}
	if (i < argc) {
		std::stringstream ss(argv[i]);
		ss >> p.times;
		++i;
	}
}

void test(testparams & p) {
	std::vector<const char *> names(4);
	names[0] = "Form runs";
	names[1] = "Merge";
	names[2] = "Verify";
	names[3] = "Correct";
	tpie::test::stat stats(names);
	for (size_t i = 0; i < p.times; ++i) {
		test_realtime_t start;
		test_realtime_t end;

		getTestRealtime(start);

		merge_sorter<test_t, false> m;
		m.set_available_memory(p.memoryLimit);
		if (p.runLength) {
			m.set_parameters(p.runLength, p.fanout);
		}
		m.begin();
		test_t x;
		for (stream_size_type j = 0; j < p.elements; ++j) {
			x = j;
			m.push(x);
		}
		m.end();

		getTestRealtime(end);
		stats(testRealtimeDiff(start, end));

		dummy_progress_indicator pi;
		getTestRealtime(start);
		m.calc(pi);
		getTestRealtime(end);
		stats(testRealtimeDiff(start, end));

		bool good = true;
		getTestRealtime(start);
		x = m.pull();
		for (stream_size_type j = 1; j < p.elements; ++j) {
			tp_assert(m.can_pull(), "Can pull");
			test_t y = m.pull();
			if (y < x) good = false;
			x = y;
		}
		getTestRealtime(end);
		stats(testRealtimeDiff(start, end));
		stats(good ? 1 : 0);
	}
}

int main(int argc, char ** argv) {
	testparams p = {10, 1, 0, 0, 50*1024*1024};
	parse_args(argc, argv, p);
	testinfo t("Pipelining sort speed test", 1024, 0, p.times);
	sysinfo s;
	s.printinfo("Elements", p.elements);
	if (p.runLength) {
		s.printinfo("Run length", p.runLength);
		s.printinfo("Fanout", p.fanout);
	} else {
		s.printinfo("Run length", "(default)");
		s.printinfo("Fanout", "(default)");
	}
	::test(p);
	return 0;
}
