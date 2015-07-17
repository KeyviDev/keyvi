// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013, The TPIE development team
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

#ifndef TPIE_TEST_MERGE_SORT_H
#define TPIE_TEST_MERGE_SORT_H

struct relative_memory_usage {
	inline relative_memory_usage(memory_size_type extraMemory)
		: m_startMemory(actual_used())
		, m_threshold(0)
		, m_extraMemory(extraMemory)
	{
	}

	inline memory_size_type used() {
		return actual_used() - m_startMemory;
	}

	inline void set_threshold(memory_size_type threshold) {
		m_threshold = threshold;
		get_memory_manager().set_limit(m_startMemory + m_threshold + m_extraMemory);
	}

	inline bool below(memory_size_type threshold) {
		if (used() > threshold) {
			report_overusage(threshold);
			return false;
		}
		return true;
	}

	inline bool below() {
		return below(m_threshold);
	}

	void report_overusage(memory_size_type threshold) {
		log_error() << "Memory usage " << used() << " exceeds threshold " << threshold << std::endl;
	}

private:
	inline static memory_size_type actual_used() {
		return get_memory_manager().used();
	}

	memory_size_type m_startMemory;
	memory_size_type m_threshold;
	memory_size_type m_extraMemory;
};

template <typename Traits>
class sort_tester {
	typedef typename Traits::test_t test_t;
	typedef typename Traits::sorter sorter;
	typedef typename Traits::item_generator item_generator;

static bool sort_test(memory_size_type m1,
					  memory_size_type m2,
					  memory_size_type m3,
					  double mb_data,
					  memory_size_type extraMemory = 0,
					  bool evacuateBeforeMerge = false,
					  bool evacuateBeforeReport = false)
{
	m1 *= 1024*1024;
	m2 *= 1024*1024;
	m3 *= 1024*1024;
	extraMemory *= 1024*1024;
	const stream_size_type bytes = static_cast<stream_size_type>(mb_data*(1024*1024));
	item_generator gen(bytes);
	const stream_size_type items = gen.items();
	log_debug() << "sort_test with " << items << " items\n";
	relative_memory_usage m(extraMemory);
	sorter s;
	s.set_available_memory(m1, m2, m3);

	log_debug() << "Begin phase 1" << std::endl;
	m.set_threshold(m1);
	s.begin();
	if (!m.below()) return false;
	for (stream_size_type i = 0; i < items; ++i) {
		s.push(gen());
		if (!m.below()) return false;
	}
	s.end();
	if (!m.below()) return false;

	if (evacuateBeforeMerge) {
		s.evacuate();
		log_debug() << "MEMORY USED: " << m.used() << '/' << s.evacuated_memory_usage() << std::endl;
		if (!m.below(s.evacuated_memory_usage())) return false;
	}

	log_debug() << "Begin phase 2" << std::endl;
	m.set_threshold(m2);
	if (!m.below()) return false;
	Traits::merge_runs(s);
	if (!m.below()) return false;

	if (evacuateBeforeReport) {
		s.evacuate();
		log_debug() << "MEMORY USED: " << m.used() << '/' << s.evacuated_memory_usage() << std::endl;
		if (!m.below(s.evacuated_memory_usage())) return false;
	}

	log_debug() << "Begin phase 3" << std::endl;
	m.set_threshold(m3);
	if (!m.below()) return false;
	test_t prev = std::numeric_limits<test_t>::min();
	memory_size_type itemsRead = 0;
	while (s.can_pull()) {
		if (!m.below()) return false;
		test_t read = s.pull();
		if (!m.below()) return false;
		if (read < prev) {
			log_error() << "Out of order" << std::endl;
			return false;
		}
		prev = read;
		++itemsRead;
	}
	if (itemsRead != items) {
		log_error() << "Read the wrong number of items. Got " << itemsRead << ", expected " << items << std::endl;
		return false;
	}

	log_debug() << "MEMORY USED: " << m.used() << '/' << s.evacuated_memory_usage() << std::endl;
	if (!m.below(s.evacuated_memory_usage())) return false;

	return true;
}

static bool empty_input_test() {
	return sort_test(100,100,100,0);
}

static bool internal_report_test() {
	return sort_test(100,100,100,40);
}

static bool internal_report_after_resize_test() {
	return sort_test(100,80,30,20, 100);
}

static bool one_run_external_report_test() {
	return sort_test(100,7,7,7.1);
}

static bool external_report_test() {
	return sort_test(20,20,20,50);
}

static bool small_final_fanout_test(double mb) {
	return sort_test(3,12,7,mb);
}

static bool evacuate_before_merge_test() {
	return sort_test(20,20,20,8, 0, true, false);
}

static bool evacuate_before_report_test() {
	return sort_test(20,20,20,50, 0, false, true);
}

public:

static tests & add_all(tests & t) {
	return t
		.test(empty_input_test, "empty_input")
		.test(internal_report_test, "internal_report")
		.test(internal_report_after_resize_test, "internal_report_after_resize")
		.test(one_run_external_report_test, "one_run_external_report")
		.test(external_report_test, "external_report")
		.test(small_final_fanout_test, "small_final_fanout", "mb", 8.0)
		.test(evacuate_before_merge_test, "evacuate_before_merge")
		.test(evacuate_before_report_test, "evacuate_before_report")
		;
}

};

#endif // TPIE_TEST_MERGE_SORT_H
