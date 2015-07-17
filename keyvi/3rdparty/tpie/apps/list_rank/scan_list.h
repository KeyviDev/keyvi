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
// File: scan_list.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//
// $Id: scan_list.h,v 1.2 2004-08-12 12:36:45 jan Exp $
//
// A scan management object that produces a linked list in order.
//
#ifndef _SCAN_LIST_H
#define _SCAN_LIST_H

#include "list_edge.h"

class scan_list : AMI_scan_object {
private:
    TPIE_OS_OFFSET maximum;
public:
    TPIE_OS_OFFSET last_to;
    TPIE_OS_OFFSET called;

    scan_list(TPIE_OS_OFFSET max = 1000);
    AMI_err initialize(void);
    AMI_err operate(edge *out1, AMI_SCAN_FLAG *sf);
};

#endif // _SCAN_LIST_H 
