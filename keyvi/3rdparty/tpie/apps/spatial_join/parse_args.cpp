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

//
// File: parse_args.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//

static char parse_args_id[] = "$Id: parse_args.cpp,v 1.4 2005-11-15 15:33:41 jan Exp $";

#include <portability.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <algorithm>
//using std::max;

// The new getopts() argument-parsing function.
#include "getopts.h"

#include "app_config.h"
#include "parse_args.h"
#include <ami_stream.h>

static size_t parse_number(char *s) {
  size_t n; 
  size_t mult = 1;
  size_t len = strlen(s);
  if(isalpha(s[len-1])) {
    switch(s[len-1]) {
    case 'G':
    case 'g':
      mult = 1 << 30;
      break;
    case 'M':
    case 'm':
      mult = 1 << 20;
      break;
    case 'K':
    case 'k':
      mult = 1 << 10;
      break;
    default:
      cerr << "Error parsing arguments: bad number format: " << s << "\n";
      exit(-1);
      break;
    }
    s[len-1] = '\0';
  }
  n = size_t(atof(s) * mult);
  return n;
}

void parse_args(int argc, char **argv, struct options *application_opts,
		void (*parse_app_opts)(int idx, char *opt_arg)) {
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
    cerr << "Error parsing arguments: NULL pointer to options array." << endl;
    return;
  }
  
  size_t l_all_o = l_app_o + l_std_o + 1; // add 1 for the null option.

  all_opts = new struct options[l_all_o];
  int i;
  for (i = 0; i < l_std_o; i++) {
    all_opts[i] = standard_opts[i];
  }
  for (i = 0; i < l_app_o; i++) {
    all_opts[i+l_std_o] = application_opts[i];
  }
  all_opts[l_app_o+l_std_o] = null_opt;

  int idx;
  TPIE_OS_SIZE_T mm_sz = DEFAULT_TEST_MM_SIZE;
  unsigned int rnd_seed = DEFAULT_RANDOM_SEED;
  char *opt_arg;

  while ((idx = getopts(argc, argv, all_opts, &opt_arg)) != 0) {
    switch (idx) {
    case 1: 
      mm_sz = max(TPIE_OS_SIZE_T(128*1024), parse_number(opt_arg));
      break;
    case 2:
      verbose = true; 
     TP_LOG_APP_DEBUG_ID("Setting verbose flag.");
      break;
    case 3: 
      test_size = parse_number(opt_arg); 
      break;
    case 4: 
      rnd_seed = max(1u, (unsigned int) parse_number(opt_arg));
      break;
    default:
      parse_app_opts(idx, opt_arg);
      break;
    }
  }

  // Set memory limit.
  MM_manager.set_memory_limit(mm_sz);
  //LOG_APP_DEBUG_ID2("Setting TPIE memory size to: ", mm_sz);

  TPIE_OS_SRANDOM(rnd_seed);
  //LOG_APP_DEBUG_ID2("Setting random seed to: ", rnd_seed);
  
  delete [] all_opts;
  if (do_exit) {
    exit(0);
  }
}
