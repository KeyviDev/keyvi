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
// Transforms an ASCII file containing rectangles (as in rectangle.h)
// into a TPIE stream.
//
#include "app_config.h"

#include <tpie/portability.h>

#include <fstream>

#include "rectangle.h"
#include <tpie/block.h>  // for bid_t
#include <tpie/scan_utils.h>
#include <tpie/stream.h>

using namespace tpie::ami;

int main(int argc, char **argv) {

    err err;
    std::istream *file_stream;
    char *out_filename;
    
    if (argc < 2) {
	std::cerr << "Transforms ASCII rectangles file into TPIE stream.\n" 
		  << "Usage: " << argv[0] 
		  << " [ <input_file> ] <output_file>" << std::endl;
	exit(-1);
    } else if (argc == 2) {
	file_stream = &std::cin;
	out_filename = argv[1];
    } else {
	file_stream = new std::ifstream(argv[1]);
	out_filename = argv[2];
    }
    
    cxx_istream_scan<rectangle<double, bid_t> > scanner(file_stream); 
    stream<rectangle<double, bid_t> > *out_stream = NULL;
    out_stream = new stream<rectangle<double, bid_t> >(out_filename); 
    out_stream->persist(tpie::PERSIST_PERSISTENT);
    
    std::cerr << "Working...";
    err = scan(&scanner, out_stream);
    
    delete out_stream;
    
    if (err != NO_ERROR) {
	std::cerr << "\nError while parsing data: " << std::hex 
		  << err << " (see tpie/err.h)" << std::endl;
	exit(-1);
    } else
	std::cerr << " done." << std::endl;
    
    return 0;
}
