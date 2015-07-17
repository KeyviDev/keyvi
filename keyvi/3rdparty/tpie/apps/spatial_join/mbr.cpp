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
// File: mbr.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/27/99
//
// $Id: mbr.cpp,v 1.3 2005-11-10 10:35:57 adanner Exp $
//
// Performs a scan of a stream of rectangles to find their minimum
// bounding rectangle (MBR). The MBR is written on cout, as well as in
// a file with the same name as the input file, but with added
// extension .mbr
//

#include <string.h>
#include <fstream>

#include "app_config.h"
#include <ami.h>
#include "rectangle.h"

TPIE_OS_SIZE_T test_mm_size = 0; // Not used.
TPIE_OS_OFFSET test_size = 0; // Not used.
bool verbose = false; // Not used.
int random_seed = 17; // Not used.

class MBRScanner: AMI_scan_object {
protected:
  rectangle mbr;
  ofstream out;
public:
  MBRScanner(char *out_filename): out(out_filename) {
    //out = new ofstream(out_filename);
    mbr.xlo = TP_INFINITY;
    mbr.ylo = TP_INFINITY;
    mbr.xhi = -TP_INFINITY;
    mbr.yhi = -TP_INFINITY;
  }

  AMI_err initialize() {
	  return AMI_ERROR_NO_ERROR;
  }

    AMI_err operate(const rectangle &in, AMI_SCAN_FLAG *sfin) {

    if (*sfin) {
      if (in.xlo < mbr.xlo) mbr.xlo = in.xlo;
      if (in.ylo < mbr.ylo) mbr.ylo = in.ylo;
      if (in.xhi > mbr.xhi) mbr.xhi = in.xhi;
      if (in.yhi > mbr.yhi) mbr.yhi = in.yhi;
    } else {
      out.write((char *) &mbr, sizeof(mbr));
      cout << mbr.xlo << " " << mbr.ylo 
      	   << " " << mbr.xhi << " " << mbr.yhi << endl;
      return AMI_SCAN_DONE;
    }
    return AMI_SCAN_CONTINUE; 
  }
};

int main(int argc, char **argv) {

  if (argc < 2 || argv[1][0] == '-') {
    cerr << "Usage: " << argv[0] << " <TPIE_stream_input> [<MBR_output_file>]" << endl
	 << "(if the output file name is missing, the .mbr extension is added to the input file name)" << endl;
    exit(-1);
  }

  char *output_filename = new char[strlen(argv[1])+5];

  if (argc == 3 && argv[2][0] != '-') {
    strcpy(output_filename, argv[2]);
  } else {
    strcpy(output_filename, argv[1]);
    strcat(output_filename, ".mbr");
  }

  cerr << "Working...";
  AMI_STREAM<rectangle> input_stream(argv[1], AMI_READ_STREAM);
  input_stream.persist(PERSIST_PERSISTENT);

  cerr << "Stream length : " << input_stream.stream_len() << endl;

  MBRScanner scan(output_filename);
  AMI_scan(&input_stream, &scan);
  cerr << "done." << endl;

}
