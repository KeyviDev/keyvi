// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
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

#include <tpie/cpu_timer.h>
#include <boost/date_time.hpp>

namespace tpie {

void cpu_timer::set_clock_tick() {
#ifdef _WIN32
	clock_tick_ = CLOCKS_PER_SEC;
#else
	clock_tick_ = sysconf(_SC_CLK_TCK);
	elapsed_.tms_utime = 0;
	elapsed_.tms_stime = 0;
	elapsed_.tms_cutime = 0;
	elapsed_.tms_cstime = 0;
#endif
}

cpu_timer::cpu_timer() :
    clock_tick_(0), last_sync_(), elapsed_(), last_sync_real_(0), elapsed_real_(0),
    running_(false) {
	set_clock_tick();
}

void cpu_timer::sync() {
    tms current_;
#ifdef _WIN32
    current_ = boost::posix_time::second_clock::local_time();
    clock_t current_real_ = clock();
#else
    clock_t current_real_ = times(&current_);
#endif

#ifndef _WIN32
	elapsed_.tms_utime += (current_).tms_utime - last_sync_.tms_utime;
	elapsed_.tms_stime += (current_).tms_stime - last_sync_.tms_stime;
	elapsed_.tms_cutime += (current_).tms_cutime - last_sync_.tms_cutime;
	elapsed_.tms_cstime += (current_).tms_cstime - last_sync_.tms_cstime;
#endif
    
    elapsed_real_ += current_real_ - last_sync_real_;
    
    last_sync_ = current_;
    last_sync_real_ = current_real_;
}

void cpu_timer::last_sync_real_declaration() {
#ifdef _WIN32
	last_sync_real_ = clock();
#else
	last_sync_real_ = times(&last_sync_);	
#endif
}

void cpu_timer::start() {

    if (!running_) {
		last_sync_real_declaration();
	running_ = true;
    }
}

void cpu_timer::stop() {
    if (running_) {
        sync();
        running_ = false;
    }
}

void cpu_timer::reset() {
    if (running_) {		
		last_sync_real_declaration();
    }
    
	set_clock_tick();
    elapsed_real_ = 0;
}

double cpu_timer::user_time() {
    if (running_) sync();
#ifdef _WIN32
	return double(elapsed_real()) / double(clock_tick());
#else
	return double(elapsed().tms_utime) / double(clock_tick());
#endif

}

double cpu_timer::system_time() {
    if (running_) sync();
#ifdef _WIN32
	return double(elapsed_real()) / double(clock_tick());
#else
	return double(elapsed().tms_stime) / double(clock_tick());
#endif
}

double cpu_timer::wall_time() {
    if (running_) sync();
    return double(elapsed_real_) / double(clock_tick_);
}

std::ostream& operator<<(std::ostream &s, cpu_timer &wt) {
    if (wt.running()) {
        wt.sync();
    }
    
#ifdef _WIN32	
    return s << double(wt.elapsed_real()) / double(wt.clock_tick()); 
#else
	return s << double(wt.elapsed().tms_utime) / double(wt.clock_tick()) << "u "
	         << double(wt.elapsed().tms_stime) / double(wt.clock_tick()) << "s "
	         << double(wt.elapsed_real()) / double(wt.clock_tick());	
#endif
}

}
