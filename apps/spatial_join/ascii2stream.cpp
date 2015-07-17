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

// Copyright (c) 1999 Octavian Procopiuc
//
// File: ascii2stream.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/26/99
// 
// Transforms an ASCII file containing rectangles (as in rectangle.H)
// into a TPIE stream.
//
#include "app_config.h"
#include <fstream>
#include "rectangle.h"
#include "my_ami_scan_utils.h"

TPIE_OS_SIZE_T test_mm_size = 0; // Not used.
TPIE_OS_OFFSET test_size = 0; // Not used.
bool verbose = false; // Not used.
int random_seed = 17; // Not used.

int main(int argc, char **argv)
{

  AMI_err err;
  istream *file_stream;
  char *out_filename;
  if (argc < 2) {
    cerr << "Transforms ASCII rectangles file into TPIE stream.\n" 
	 << "Usage: " << argv[0] << " [ <input_file> ] <output_file>\n";
    exit(-1);
  } else if (argc == 2) {
    file_stream = &cin;
    out_filename = argv[1];
  } else {
    file_stream = new ifstream(argv[1]);
    out_filename = argv[2];
  }
  my_cxx_istream_scan<rectangle> *scanner;
  AMI_STREAM<rectangle> *out_stream;
  out_stream = new AMI_STREAM<rectangle>(out_filename);
  out_stream->persist(PERSIST_PERSISTENT);
  cerr << "Working...";

  scanner = new my_cxx_istream_scan<rectangle>(file_stream);
  err = AMI_scan(scanner, out_stream);
  delete scanner;
  delete out_stream;

  if (err != AMI_ERROR_NO_ERROR) {
    cerr << "\nError while parsing data: " << hex 
	 << err << " (see ami_base.H)" << endl;
    exit(-1);
  } else
    cerr << " done.\n";
}
