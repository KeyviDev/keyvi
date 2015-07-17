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
//  Description:     source code for program buildtree
//  Created:         09.02.1999
//  Author:          Jan Vahrenhold
//  mail:            jan.vahrenhold@math.uni-muenster.de
//  $Id: build_rtree.cpp,v 1.2 2004-08-12 12:37:24 jan Exp $
//
//  Copyright (C) 1999-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

#include "app_config.h"

#include <tpie/portability.h>
#include <tpie/stream.h>

#include "bulkloader.h"

// #define COUNT_ALL_OBJECTS

using namespace tpie;

int main(int argc, char** argv) {

	get_memory_manager().set_enforcement(memory_manager::ENFORCE_IGNORE);
    //  i.e., TPIE is not in control of memory allocation and does not
    //  complain if more than  test_mm_size is allocated.
    
    // Set the main memory size. 
	get_memory_manager().set_limit(25*1024*1024);    

    if (argc < 4) {
        std::cerr << "Missing command parameter." << std::endl;
        std::cerr << "Usage: buildtree <input_stream> <fanout> <R|H>" << std::endl;
    }
    else {

	ami::rstartree<double>* tree = NULL;
	
	std::cerr << std::endl;
	std::cerr << "----------------------------------------------------------------------" << std::endl;
        std::cerr << std::endl << "Creating ";
	if (!strcmp(argv[3], "H")) {
	    std::cerr << "Hilbert";
	}
	else {
	    std::cerr << "R*";
	}
	std::cerr << "-Tree (fanout=" << atol(argv[2]) << ") for " << argv[1] << "...";

	ami::bulkloader<double> bl(argv[1], (unsigned short)atol(argv[2]));
	ami::err result = ami::NO_ERROR;

	if (!strcmp(argv[3], "H")) {
	    result = bl.create_hilbert_rtree(&tree);
	}
	else {
	    result = bl.create_rstartree(&tree);
	}

	if (result != ami::NO_ERROR) {
	    std::cerr << "Error " << std::hex << result;
	}

	std::cerr << "...done (" << tree->total_objects() << " objects)." << std::endl;
	tree->show_stats();

#ifdef COUNT_ALL_OBJECTS
	ami::rstarnode<double>*  n = NULL;
	std::list<ami::bid_t> l;
 	int nodes = 0;
 	int objects = 0;

 	l.push_back(tree->rootPosition());
	
 	while (!l.empty()) {
	    ami::bid_t next = l.front();
 	    l.pop_front();
 	    n = tree->readNode(next);
 	    ++nodes;
 	    if (!n->isLeaf()) {
 		for(ami::children_count_t i=0; i<n->numberOfChildren(); ++i) {
 		    l.push_back(n->getChild(i).get_id());		    
 		}
 	    }
 	    else {
 		objects += n->numberOfChildren();
 	    }
 	    delete n;
 	}
	std::cout << nodes << " nodes" << std::endl;
 	std::cout << objects << " objects" << std::endl;
#endif

	delete tree;
    }

    return 0;
}
