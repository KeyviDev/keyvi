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

// Copyright (c) 1994 Darren Vengroff
//
// File: scan_list.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//


#include "app_config.h"
#include <ami.h>
#include "scan_list.h"

VERSION(scan_list_cpp,"$Id: scan_list.cpp,v 1.3 2004-08-12 12:36:45 jan Exp $");

// MODULUS and INCREMENT are used to compute (vaguely psuedo-random)
// node numbers for the edges of the list.  They should be relatively
// prime.  Modulus should be large, since it is what determines how
// many nodes we can go through without cycling.

#define MODULUS   (1073741824L)			// = 2^30
#define INCREMENT (1977326743L - MODULUS) 	// = 7^11 % MODULUS

// Special values that may appear in the to or from fields of nodes to
// indicate that that are the initial or final edges of the list.
// These are good for a quick consistency check after ranking the
// list.

#ifndef LIST_NODE_SPECIAL_VALUES 
#define LIST_NODE_SPECIAL_VALUES 0
#endif

#define LIST_NODE_ZERO 0
#define LIST_NODE_INFINITY MODULUS


scan_list::scan_list(TPIE_OS_OFFSET max) : maximum(max), called(0)
{
}

AMI_err scan_list::initialize(void)
{
    called = 0;
#if LIST_NODE_SPECIAL_VALUES    
    last_to = LIST_NODE_ZERO;
#else
    last_to = 17;
#endif    
    return AMI_ERROR_NO_ERROR;
};

AMI_err scan_list::operate(edge *out1, AMI_SCAN_FLAG *sf)
{
    called++;
    out1->from = last_to;
#if LIST_NODE_SPECIAL_VALUES        
    out1->to =  (called == maximum) ? LIST_NODE_INFINITY : 
        (last_to = (last_to + INCREMENT) % MODULUS);
#else
    out1->to = (last_to = (last_to + INCREMENT) % MODULUS);
#endif    
    out1->weight = 1;
    out1->flag = false;
    return (*sf = (called <= maximum)) ? AMI_SCAN_CONTINUE : AMI_SCAN_DONE;
};


