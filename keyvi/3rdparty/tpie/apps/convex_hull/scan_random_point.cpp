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
// File: scan_random_point.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/19/94
//

// Get information on the configuration to test.
#include "app_config.h"

// Define it all.
#include <ami.h>

VERSION(scan_random_point_cpp,"$Id: scan_random_point.cpp,v 1.9 2004-08-12 12:36:24 jan Exp $");

#include "scan_random_point.h"

scan_random_point::scan_random_point(TPIE_OS_OFFSET count, int seed) 
{
    this->max = count;
    this->remaining = count;
    TP_LOG_APP_DEBUG("scan_random_point seed = ");
    TP_LOG_APP_DEBUG(seed);
    TP_LOG_APP_DEBUG('\n');

    TPIE_OS_SRANDOM(seed);
}

scan_random_point::~scan_random_point(void)
{
}


AMI_err scan_random_point::initialize(void)
{
    this->remaining = this->max;

    return AMI_ERROR_NO_ERROR;
};

AMI_err scan_random_point::operate(point<int> *out1, AMI_SCAN_FLAG *sf)
{
    if ((*sf = (remaining-- > 0))) {
        do {
        out1->x = TPIE_OS_RANDOM() & 0xFFFF;
        out1->y = TPIE_OS_RANDOM() & 0xFFFF;
        } while (((out1->x - 0x7FFF) * (out1->x - 0x7FFF) +
                  (out1->y - 0x7FFF) * (out1->y - 0x7FFF)) > 0x7000 * 0x7000);
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};
