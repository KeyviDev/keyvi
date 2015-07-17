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

// Copyright (c) 2002 Octavian Procopiuc
//
// File:         sssj.cpp
// Authors:      Octavian Procopiuc <tavi@cs.duke.edu>
//               and Jan Vahrenhold <jan@cs.duke.edu>
// Created:      01/24/99
// Description:  Join two sets using sort and sweep.
//
// $Id: sssj.cpp,v 1.3 2005-11-10 10:35:57 adanner Exp $
//
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::istream;
using std::ostream;

#include "app_config.h"
#include <ami_stream.h>
#include "sorting_adaptor.h"
#include "sortsweep.h"
#include "parse_args.h"
#include "joinlog.h"

// default file names for input and output streams...
static char def_if_red[] = "rectangles.red";
static char def_if_blue[] = "rectangles.blue";
static char def_of[] = "output";

static char *input_filename_red = def_if_red;
static char *input_filename_blue = def_if_blue;
static char *output_filename = def_of;

bool verbose = false;
TPIE_OS_SIZE_T test_mm_size = 64*1024*1024; // Default mem. size.
TPIE_OS_OFFSET test_size = 0; // Not used.
int random_seed = 17;


// set the additional command line parameters to be parsed.

void usage(const char* progname) {
      cerr << "  Usage: sortjoin" << endl 
           << "\t[ -R red_input_stream]" << endl
           << "\t[ -B blue_input_stream ]" << endl
	   << "\t[ -o output_file_name ]" << endl
	   << "\t[ -m memory_size ]" << endl;
}

static struct options as_opts[]={
  { 5, "red", "Red input Stream", "R", 1 },
  { 6, "blue", "Blue input Stream", "B", 1 },
  { 7, "out", "Output file name", "o", 1 },
};

void parse_app_opt(int idx, char *optarg)
{
    switch (idx) {
    case 5:
      input_filename_red = optarg;
      break;
    case 6:
      input_filename_blue = optarg;
      break;
    case 7:
      output_filename = optarg;
      break;
    }
}

AMI_err join() {

  sort_sweep* sweeper;
  AMI_STREAM<pair_of_rectangles>* output_stream;
  AMI_err err;
  TPIE_OS_SIZE_T sz_avail;

  // Create the output stream.
  output_stream = new AMI_STREAM<pair_of_rectangles>(output_filename);
  output_stream->persist(PERSIST_PERSISTENT);

  JoinLog jl("SRTS", input_filename_red,  0, input_filename_blue, 0);
  sz_avail = MM_manager.memory_available();
  cerr << "Beginning. Avail. mem: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(sz_avail) << endl;
  jl.UsageStart();

  sweeper = new sort_sweep(input_filename_red, input_filename_blue,
			   output_stream);
  sz_avail = MM_manager.memory_available();
  cerr << "Finished sorting. Avail. memory: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(sz_avail) << endl;

  err = sweeper->run();

  jl.setRedLength(sweeper->redSize());
  jl.setBlueLength(sweeper->blueSize());
  jl.UsageEnd("SortJoin:", output_stream->stream_len());

  cerr << "Output length: " << output_stream->stream_len() << endl;

  delete sweeper;
  delete output_stream;
  return err;
}


int main(int argc, char** argv) {

  AMI_err err;

  tpie_log_init(TPIE_LOG_APP_DEBUG);

  if (argc < 2) {
    usage(argv[0]);
    exit(1);
  }
  // Parse the command line arguments.
  parse_args(argc,argv,as_opts,parse_app_opt);

  // Set the main memory size. 
  //MM_manager.set_memory_limit(test_mm_size); //done by parse_args
  MM_manager.enforce_memory_limit();

  if ((err = join()) != AMI_ERROR_NO_ERROR)
    cout << "Error: " << err << endl;

  return 0;
}

