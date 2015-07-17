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

// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_count.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// A scan object to generate a stream of intergers in ascending order.
//


// static char scan_count_id[] = "$Id: scan_count.cpp,v 1.5 2005-11-16 17:03:49 jan Exp $"; 

#include "app_config.h"
#include "scan_count.h"

ami::err scan_count::initialize(void) {

    called = 0;
    ii = 0;
    
    return ami::NO_ERROR;
};

scan_count::scan_count(TPIE_OS_OFFSET max) :
        maximum(max),
        ii(0) {

    //  No code in this constructor
};

ami::err scan_count::operate(TPIE_OS_OFFSET *out1, ami::SCAN_FLAG *sf) {
    called++;
    *out1 = ++ii;
    return (*sf = (ii <= maximum)) ? ami::SCAN_CONTINUE : ami::SCAN_DONE;
};

