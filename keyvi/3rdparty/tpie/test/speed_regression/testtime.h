// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2005-2009, The TPIE development team
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


#ifndef __TPIE_TESTTIME_H
#define __TPIE_TESTTIME_H
#ifndef WIN32
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <stdint.h>
#include <time.h>
#include <tpie/unittest.h>
///////////////////////////////////////////////////////////////////////////
/// \file testtime.h 
/// Time managment for tests
///////////////////////////////////////////////////////////////////////////

namespace tpie {
	namespace test {
#ifndef WIN32
		///////////////////////////////////////////////////////////////////
		/// Type used for vaiable holding time information
		///////////////////////////////////////////////////////////////////
		typedef struct rusage test_time_t;

		///////////////////////////////////////////////////////////////////
		/// Sample the time and store it
		///////////////////////////////////////////////////////////////////
		inline void getTestTime(test_time_t &a) {
			getrusage(RUSAGE_SELF, &a);
		}


		
		///////////////////////////////////////////////////////////////////
		/// Calculate page fault difference
		///////////////////////////////////////////////////////////////////
		inline uint_fast64_t testPagefaultDiff(const test_time_t& a, const test_time_t& b) {
			return b.ru_majflt - a.ru_majflt;
		}

		///////////////////////////////////////////////////////////////////
		/// Calculate io usage in blocks
		///////////////////////////////////////////////////////////////////
		inline uint_fast64_t testIODiff(const test_time_t& a, const test_time_t& b) {
			return b.ru_inblock - a.ru_inblock + b.ru_oublock - a.ru_oublock;
		}
		
		///////////////////////////////////////////////////////////////////
		/// Calculate time difference in micro seconds
		///////////////////////////////////////////////////////////////////
		inline uint_fast64_t testTimeDiff(const test_time_t& a, const test_time_t& b) {
			uint_fast64_t time = b.ru_utime.tv_sec - a.ru_utime.tv_sec;
			time *= 1000*1000;
			time += b.ru_utime.tv_usec - a.ru_utime.tv_usec;
			return time;
		}
#endif

		///////////////////////////////////////////////////////////////////
		/// Type used for vaiable holding real time information
		///////////////////////////////////////////////////////////////////
		typedef tpie::test_time test_realtime_t;

		///////////////////////////////////////////////////////////////////
		/// Sample the real time and store it
		///////////////////////////////////////////////////////////////////
		inline void getTestRealtime(test_time& a) {
			a = tpie::test_now();
		}	

		///////////////////////////////////////////////////////////////////
		/// Calculate real time difference in micro seconds
		///////////////////////////////////////////////////////////////////
		inline uint_fast64_t testRealtimeDiff(const test_realtime_t a, const test_realtime_t b) {
			return (uint_fast64_t)tpie::test_millisecs(a, b);
		}
	}
}
#endif //__TPIE_TESTTIME_H
