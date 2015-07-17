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
// $Id: mbr.cpp,v 1.2 2004-02-05 17:54:14 jan Exp $
//
// Performs a scan of a stream of rectangles to find their
// minimum bounding rectangle (MBR).
//

#include "app_config.h"

#include <tpie/portability.h>

#include <tpie/stream.h>

#include "rectangle.h"
#include <tpie/scan.h>
#include <tpie/block.h>
#include <iostream>

using namespace tpie::ami;

///////////////////////////////////////////////////////////////////////////
/// Scan all rectangles in the input stream, computes the mininum 
/// bounding box, and write it to an output file.
///////////////////////////////////////////////////////////////////////////
template<class coord_t, class oid_t>
class MBRScanner: public scan_object {

#define MAGIC_NUMBER_UNINITIALIZED_RECTANGLE (oid_t)17

private:
    rectangle<coord_t, oid_t> mbr;
    std::ofstream *out;

public:

    ///////////////////////////////////////////////////////////////////////////
    /// The constructor expects a filename for the output file where the
    /// minimum bounding rectangle is to be written to
    /// \param out_filename filename for the output file
    ///////////////////////////////////////////////////////////////////////////
    MBRScanner(std::string out_filename) {
	out = new std::ofstream(out_filename.c_str());

	assert(MAGIC_NUMBER_UNINITIALIZED_RECTANGLE != 0);
	// ...otherwise the hack won't work.

	mbr.set_id(MAGIC_NUMBER_UNINITIALIZED_RECTANGLE);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    /// Nothing happens here
    ///////////////////////////////////////////////////////////////////////////
    err initialize() {
	return NO_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// The current minimum bounding rectangle is extended to enclose the
    /// rectangle passed to this method.
    /// \param[in] in current rectangle read from input stream
    /// \param[in] sfin (scan flag)
    ///////////////////////////////////////////////////////////////////////////
    err operate(const rectangle<coord_t, oid_t> &in, SCAN_FLAG *sfin) {

	if (*sfin) {
	    if (mbr.get_id() == MAGIC_NUMBER_UNINITIALIZED_RECTANGLE) {
		mbr = in;
		// ...un-hack.
		mbr.set_id(0);
	    }
	    else {
		mbr.extend(in);
	    }
	} else {
	    out->write((char *) &mbr, sizeof(mbr));
	    std::cerr << " " << mbr.get_left() << " " << mbr.get_lower() 
		      << " " << mbr.get_right() << " " << mbr.get_upper() << " ";
	    return SCAN_DONE;
	}
	return SCAN_CONTINUE; 
    }
};

int main(int argc, char **argv) {

    if (argc < 2) {
	std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
	exit(-1);
    }

    std::cerr << "Working...";
    stream<rectangle<double, bid_t> > input_stream(argv[1]);
    input_stream.persist(tpie::PERSIST_PERSISTENT);

    std::cerr << "Stream length : "
	      << input_stream.stream_len() 
	      << std::endl;

    std::string output_filename(argv[1]);

    MBRScanner<double, bid_t> scanObject(output_filename+".mbr");
    scan(&input_stream, &scanObject);

    std::cerr << "done." << std::endl;

    return 0;
}
