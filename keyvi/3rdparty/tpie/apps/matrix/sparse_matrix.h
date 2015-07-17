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
#ifndef _TPIE_APPS_SPARSE_MATRIX_H
#define _TPIE_APPS_SPARSE_MATRIX_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <iostream>

// We need dense matrices to support some sparse/dense interactions.
#include "matrix.h"

#include "sm_elem.h"
#include "sm_band_comparator.h"

namespace tpie {

    namespace apps {

	template<class T>
	class sparse_matrix : public ami::stream< sm_elem<T> > {

	private:
	    // How many rows and columns.
	    TPIE_OS_OFFSET r,c;

	public:
	    sparse_matrix(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col);
	    ~sparse_matrix(void);
	    TPIE_OS_OFFSET rows();
	    TPIE_OS_OFFSET cols();
	};

	template<class T>
	sparse_matrix<T>::sparse_matrix(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col) :
	    ami::stream< sm_elem<T> >(), r(row), c(col) {
	}

	template<class T>
	sparse_matrix<T>::~sparse_matrix(void) {
	}
	
	template<class T>
	TPIE_OS_OFFSET sparse_matrix<T>::rows(void) {
	    return r;
	}

	template<class T>
	TPIE_OS_OFFSET sparse_matrix<T>::cols(void) {
	    return c;
	}


// A function to bandify a sparse matrix.
	
	template<class T>
	ami::err sparse_bandify(sparse_matrix<T> &sm,
				sparse_matrix<T> &bsm,
				TPIE_OS_SIZE_T rows_per_band) {
	    
	    ami::err ae = ami::NO_ERROR;
	    
	    sm_band_comparator<T> cmp(rows_per_band);    
	    
	    ae = ami::sort(&sm, &bsm, &cmp);	    
	    
	    return ae;
	}
	
// Get all band information for the given matrix and the current
// runtime environment.
	
	template<class T>
	ami::err sparse_band_info(sparse_matrix<T> &opm,
				  TPIE_OS_SIZE_T &rows_per_band,
				  TPIE_OS_OFFSET &total_bands) {
	    
	    TPIE_OS_SIZE_T sz_avail, single_stream_usage;
	    
	    TPIE_OS_OFFSET rows = opm.rows();
	    
	    ami::err ae = ami::NO_ERROR;
	    
	    // Check available main memory.
	    sz_avail = get_memory_manager().available ();
	    
	    // How much memory does a single stream need in the worst case?
    
	    if ((ae = opm.main_memory_usage(&single_stream_usage,
										STREAM_USAGE_MAXIMUM)) !=
		ami::NO_ERROR) {
		return ae;
	    }
	    
	    // Figure out how many elements of the output can fit in main
	    // memory at a time.  This will determine the number of rows of
	    // the sparse matrix that go into a band.
	    
	    rows_per_band = (sz_avail - single_stream_usage * 5) / sizeof(T);
	    
	    if (static_cast<TPIE_OS_OFFSET>(rows_per_band) > rows) {
		rows_per_band = static_cast<TPIE_OS_SIZE_T>(rows);
	    }
	    
	    total_bands = (rows + rows_per_band - 1) / rows_per_band;
	    
	    return ami::NO_ERROR;
	}

	
//
//
//
//
	
