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
#include <tpie/queue.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include "testinfo.h"

using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;

const size_t mb_default=1;

typedef tpie::uint64_t count_t; // number of items
typedef tpie::uint64_t elm_t; // type of element we enqueue

void usage() {
	std::cout << "Parameters: [times] [mb] [memory] [continous_count] [compressed]" << std::endl;
}

void test(size_t mb, size_t times, size_t countinous_count, compression_flags compressionFlags) {
	std::vector<const char *> names;
	names.resize(1);
	names[0] = "Push/pop";
	tpie::test::stat s(names);
	count_t count=static_cast<count_t>(mb)*1024*1024/sizeof(elm_t);

	for (size_t i=0; i < times; ++i) {
		test_realtime_t start;
		test_realtime_t end;

		tpie::queue<elm_t> q(access_sequential, compressionFlags);
		getTestRealtime(start);
		{
			for(count_t j = 0; j < count; j += countinous_count) {
				for(count_t l = j; l < j+countinous_count && l < count; ++l) {
					elm_t x = (l + 91493) * 104729;
					q.push(x);
				}

				for(count_t l = j; l < j+countinous_count && l < count; ++l) {
					q.pop();
				}
			}
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));
	}
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t mb = mb_default;
	size_t memory = 1024;
	size_t countinous_count = 1; // the number of push and pop operations to appear after eachother
	compression_flags compressionFlags = compression_none;

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
	if (argc > 4) {
		std::stringstream(argv[4]) >> countinous_count;
		if (!countinous_count) {
			usage();
			return EXIT_FAILURE;
		}
	}
	if (argc > 5) {
		compressionFlags = compression_normal;
	}

	testinfo t("Queue speed test", memory, mb, times);
	sysinfo().printinfo("Compression", (compressionFlags == compression_normal) ? "Enabled" : "Disabled");
	::test(mb, times, countinous_count, compressionFlags);
	return EXIT_SUCCESS;
}
