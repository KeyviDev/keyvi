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
// File: scan_random_point.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/19/94
//
// $Id: scan_random_point.h,v 1.6 2004-08-12 12:36:24 jan Exp $
//
#ifndef _SCAN_RANDOM_POINT_H
#define _SCAN_RANDOM_POINT_H

#include "point.h"

// A scan object to generate random integers.
class scan_random_point : AMI_scan_object {
private:
    TPIE_OS_OFFSET max, remaining;
public:
    scan_random_point(TPIE_OS_OFFSET count = 1000, int seed = 17);
    virtual ~scan_random_point(void);
    AMI_err initialize(void);
    AMI_err operate(point<int> *out1, AMI_SCAN_FLAG *sf);
};

#endif // _SCAN_RANDOM_POINT_H 
