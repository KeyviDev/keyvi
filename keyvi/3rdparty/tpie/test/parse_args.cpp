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

#include "parse_args.h"
#include <cstdlib>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <algorithm>

// The new getopts() argument-parsing function.
#include "getopts.h"
#include <tpie/stream.h>
#include <tpie/memory.h>
#include "test_portability.h"

using namespace tpie;

void parse_args(int argc, char **argv, struct options *application_opts,
		void (*parse_app_opts)(int idx, char *opt_arg), bool stop_if_no_args) {
  bool do_exit = false;

  static struct options standard_opts[] = {
    { 1, "memory", "Set TPIE memory limit", "m", 1 },
    { 2, "verbose", "Set verbose flag", "v", 0 },
    { 3, "test-size", "Set test input size (no. of items)", "t", 1 },
    { 4, "random-seed", "Set random seed", "z", 1 },
  };
  static struct options null_opt = { 0,  NULL, NULL, NULL, 0 };

  static size_t l_std_o = 4;
  size_t l_app_o = 0;
  while (application_opts[l_app_o].number != 0) {
    ++l_app_o;
  }
  struct options *all_opts;
  if (application_opts == NULL && l_app_o != 0) {
    std::cerr << "Error parsing arguments: NULL pointer to options array." << std::endl;
    return;
  }
  
  size_t l_all_o = l_app_o + l_std_o + 1; // add 1 for the null option.

  all_opts = new struct options[l_all_o];
  size_t i;
  for (i = 0; i < l_std_o; i++) {
    all_opts[i] = standard_opts[i];
  }
  for (i = 0; i < l_app_o; i++) {
    all_opts[i+l_std_o] = application_opts[i];
  }
  all_opts[l_app_o+l_std_o] = null_opt;

  if (stop_if_no_args && argc == 1) {
    getopts_usage(argv[0], all_opts);
    exit(0);
  }

  int idx;
  TPIE_OS_SIZE_T mm_sz = DEFAULT_TEST_MM_SIZE;
  unsigned int rnd_seed = DEFAULT_RANDOM_SEED;
  char *opt_arg;

  while ((idx = getopts(argc, argv, all_opts, &opt_arg)) != 0) {
    switch (idx) {
    case 1: 
        // mm_size should be small.
      mm_sz = std::max(size_t(128*1024), parse_number<TPIE_OS_SIZE_T>(opt_arg));
      break;
    case 2:
      verbose = true; 
      TP_LOG_APP_DEBUG_ID("Setting verbose flag.");
      break;
    case 3: 
      test_size = parse_number<TPIE_OS_OFFSET>(opt_arg); 
      break;
    case 4: 
#ifdef _WIN32
		// Suppress warning 4267 (size mismatch of size_t and unsigned int) once.
		// This is recommended by Visual Studio's dynamic help for warning C4267.
#pragma warning(disable : 4267)
#endif
		rnd_seed = std::max<unsigned int>(1, parse_number<unsigned int>(opt_arg));
#ifdef _WIN32
		//  Reset to the default state.
#pragma warning(default : 4267)
#endif
      break;
    default:
      parse_app_opts(idx, opt_arg);
      break;
    }
  }

  // Set memory limit.
  get_memory_manager().set_limit(mm_sz);

  //LOG_APP_DEBUG_ID2("Setting TPIE memory size to: ", mm_sz);

  TPIE_OS_SRANDOM(rnd_seed);
  //LOG_APP_DEBUG_ID2("Setting random seed to: ", rnd_seed);
  
  delete [] all_opts;
  if (do_exit) {
    exit(0);
  }
}

