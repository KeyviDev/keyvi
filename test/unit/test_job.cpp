// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
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

#include "common.h"
#include <tpie/job.h>
#include <tpie/array.h>

class test_job : public tpie::job {
	size_t * ctr;
public:
	test_job(size_t * ctr)
		: ctr(ctr)
	{
	}

	void operator()() {
		++*ctr;
	}
};

bool repeat_test() {
	size_t workers = 10;
	size_t times = 10;
	tpie::array<size_t> scratch(workers*64, 0);
	tpie::array<tpie::unique_ptr<test_job> > jobs(workers);
	for (size_t i = 0; i < workers; ++i) {
		jobs[i].reset(tpie::tpie_new<test_job>(&scratch[64*i]));
	}
	for (size_t j = 0; j < times; ++j) {
		for (size_t i = 0; i < workers; ++i) {
			jobs[i]->enqueue();
		}
		for (size_t i = 0; i < workers; ++i) {
			jobs[i]->join();
		}
		for (size_t i = 0; i < workers; ++i) {
			if (scratch[64*i] != j+1) {
				tpie::log_error() << "Did not increase counter " << i << " to " << j+1 << std::endl;
				return false;
			}
		}
	}
	tpie::log_info() << "Increased all " << workers << " counters " << times << " times" << std::endl;
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(repeat_test, "repeat")
		;
}
