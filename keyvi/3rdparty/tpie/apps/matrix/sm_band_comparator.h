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
// File: ami_sparse_matrix.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/2/95
//
// $Id: ami_sparse_matrix.h,v 1.16 2005-11-16 16:53:50 jan Exp $
//
#ifndef _TPIE_APPS_SM_BANDCOMPARATOR_H
#define _TPIE_APPS_SM_CANDCOMPARATOR_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {

    namespace apps {

//
// A class of comparison object designed to facilitate sorting of
// elements of the sparse matrix into bands.
//
	
	template<class T>
	class sm_band_comparator {
	private:
	    TPIE_OS_SIZE_T rpb;
	    
	public:
	    sm_band_comparator(TPIE_OS_SIZE_T rows_per_band) :
		rpb(rows_per_band) {};
	    
	    virtual ~sm_band_comparator(void) {};

	    // If they are in the same band, compare columns, otherwise,
	    // compare rows.
	    int compare(const sm_elem<T> &t1, const sm_elem<T> &t2) {
		if ((t1.er / rpb) == (t2.er / rpb)) {
		    return int(t1.ec) - int(t2.ec);
		} else {
		    return int(t1.er) - int(t2.er);
		}
	    }
	};
	
    }  //  namespace apps

}  // namespace tpie

#endif 

