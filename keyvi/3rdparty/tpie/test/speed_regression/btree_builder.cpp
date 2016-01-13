// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2015, The TPIE development team
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
#include <tpie/btree/btree_builder.h>
#include <tpie/btree/btree.h>
#include <tpie/btree/external_store.h>
#include <tpie/queue.h>
#include <iostream>
#include <set>
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
	std::cout << "Parameters: [times] [mb]" << std::endl;
}

void test(size_t mb, size_t times) {
	std::vector<const char *> names;
	names.resize(2);
	names[0] = "Builder";
	names[1] = "Inserts";
	tpie::test::stat s(names);
	count_t count=static_cast<count_t>(mb)*1024*1024/sizeof(elm_t);
    tpie::get_memory_manager().set_limit(1000 * 1024 * 1024);

	for (size_t i=0; i < times; ++i) {
		test_realtime_t start;
		test_realtime_t end;

		getTestRealtime(start);
		{
            temp_file tmp;
            btree_builder<int, btree_external> builder(tmp.path());
            for(count_t i = 0; i < count; ++i)
                builder.push(i);
            auto tree(builder.build());
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));

		getTestRealtime(start);
		{
            temp_file tmp;
			btree<int, btree_external> tree(tmp.path());
            for(count_t i = 0; i < count; ++i)
                tree.insert(i);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));
	}
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t mb = mb_default;

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

    log_info() << "Repetitions: " << times << std::endl;
    log_info() << "Test size: " << mb << " MB" << std::endl;

	testinfo t("Btree builder speed regresssion test", mb, times);
	::test(mb, times);
	return EXIT_SUCCESS;
}