	template<class T>
	ami::err sparse_mult_scan_banded(sparse_matrix<T> &banded_opm,
					 matrix<T> &opv, matrix<T> &res,
					 TPIE_OS_OFFSET rows, TPIE_OS_OFFSET /*cols*/,
					 TPIE_OS_SIZE_T rows_per_band) {

	    ami::err ae = ami::NO_ERROR;
	    
	    sm_elem<T> *sparse_current;
	    T *vec_current;
	    TPIE_OS_OFFSET vec_row;
	    
	    banded_opm.seek(0);
	    ae = banded_opm.read_item(&sparse_current);
	    if (ae != ami::NO_ERROR) {
		return ae;
	    }
	    
	    opv.seek(0);
	    ae = opv.read_item(&vec_current);
	    if (ae != ami::NO_ERROR) {
		return ae;
	    }
	    vec_row = 0;
	    
	    res.seek(0);
	    
	    TPIE_OS_OFFSET next_band_start = rows_per_band;
	    
	    TPIE_OS_OFFSET ii;
	    
	    TPIE_OS_OFFSET curr_band_start = 0;
	    TPIE_OS_SIZE_T rows_in_current_band = rows_per_band;
	    
	    bool sparse_done = false;
	    
	    T *output_subvector = new T[rows_per_band];
	    
	    for (ii = rows_per_band; ii--; ) {
		output_subvector[ii] = 0;
	    }
	    
	    while (1) {
		//
		// Each time we enter this loop, we have the following invariants:
		//
		// 	vec_current = an element of the vector.
		//
		// 	vec_row = row vec_current came from.
		//
		//	sparse_current = current element from the banded sparse mat.
		//
		//	curr_band_start = row beginning current band.
		//
		// 	next_band_start = row beginning next band.
		//
		// 	rows_in_current_band = as name implies.
		//
		
		if (sparse_done || (sparse_current->er >= next_band_start)) {
		    
		    // If we are out of sparse elements or the sparse element
		    // row is in the next band then we have to write the
		    // current results, reset the output buffer, and rewind
		    // the vector.
		    
		    ae = res.write_array(output_subvector, rows_in_current_band);
		    if (ae != ami::NO_ERROR) {
			return ae;
		    }
		    
		    if (sparse_done) {
			// Write more zeores if necessary before breaking out
			// of the loop.  We have to do this in some cases if
			// there are one or more empty bands at the bottom of
			// the sparse matrix.  It is unlikely this sort of
			// thing would ever really happen, but we should be
			// careful anyway.
			T tmp = 0;
			for (ii = rows - next_band_start; ii--; ) {
			    ae = res.write_item(tmp);
			    if (ae != ami::NO_ERROR) {
				return ae;
			    }
			}
			break;
		    }
		    
		    for (ii = rows_in_current_band; ii--; ) {
			output_subvector[ii] = 0;
		    }
		    
		    opv.seek(0);
		    
		    curr_band_start = next_band_start;
		    
		    next_band_start += rows_per_band;
		    
		    if (next_band_start > rows) {
			// The final band may not have exactly rows_per_band
			// rows due to roundoff.  We make the appropropriate
			// adjustments here.
			rows_in_current_band = static_cast<TPIE_OS_SIZE_T>(rows - curr_band_start); 
			next_band_start = rows;                
		    } else {
			rows_in_current_band = rows_per_band;
		    }
		} else if (sparse_current->ec == vec_row) {
		    
		    // If the column of the sparse matrix and the row of the
		    // vector that the current inputs come from are the same,
		    // then multiply them, add the result to the appropriate
		    // output element, and advance past the sparse element.
		    
		    output_subvector[sparse_current->er - curr_band_start] +=
			sparse_current->val * *vec_current;
		    
		    ae = banded_opm.read_item(&sparse_current);
		    if (ae == ami::END_OF_STREAM) {
			sparse_done = true;
		    } else if (ae != ami::NO_ERROR) {
			return ae;
		    }
		    
		} else {
		    
		    // If the sparse element column is past the current
		    // row in the vector, then advance the vector.
		    
		    tp_assert(sparse_current->ec > vec_row,
			      "Sparse column fell behind current row.");
		    
		    ae = opv.read_item(&vec_current);
		    if (ae != ami::NO_ERROR) {
			return ae;
		    }
		    vec_row++;
		}
		
	    }
	    
	    delete [] output_subvector;
	    
	    return ami::NO_ERROR;
	}    
	
	
	
// Multiply a sparse (n,m)-matrix by a dense m-vector to get a dense
// n-vector.
	
	template<class T>
	ami::err sparse_mult(sparse_matrix<T> &opm, matrix<T> &opv,
			     matrix<T> &res) {
	    
	    TPIE_OS_SIZE_T rows_per_band;
	    TPIE_OS_OFFSET total_bands;
	    
	    TPIE_OS_OFFSET rows;
	    TPIE_OS_OFFSET cols;
	    
	    // size_t sz_avail, single_stream_usage;
	    
	    ami::err ae = ami::NO_ERROR;
	    
	    // Make sure the sizes of the matrix and vectors match up.
	    
	    rows = opm.rows();
	    cols = opm.cols();
	    
	    if ((cols != opv.rows()) || (rows != res.rows()) ||
		(opv.cols() != 1) || (res.cols() != 1)) {
		return ami::MATRIX_BOUNDS;
	    }
	    
	    // Get band information.
	    
	    ae = sparse_band_info(opm, rows_per_band, total_bands);
	    if (ae != ami::NO_ERROR) {
		return ae;
	    }
	    
	    // Partition the sparse matrix into bands with the elements within
	    // a band sorted by column.  This is all done by a single sort
	    // operation.
	    //
	    // Note that if our goal is to multiply a large number of
	    // different vectors by a single matrix we should seperate this
	    // step out into a preprocessing phacse so that the sort is only
	    // done once.
	    
	    sparse_matrix<T> banded_opm(rows, cols);
	    
	    ae = sparse_bandify(opm, banded_opm, rows_per_band);
	    if (ae != ami::NO_ERROR) {
		return ae;
	    }
	    
	    // Scan the contents of the bands and the vector to produce output.
	    
	    ae = sparse_mult_scan_banded(banded_opm, opv, res,
					 rows, cols, rows_per_band);
	    
	    return ae;
	}
	
    }  //  namespace apps

}  //  namespace tpie
	
#endif // _TPIE_APPS_SPARSE_MATRIX_H 
