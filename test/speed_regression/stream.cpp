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
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include <boost/filesystem/operations.hpp>
#include "testinfo.h"

using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;

const size_t default_mb=10240;

typedef tpie::uint64_t count_t;
typedef tpie::uint64_t test_t;

void usage() {
	std::cout << "Parameters: [times] [mb] [\"file_accessor\"]" << std::endl;
}

void test(size_t mb, size_t times) {
	testinfo t("Stream speed test", 0, mb, times);
    {
	stream<test_t> s("tmp", WRITE_STREAM);
	TPIE_OS_SIZE_T sz = 0;
	s.main_memory_usage(&sz, STREAM_USAGE_MAXIMUM);
	std::cout << "Stream memory usage: " << sz << std::endl;
    }
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
			stream<test_t> s("tmp", WRITE_STREAM);
			test_t x=42;
			for(count_t i=0; i < count; ++i) s.write_item(x);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));
		
		test_t hash = 0;
		getTestRealtime(start);
		{
			stream<test_t> s("tmp", READ_STREAM);
			test_t * x = 0;
			for(count_t i=0; i < count; ++i) {
				s.read_item(&x);
				hash = hash * 13 + *x;
			}
		}
		getTestRealtime(end);
		hash %= 100000000000000ull;
		s(testRealtimeDiff(start,end));
		s(hash);
		boost::filesystem::remove("tmp");
	}
}

struct test_file_accessor {
	const size_t times;
	const memory_size_type itemSize;
	const stream_size_type item_count;
	const stream_size_type block_count;
	const memory_size_type itemsPerBlock;
	testinfo t;
	std::vector<const char *> names;
	tpie::test::stat s;
	test_t * block;
	test_t hash;

	static std::vector<const char *> make_names_vector() {
		std::vector<const char *> names(3);
		names[0] = "Write";
		names[1] = "Read";
		names[2] = "Hash";
		return names;
	}

	test_file_accessor(size_t mb, size_t times)
		: times(times)
		, itemSize(sizeof(test_t))
		, item_count((1<<20)*mb/itemSize)
		, block_count((1<<20)*mb/sysinfo::blocksize_bytes())
		, itemsPerBlock(sysinfo::blocksize_bytes()/itemSize)
		, t("File accessor speed test", 0, mb, times)
		, names(make_names_vector())
		, s(names)
	{
	}

	void go() {
		for (size_t i = 0; i < times; ++i) go_once();
	}

	const static bool fn_write = true;
	const static bool fn_read = false;

	inline void go_once() {
		hash = 0;
		s(time_to<fn_write>());
		s(time_to<fn_read>());
		s(hash % 100000000000000ull);
	}

	template <bool fn>
	inline uint_fast64_t time_to() {
		test_realtime_t start;
		test_realtime_t end;
		getTestRealtime(start);
		block = tpie_new_array<test_t>(itemsPerBlock);
		if (fn == fn_write)
			write();
		else
			read();
		tpie_delete_array(block, itemsPerBlock);
		getTestRealtime(end);
		return testRealtimeDiff(start, end);
	}

	inline void write() {
		tpie::default_file_accessor fa;
		fa.open("tmp", false, true, sizeof(test_t), sysinfo::blocksize_bytes(), 0, access_sequential, false);
		for (count_t j = 0; j < block_count; ++j) {
			for (count_t k = 0; k < itemsPerBlock; ++k) {
				block[k] = 42;
			}
			fa.write_block(block, j, itemsPerBlock);
		}
	}

	inline void read() {
		tpie::default_file_accessor fa;
		fa.open("tmp", true, false, sizeof(test_t), sysinfo::blocksize_bytes(), 0, access_sequential, false);
		for (count_t j = 0; j < block_count; ++j) {
			fa.read_block(block, j, itemsPerBlock);
			for (count_t k = 0; k < itemsPerBlock; ++k) {
				hash = hash * 13 + block[k];
			}
		}
	}
};

int main(int argc, char **argv) {
	size_t times = 4;
	size_t mb = default_mb;

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
	boost::filesystem::remove("tmp");
	if (1 || (argc > 3 && std::string(argv[3]) == "file_accessor")) {
		test_file_accessor tester(mb, times);
		tester.go();
	} else {
		::test(mb, times);
	}
	boost::filesystem::remove("tmp");
	return EXIT_SUCCESS;
}
