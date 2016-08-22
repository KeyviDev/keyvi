// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2012, 2013, The TPIE development team
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
#include <tpie/pipelining/merge_sorter.h>
#include <tpie/parallel_sort.h>
#include <tpie/sysinfo.h>
#include <random>

using namespace tpie;

#include "merge_sort.h"

class use_merge_sort {
public:
	typedef uint64_t test_t;
	typedef merge_sorter<test_t, false> sorter;

	class item_generator {
	private:
		stream_size_type m_items;
		std::mt19937 m_rng;

	public:
		item_generator(stream_size_type bytes)
			: m_items(bytes / sizeof(test_t))
		{
		}

		stream_size_type items() const { return m_items; }
		test_t operator()() { return m_rng(); }
	};

	static void merge_runs(sorter & s) {
		dummy_progress_indicator pi;
		s.calc(pi);
	}
};

bool sort_upper_bound_test_base(memory_size_type dataUpperBound) {
	typedef use_merge_sort Traits;
	typedef Traits::sorter sorter;
	typedef Traits::test_t test_t;

	memory_size_type m1 = 100 *1024*1024;
	memory_size_type m2 = 20  *1024*1024;
	memory_size_type m3 = 20  *1024*1024;
	memory_size_type dataSize = 15*1024*1024;

	memory_size_type items = dataSize / sizeof(test_t);

	stream_size_type io = get_bytes_written();

	relative_memory_usage m(0);
	sorter s;
	s.set_available_memory(m1, m2, m3);
	s.set_items(dataUpperBound / sizeof(test_t));

	m.set_threshold(m1);
	s.begin();
	for (stream_size_type i = 0; i < items; ++i) {
		s.push(i);
	}
	s.end();
	Traits::merge_runs(s);
	while (s.can_pull()) s.pull();

	TEST_ENSURE_EQUALITY(io, get_bytes_written(), "The number of bytes written was not correct.")
	return true;
}

bool sort_upper_bound_test() {
	return sort_upper_bound_test_base(80*1024*1024);
}

bool sort_faulty_upper_bound_test() {
	return sort_upper_bound_test_base(400);  // much lower than dataSize
}

bool temp_file_usage_test() {
	bool result = true;
	const stream_size_type initialUsage = get_temp_file_usage();
	log_debug() << "Offsetting get_temp_file_usage by " << initialUsage
				<< std::endl;
	const memory_size_type runLength = get_block_size() / sizeof(size_t);
	const memory_size_type runs = 16;
	const stream_size_type expectedUsage = runLength * runs * sizeof(size_t);
	// Presumably, the data is not compressed beyond 1 byte per item.
	const stream_size_type expectedUsageLowerBound = runLength * runs;
	const memory_size_type fanout = runs;
	{
		merge_sorter<size_t, false> s;
		s.set_parameters(runLength, fanout);
		s.begin();
		for (size_t i = 0; i < runs * runLength; ++i) {
			s.push(i);
		}
		s.end();
		const stream_size_type afterPhase1 = get_temp_file_usage()
											 - initialUsage;
		log_debug() << "After phase 1: " << afterPhase1 << std::endl;
		if (afterPhase1 < expectedUsageLowerBound) {
			log_error() << "Used less space than expected!\nExpected is "
						<< expectedUsage << ", but " << afterPhase1
						<< " is way below that (< " << expectedUsageLowerBound
						<< ")" << std::endl;
			result = false;
		}
		dummy_progress_indicator pi;
		s.calc(pi);
		const stream_size_type afterPhase2 = get_temp_file_usage()
											 - initialUsage;
		log_debug() << "After phase 2: " << afterPhase2 << std::endl;
		while (s.can_pull()) s.pull();
		const stream_size_type afterPhase3 = get_temp_file_usage()
											 - initialUsage;
		log_debug() << "After phase 3: " << afterPhase3 << std::endl;
	}
	const stream_size_type afterDestroy = get_temp_file_usage() - initialUsage;
	log_debug() << "After destroying merge_sorter: " << afterDestroy
				<< std::endl;
	return result;
}

bool tall_tree_test(size_t fanout, size_t height) {
	{
		merge_sorter<size_t, false> s;
		const memory_size_type runLength = get_block_size() / sizeof(size_t);
		memory_size_type runs = 1;
		for (size_t i = 0; i < height; ++i) runs *= fanout;
		runs += 1;
		s.set_parameters(runLength, fanout);
		s.begin();
		for (size_t i = runs * runLength; i--;) {
			s.push(i);
		}
		s.end();
		dummy_progress_indicator pi;
		s.calc(pi);
		while (s.can_pull()) s.pull();
	}
	return true;
}

int main(int argc, char ** argv) {
	tests t(argc, argv);
	return
		sort_tester<use_merge_sort>::add_all(t)
		.test(sort_upper_bound_test, "sort_upper_bound")
		.test(sort_faulty_upper_bound_test, "sort_faulty_upper_bound")
		.test(temp_file_usage_test, "temp_file_usage")
		.test(tall_tree_test, "tall_tree", "fanout", static_cast<size_t>(6), "height", static_cast<size_t>(1))
		;
}
