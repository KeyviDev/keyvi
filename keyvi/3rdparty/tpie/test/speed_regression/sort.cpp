// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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

#include "blocksize_2MB.h"

#include <tpie/tpie.h>
#include <tpie/stream.h>
#include <tpie/sort.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include <boost/filesystem/operations.hpp>
#include "testinfo.h"

using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;

const size_t mb_default=1;

typedef tpie::uint64_t count_t; // number of items
typedef tpie::uint64_t elm_t; // type of element we sort

void usage() {
	std::cout << "Parameters: [times] [mb] [memory]" << std::endl;
}

void test(size_t mb, size_t times) {
	std::vector<const char *> names;
	names.resize(3);
	names[0] = "Write";
	names[1] = "Sort";
	names[2] = "Hash";
	tpie::test::stat s(names);
	count_t count=static_cast<count_t>(mb)*1024*1024/sizeof(elm_t);
	for (size_t i=0; i < times; ++i) {
		
		test_realtime_t start;
		test_realtime_t end;
		
		boost::filesystem::remove("tmp");
		
		//The purpose of this test is to test the speed of the io calls, not the file system
		getTestRealtime(start);
		{
			stream<elm_t> s("tmp", WRITE_STREAM);
			for(count_t i=0; i < count; ++i) {
				elm_t x= (i+ 91493)*104729;
				s.write_item(x);
			}
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));
		
		getTestRealtime(start);
		{
			stream<elm_t> s("tmp");
			tpie::ami::sort(&s);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));

		elm_t hash = 0;
		elm_t prev = 0;
		bool sorted = true;
		{
			stream<elm_t> s("tmp", READ_STREAM);
			for(count_t i=0; i < count; ++i) {
				elm_t *x = 0;
				s.read_item(&x);
				if (i > 0 && prev > *x) {
					sorted = false;
				}
				prev = *x;
				hash = hash * 13 + *x;
			}
		}
		boost::filesystem::remove("tmp");
		hash %= 100000000000000ull;
		s(hash);
		if (!sorted) std::cout << "\nNot sorted!" << std::endl;
	}
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t mb = mb_default;
	size_t memory = 1024;
			
	if (argc > 1) {
		if (std::string(argv[1]) == "0") {
			times = 0;
		} else {
			std::stringstream(argv[1]) >> times;
			if (!times) {
				usage();
				return EXIT_FAILURE;
			}
		}
	}
	if (argc > 2) {
		std::stringstream(argv[2]) >> mb;
		if (!mb) {
			usage();
			return EXIT_FAILURE;
		}
	}
	if (argc > 3) {
		std::stringstream(argv[3]) >> memory;
		if (!memory) {
			usage();
			return EXIT_FAILURE;
		}
	}

	testinfo t("Sort speed test", memory, mb, times);
	::test(mb, times);
	return EXIT_SUCCESS;
}
