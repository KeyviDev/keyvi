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

// Copyright (c) 1995 Darren Vengroff
//
// File: scan_uniform_sm.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: scan_uniform_sm.h,v 1.7 2004-08-12 15:15:11 jan Exp $
//
#ifndef _TPIE_APPS_SCAN_UNIFORM_SM_H
#define _TPIE_APPS_SCAN_UNIFORM_SM_H

#include "sparse_matrix.h"

namespace tpie {

    namespace apps {
	
	class scan_uniform_sm : public ami::scan_object {
	private:
	    TPIE_OS_OFFSET r,c, rmax, cmax;
	    double d;

	public:
	    scan_uniform_sm(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols,
			    double density, int seed);
	    virtual ~scan_uniform_sm(void);

	    ami::err initialize(void);
	    ami::err operate(sm_elem<double> *out, ami::SCAN_FLAG *sf);
	};

    }  //  namespace apps

}  //  namespace tpie

#endif // _TPIE_APPS_SCAN_UNIFORM_SM_H 
