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
// File: ami_matrix_fill.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/12/94
//
// $Id: ami_matrix_fill.h,v 1.9 2005-11-17 17:11:25 jan Exp $
//
#ifndef _TPIE_APPS_MATRIX_FILL_H
#define _TPIE_APPS_MATRIX_FILL_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
// Get the AMI_scan_object definition.
#include <tpie/scan.h>

namespace tpie {
    
    namespace apps {
	
	template<class T>
	class matrix_filler {

	public:
	    virtual ami::err initialize(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols) = 0;
	    virtual T element(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col) = 0;
	    virtual ~matrix_filler() {};
	};

    }  //  namespace apps

}  //  namespace tpie

namespace tpie {

    namespace apps {
	
 
	template<class T>
	class matrix_fill_scan : ami::scan_object {

	private:
	    // Prohibit these
	    matrix_fill_scan(const matrix_fill_scan<T>& other);
	    matrix_fill_scan<T>& operator=(const matrix_fill_scan<T>& other);
	    
	    TPIE_OS_OFFSET r, c;
	    TPIE_OS_OFFSET cur_row, cur_col;
	    matrix_filler<T> *pemf;

	public:
	    matrix_fill_scan(matrix_filler<T> *pem_filler,
			     TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols) :
		r(rows), c(cols), 
		cur_row(0), cur_col(0),
		pemf(pem_filler) {
	    };
	    
	    ami::err initialize(void) {
		cur_row = cur_col = 0;
		return ami::NO_ERROR;
	    };

	    ami::err operate(T *out, ami::SCAN_FLAG *sf) {
		if ((*sf = (cur_row < r))) {
		    *out = pemf->element(cur_row,cur_col);
		    if ((cur_col = (cur_col+1) % c)==0) {
			cur_row++;
		    }
		    return ami::SCAN_CONTINUE;
		} else {
		    return ami::SCAN_DONE;
		}        
	    };
	};
    

	template<class T>
	ami::err matrix_fill(matrix<T> *pem, matrix_filler<T> *pemf) {
	    ami::err ae;
	    
	    ae = pemf->initialize(pem->rows(), pem->cols());
	    if (ae != ami::NO_ERROR) {
		return ae;
	    }
	    
	    matrix_fill_scan<T> emfs(pemf, pem->rows(), pem->cols());
	    
	    return ami::scan(&emfs, dynamic_cast<ami::stream<T>*>(pem));
	};

    }  //  namespace apps

}  //  namespace tpie 

#endif // _TPIE_APPS_MATRIX_FILL_H 
