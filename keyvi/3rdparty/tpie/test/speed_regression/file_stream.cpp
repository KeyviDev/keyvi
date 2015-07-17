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

#include "blocksize_2MB.h"

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include <boost/filesystem/operations.hpp>
#include "testinfo.h"
#include <tpie/types.h>

using namespace tpie;
using namespace tpie::test;

const size_t default_mb=1;

typedef tpie::uint64_t test_t;
typedef tpie::uint64_t count_t;

void usage() {
	std::cout << "Parameters: [times] [mb] [backwards]" << std::endl;
}

test_t read_forwards(count_t count) {
	test_t hash = 0;
	file_stream<test_t> s;
	s.open("tmp");
	for(count_t i=0; i < count; ++i) {
		hash = hash * 13 + s.read();
	}
	return hash;
}

test_t read_backwards(count_t count) {
	test_t hash = 0;
	file_stream<test_t> s;
	s.open("tmp");
	s.seek(count);
	for(count_t i=0; i < count; ++i) {
		hash = hash * 13 + s.read_back();
	}
	return hash;
}

void test(size_t mb, size_t times, bool backwards) {
	std::cout << "file_stream memory usage: " << file_stream<test_t>::memory_usage() << std::endl;
	std::vector<const char *> names;
	names.resize(3);
	names[0] = "Write";
	names[1] = "Read";
	names[2] = "Hash";
	tpie::test::stat s(names);
	count_t count=mb*1024*1024/sizeof(test_t);

	for(size_t i = 0; i < times; ++i) {
		test_realtime_t start;
		test_realtime_t end;

		boost::filesystem::remove("tmp");

		//The purpose of this test is to test the speed of the io calls, not the file system
		getTestRealtime(start);
		{
			file_stream<test_t> s;
			s.open("tmp");
			for(count_t i=0; i < count; ++i) s.write(42);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));

		getTestRealtime(start);
		test_t hash = backwards ? read_backwards(count) : read_forwards(count);
		getTestRealtime(end);
		hash %= 100000000000000ull;
		s(testRealtimeDiff(start,end));
		s(hash);
		boost::filesystem::remove("tmp");
	}
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t mb = default_mb;
	bool backwards = false;

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
		backwards = true;
	}

	testinfo t("file_stream speed test", 0, mb, times);
	::test(mb, times, backwards);
	return EXIT_SUCCESS;
}
