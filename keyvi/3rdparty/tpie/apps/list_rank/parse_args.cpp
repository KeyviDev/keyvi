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

static char parse_args_id[] = "$Id: parse_args.cpp,v 1.15 2005-11-15 15:33:40 jan Exp $";

#include <portability.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>

#include "app_config.h"
#include "parse_args.h"

static TPIE_OS_OFFSET parse_number(char *s) {
  TPIE_OS_OFFSET n; 
  TPIE_OS_OFFSET mult = 1;
  size_t len = strlen(s);
  if(isalpha(s[len-1])) {
    switch(s[len-1]) {
    case 'M':
      mult = 1 << 20;
      break;
    case 'K':
      mult = 1 << 10;
      break;
    default:
      cerr << "bad number format: " << s << "\n";
      exit(-1);
      break;
    }
    s[len-1] = '\0';
  }
  n = (TPIE_OS_OFFSET)atol(s);
  return n * mult;
}


void parse_args(int argc, char **argv, const char *as_opts,
                void (*parse_app_opt)(char opt, char *optarg))
{
  static char standard_opts[] = "m:t:z:v";
  
  // All options, standard and application specific.
  char *all_opts;

  if (as_opts != NULL) {
    size_t l_aso;
    
    all_opts = new char[sizeof(standard_opts) + (l_aso = strlen(as_opts))]; 
    strncpy(all_opts, standard_opts, sizeof(standard_opts));
    strncat(all_opts, as_opts, l_aso);
  } else {
    all_opts = standard_opts;
  }
  
  char c;
  
  optarg = NULL;
  while((c = getopt(argc, argv, all_opts)) != -1) {
    switch (c) {
    case 'v':
      verbose = true;
      break;
    case 'm':
      // mm_size should be small.
      test_mm_size = static_cast<TPIE_OS_SIZE_T>(parse_number(optarg));
      break;                
    case 't':
      test_size = parse_number(optarg);
      break;
    case 'z':
      random_seed = atol(optarg);
      break;
    default:
      parse_app_opt(c, optarg);
      break;
    }
  }
}

