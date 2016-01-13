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

#include <tpie/tpie.h>
#include <tpie/unittest.h>
#include <tpie/job.h>
#include <tpie/sysinfo.h>
#include <boost/lexical_cast.hpp>
using namespace tpie;

struct simple_job : public tpie::job {
	size_t jobs;
	simple_job * parent;
	simple_job(size_t jobs, simple_job * parent = 0) : jobs(jobs), parent(parent) {}
	~simple_job() {}
	void operator()() {
		if (!jobs) return;
		simple_job * j = new simple_job(jobs-1, parent ? parent : this);
		//std::cout << "Meep!" << std::endl;
		j->enqueue(parent ? parent : this);
	}
	void child_done(tpie::job * child) {
		delete child;
	}
};

int main(int argc, char ** argv) {
	tpie::tpie_init();
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " [jobs]" << std::endl;
		return 1;
	}
	size_t jobs = boost::lexical_cast<size_t>(argv[1]);
	test_time start=test_now();
	simple_job j(jobs);
	j.enqueue();
	j.join();
	test_time end=test_now();
	std::cout << "Running " << jobs << " jobs took " << test_millisecs(start, end) << "ms" << std::endl;
	tpie::tpie_finish();
	return 0;
}
