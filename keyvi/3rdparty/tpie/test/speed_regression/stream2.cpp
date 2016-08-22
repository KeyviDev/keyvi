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

#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <random>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include "testtime.h"
#include "stat.h"
#include "testinfo.h"

using namespace tpie;
using namespace tpie::test;

typedef tpie::uint64_t count_t;
typedef tpie::uint64_t test_t;

void usage() {
	std::cout << "Parameters: <times> <mb> <series>" << std::endl;
}

class series_base {
public:
	virtual void write(file_stream<test_t> & s, count_t n) = 0;
	virtual ~series_base() {}
	virtual const char * name() const = 0;
};

template <typename child_t>
class series_crtp : public series_base {
	child_t & self() { return *static_cast<child_t *>(this); }
	const child_t & self() const { return *static_cast<const child_t *>(this); }
public:
	void begin(count_t) {
	}
	void end(count_t) {
	}

	virtual void write(file_stream<test_t> & s, count_t n) override {
		self().begin(n);
		for (count_t i = 0; i < n; ++i) {
			s.write(self().item(i, n));
		}
		self().end(n);
	}

	const char * name() const override { return self().get_name(); }
};

class series_random : public series_crtp<series_random> {
	std::mt19937 rng;
public:
	const char * get_name() const { return "random"; }

	test_t item(count_t, count_t) {
		return rng();
	}
};

class series_constant : public series_crtp<series_constant> {
public:
	const char * get_name() const { return "constant"; }

	test_t item(count_t, count_t) {
		return 42;
	}
};

class series_naturals : public series_crtp<series_naturals> {
public:
	const char * get_name() const { return "naturals"; }

	test_t item(count_t i, count_t) {
		return i;
	}
};

class series_quadratic : public series_crtp<series_quadratic> {
public:
	const char * get_name() const { return "quadratic"; }

	test_t item(count_t i, count_t) {
		return i*i;
	}
};

void test(size_t mb, size_t times, series_base * series) {
	testinfo t("Stream speed test", 0, mb, times);
	sysinfo().printinfo("Series", series->name());
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

		getTestRealtime(start);
		{
			file_stream<test_t> s;
			s.open("tmp");
			series->write(s, count);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));

		test_t hash = 0;
		getTestRealtime(start);
		{
			file_stream<test_t> s;
			s.open("tmp");
			for(count_t i=0; i < count; ++i) {
				hash = hash * 13 + s.read();
			}
		}
		getTestRealtime(end);
		hash %= 100000000000000ull;
		s(testRealtimeDiff(start,end));
		s(hash);
		boost::filesystem::remove("tmp");
	}
}

series_base * choose_series(std::string arg) {
	if (arg == "random") return new series_random();
	else if (arg == "constant") return new series_constant();
	else if (arg == "naturals") return new series_naturals();
	else if (arg == "quadratic") return new series_quadratic();
	else {
		std::cout << "Invalid series. Choose one of:\n"
			"    random\n"
			"    constant\n"
			"    naturals\n"
			"    quadratic\n"
			;
		return 0;
	}
}

int main(int argc, char **argv) {
	size_t times;
	size_t mb;
	if (argc < 4) {
		usage();
		return EXIT_FAILURE;
	}

	std::stringstream(argv[1]) >> times;
	std::stringstream(argv[2]) >> mb;
	series_base * series = choose_series(argv[3]);
	if (!series) return EXIT_FAILURE;
	boost::filesystem::remove("tmp");
	::test(mb, times, series);
	boost::filesystem::remove("tmp");
	delete series;
	return EXIT_SUCCESS;
}
