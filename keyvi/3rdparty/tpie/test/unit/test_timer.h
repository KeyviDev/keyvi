// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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
#ifndef __TEST_UNIT_TEST_TIMER_H__
#define __TEST_UNIT_TEST_TIMER_H__

#include <string>
#include <iostream>
#include <algorithm>
#include <tpie/unittest.h>

class test_timer {
public:
	test_timer(const std::string & n): name(n) {sum=0; ssum=0; l=1000000000000ll; h=0; tests=0;}
	inline double avg() {return sum/tests;}
	inline double var() {return ssum/tests-avg()*avg();}
	inline double sd() {return sqrt(var());}
	
	inline void start() {t=tpie::test_now();}
	void stop() {
		double time = tpie::test_secs(t, tpie::test_now());
		sum += time;
		ssum += time*time;
		tests += 1;
		l = std::min(l, time);
		h = std::max(h, time);
	}
	void output() {
		tpie::log_info() << name <<": "  
			"Tests: " << tests << "; " << 
			"avg: " << avg() << "s; " << 
			"sd: " << sd() << "s^2; " << 
			"min: " << l << "s; " <<
			"max: " << h << "s;" << std::endl;
	}
private:
	std::string name;
	double sum,ssum,l,h;
	int tests;
	tpie::test_time t;
};

#endif //__TEST_UNIT_TEST_TIMER_H__
