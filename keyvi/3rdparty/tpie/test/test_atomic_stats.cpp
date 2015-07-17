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

#include <iostream>
#include <boost/thread.hpp>
#include <tpie/tpie.h>
#include <tpie/sysinfo.h>
#include <tpie/array.h>
#include <tpie/stats.h>

namespace {

using namespace tpie;

void usage() {
	std::cout << "Run without arguments to run test." << std::endl;
}

void print_version() {
	tpie::sysinfo si;
	std::cout << si << std::endl;
}

static const stream_size_type x = 0x100000001ull;
static const size_t timesPerWorker = 1000000;
static const size_t workers = 8;
static const stream_size_type expectedValue = x * timesPerWorker * workers;

void increase_stats_worker() {
	for (size_t i = 0; i < timesPerWorker; ++i) {
		tpie::increment_temp_file_usage(x);
	}
}

void increase_stats() {
	std::cout << "Run " << workers << " workers, each incrementing the statistic "
		<< timesPerWorker << " times." << std::endl;
	tpie::array<boost::thread> threads(workers);
	for (size_t i = 0; i < workers; ++i) {
		boost::thread t(increase_stats_worker);
		threads[i].swap(t);
	}
	for (size_t i = 0; i < workers; ++i) {
		threads[i].join();
	}
}

bool check_stats() {
	return tpie::get_temp_file_usage() == expectedValue;
}

} // unnamed namespace

int main(int argc, char ** argv) {
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--help") {
			usage();
			return 1;
		}
		if (arg == "--version") {
			print_version();
			return 1;
		}
	}
	tpie::tpie_init();
	increase_stats();
	bool result = check_stats();
	tpie::tpie_finish();
	if (result) {
		std::cout << "Success!" << std::endl;
		return 0;
	} else {
		std::cout << "Wrong result." << std::endl;
		return 1;
	}
}
