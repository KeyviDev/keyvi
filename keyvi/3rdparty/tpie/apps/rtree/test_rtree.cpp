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

#include "app_config.h"

#include <tpie/portability.h>
#include <tpie/block.h>
#include "rstartree.h"

#include <iostream>
#include <list>

const unsigned short fanOut = 10;

using namespace tpie::ami;

void insertIntoTree(rstartree<double>* r, int numberOfObjects) {

    for(int i = 0; i < numberOfObjects; ++i) {
	if (i == 89) {
	    std::cerr << numberOfObjects << std::endl;
	}
 	r->insert(rectangle<double, bid_t>(i, i*5,i*5, i*5+2, i*5+2));
	std::cerr << i << " ";
    }

    r->show_stats();
    r->check_tree();
}

void deleteFromTree(rstartree<double>* r, int numberOfObjects) {

    for(int i = 0; i < numberOfObjects; ++i) {
 	r->remove(rectangle<double, bid_t>(i, i*5,i*5, i*5+2, i*5+2));
    }

    r->show_stats();
    r->check_tree();
}

void printTree(rstartree<double>* r) {
    
    rstarnode<double>* n=NULL;
    std::list<bid_t> l;
    
    l.push_back(r->root_position());
    
    while (!l.empty()) {

	bid_t next = l.front();
	l.pop_front();

	n = r->read_node(next);

	std::cout << "-------------------------------------------" << std::endl;
	std::cout << *n << std::endl;

	n->show_children();

	if (!n->is_leaf()) {
	    for(unsigned short i=0; i<n->children(); ++i) {
		l.push_back(n->get_child(i).get_id());
	    }
	}
	delete n;
    }
}

int main(int /* argc */, char** /* argv */) {

    rstartree<double>* r = new rstartree<double>("rectangles100.rtree", fanOut);        
    r->show_stats();
    r->check_tree();
    
    std::cerr << "inserting rectangles..." << std::endl;
    insertIntoTree(r, 100);
    std::cerr << "done..." << std::endl;

    r->show_stats();
    r->check_tree();
    printTree(r);
    delete r;

    return 0;
}
