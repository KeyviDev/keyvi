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

#include <tpie/stream.h>
#include <iostream>
#include "testtime.h"
#include <tpie/progress_indicator_arrow.h>
using namespace tpie::ami;
using namespace tpie::test;


uint64_t size= 1024ull*1024ull*1024ull*16ull;
int main() {
  test_realtime_t start;
  test_realtime_t end;
	
  getTestRealtime(start);
  tpie::progress_indicator_arrow pi("Test", size);
  pi.init(size);
  for(uint64_t i=0; i < size; ++i) {
    pi.step();
  }
  pi.done();

  getTestRealtime(end);
  std::cout << "Time " << testRealtimeDiff(start,end) << std::endl;
}
