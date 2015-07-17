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
// File: ami_matrix_pad.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//
// $Id: ami_matrix_pad.h,v 1.10 2005-11-17 17:11:25 jan Exp $
//
#ifndef _AMI_MATRIX_PAD_H
#define _AMI_MATRIX_PAD_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
// Get definition of AMI_scan_object class.
#include <tpie/scan.h>

// This is a scan management object designed to pad a rows by cols
// matrix with zeroes so that is becomes an (i * block_extent) by (j *
// block_extent) matrix where i and j are integers as small as
// possible.

namespace tpie {
    
    namespace apps {
	
	template<class T>
	class matrix_pad : ami::scan_object {
	private:
	    TPIE_OS_OFFSET cur_row, cur_col;
	    TPIE_OS_OFFSET orig_rows, orig_cols;
	    TPIE_OS_OFFSET final_rows, final_cols;
	public:
	    matrix_pad(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols,
		       TPIE_OS_OFFSET block_extent);
	    virtual ~matrix_pad();
	    ami::err initialize(void);
	    ami::err operate(const T &in, ami::SCAN_FLAG *sfin,
			     T *out, ami::SCAN_FLAG *sfout);
	};


	template<class T>
	matrix_pad<T>::matrix_pad(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols,
                                  TPIE_OS_OFFSET block_extent) :
	    cur_row(0), cur_col(0),
	    orig_rows(0), orig_cols(0),
	    final_rows(0), final_cols(0)
	    
	{
	    orig_rows = rows;
	    orig_cols = cols;
	    final_rows = block_extent * (((orig_rows - 1) / block_extent) + 1);
	    final_cols = block_extent * (((orig_cols - 1) / block_extent) + 1);
	}
	
	template<class T>
	matrix_pad<T>::~matrix_pad() {
	}
	
	template<class T>
	ami::err matrix_pad<T>::initialize(void) {
	    cur_col = cur_row = 0;
	    return ami::NO_ERROR;
	}
	
	template<class T>
	ami::err matrix_pad<T>::operate(const T &in, ami::SCAN_FLAG *sfin,
					T *out, ami::SCAN_FLAG *sfout)
	{
	    ami::err ae;
	    
	    // If we are within the bounds of the original matrix, simply copy.
	    if ((cur_col < orig_cols) && (cur_row < orig_rows)) {
		*out = in;
		*sfout = true;
		ae = ami::SCAN_CONTINUE;
	    } else {
		// Don't take the input.
		*sfin = false;
		// If we are not completely done then write padding.
		if ((*sfout = (cur_row < final_rows))) {
		    *out = 0;
		    ae = ami::SCAN_CONTINUE;
		} else {
		    tp_assert(cur_row == final_rows, "Too many rows.");
		    ae = ami::SCAN_DONE;
		}
	    }
	    
	    // Increment the column.
	    cur_col = (cur_col + 1) % final_cols;
	    
	    // Increment the row if needed.
	    if (!cur_col) {
		cur_row++;
	    }
	    
	    return ae;
	}
	
    }  //  namespace apps

}  //  namespace tpie


namespace tpie {

    namespace apps {
	
// This is a scan management object designed to unpad a rows by cols
// matrix that was padded by a an object of type scan_matrix_pad.
	
	template<class T>
	class matrix_unpad : ami::scan_object {
	private:
	    TPIE_OS_OFFSET cur_row, cur_col;
	    TPIE_OS_OFFSET orig_rows, orig_cols;
	    TPIE_OS_OFFSET final_rows, final_cols;
	public:
	    matrix_unpad(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols,
			 TPIE_OS_OFFSET block_extent);
	    virtual ~matrix_unpad();
	    ami::err initialize(void);
	    ami::err operate(const T &in, ami::SCAN_FLAG *sfin,
			     T *out, ami::SCAN_FLAG *sfout);
	};
	
	template<class T>
	matrix_unpad<T>::matrix_unpad(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols,
                                      TPIE_OS_OFFSET block_extent) :
	    cur_row(0), cur_col(0),
	    orig_rows(0), orig_cols(0),
	    final_rows(0), final_cols(0)
	    
	{
	    orig_rows = rows;
	    orig_cols = cols;
	    final_rows = block_extent * (((orig_rows - 1) / block_extent) + 1);
	    final_cols = block_extent * (((orig_cols - 1) / block_extent) + 1);
	}
	
	template<class T>
	matrix_unpad<T>::~matrix_unpad() {
	}
	
	template<class T>
	ami::err matrix_unpad<T>::initialize(void)
	{
	    cur_col = cur_row = 0;
	    return ami::NO_ERROR;
	}
	
	template<class T>
	ami::err matrix_unpad<T>::operate(const T &in, ami::SCAN_FLAG *sfin,
					  T *out, ami::SCAN_FLAG *sfout)
	{
	    ami::err ae;
	    
	    // If we are within the bounds of the original matrix, simply copy.
	    if ((cur_col < orig_cols) && (cur_row < orig_rows)) {
		*out = in;
		*sfout = true;
		ae = ami::SCAN_CONTINUE;
	    } else {
		// Don't write anything.
		*sfout = false;
		
		// If we are not completely done then skip padding.
		if ((*sfin = (cur_row < final_rows))) {
		    ae = ami::SCAN_CONTINUE;
		} else {
		    tp_assert(cur_row == final_rows, "Too many rows.");
		    ae = ami::SCAN_DONE;
		}
	    }
	    
	    // Increment the column.
	    cur_col = (cur_col + 1) % final_cols;
	    
	    // Increment the row if needed.
	    if (!cur_col) {
		cur_row++;
	    }
	    
	    return ae;        
	}
	
    }  //  namespace apps

}  //  namespace tpie

#endif // _AMI_MATRIX_PAD_H 
