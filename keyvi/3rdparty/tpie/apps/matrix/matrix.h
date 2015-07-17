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
// File: ami_matrix.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/9/94
//
// $Id: ami_matrix.h,v 1.15 2005-11-16 16:52:31 jan Exp $
//
#ifndef _TPIE_APPS_MATRIX_H
#define _TPIE_APPS_MATRIX_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

//#define QUICK_MATRIX_MULT 1
#define AGGARWAL_MATRIX_MULT 1

#define INTERNAL_TIMING 1

#ifdef INTERNAL_TIMING
#  include <tpie/cpu_timer.h>
#  include <iostream>
#endif

#include "mm_matrix.h"

#include "matrix_pad.h"
#include "matrix_blocks.h"
#include <tpie/stream_arith.h>

#include <tpie/gen_perm.h>

namespace tpie {
    
    namespace apps {

	template<class T>
	class matrix : public ami::stream<T> {

	private:
	    TPIE_OS_OFFSET r,c;
	public:
	    matrix(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col);
	    ~matrix(void);
	    TPIE_OS_OFFSET rows();
	    TPIE_OS_OFFSET cols();
	};
	
	template<class T>
	matrix<T>::matrix(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col) :
	    ami::stream<T>(), r(row), c(col)	{
	}
	
	template<class T>
	matrix<T>::~matrix(void) {
	}
	
	template<class T>
	TPIE_OS_OFFSET matrix<T>::rows(void) {
	    return r;
	}
	
	template<class T>
	TPIE_OS_OFFSET matrix<T>::cols(void) {
	    return c;
	}
	
        // Add two matrices.	
	template<class T>
	ami::err matrix_add(matrix<T> &op1, matrix<T> &op2,
                       matrix<T> &res) {
	    ami::scan_add<T> sa;
	    
	    // We should do some bound checking here.
	    
	    return scan(&op1, &op2, &sa, &res);
	}

        // Subtract.	
	template<class T>
	ami::err matrix_sub(matrix<T> &op1, matrix<T> &op2,
                       matrix<T> &res){
	    ami::scan_sub<T> ss;
	    
	    // We should do some bound checking here.
	    
	    return scan(&op1, &op2, &ss, &res);
	}


// Matrix multiply.
	
// For standard (non-Strassen) matrix multiply, there are at least two
// algorithms with the same asymptotic complexity.  There is the 4 way
// divide and conquer algorithms of Vitter and Shriver and there is
// the technique which divides the matrix into blocks of size
// $sqrt(M/B) \times sqrt(M/B)$.  The latter has a smaller constant and is
// simpler to implement, so we chose to use it.
	
	template<class T>
	ami::err matrix_mult(matrix<T> &op1, matrix<T> &op2,
                        matrix<T> &res)	{
	    ami::err ae = ami::NO_ERROR;
	    
	    TPIE_OS_SIZE_T sz_avail;
	    TPIE_OS_SIZE_T mm_matrix_extent;
	    TPIE_OS_SIZE_T single_stream_usage;    
	    
	    // Check bounds on the matrices to make sure they match up.
	    if ((op1.cols() != op2.rows()) || (res.rows() != op1.rows()) ||
		(res.cols() != op2.cols())) {
		return ami::MATRIX_BOUNDS;
	    }
	    
	    // Check available main memory.
	    sz_avail = get_memory_manager().available ();
	    
	    // How much memory does a single streamneed in the worst case?
	    
	    if ((ae = op1.main_memory_usage(&single_stream_usage,
					    STREAM_USAGE_MAXIMUM)) !=
		ami::NO_ERROR) {
		return ae;
	    }
	    
	    // Will the problem fit in main memory?
	    
	    {
		TPIE_OS_OFFSET sz_op1 = op1.rows() * op1.cols() * sizeof(T);
		TPIE_OS_OFFSET sz_op2 = op2.rows() * op2.cols() * sizeof(T);
		TPIE_OS_OFFSET sz_res = res.rows() * res.cols() * sizeof(T);
		
		if (sz_avail > sz_op1 + sz_op2 + sz_res + 3 * single_stream_usage +
		    3 * sizeof(matrix<T>)) {
		    
		    TPIE_OS_SSIZE_T ii,jj;
		    T *tmp_read;
		    
		    // Main memory copies of the matrices.
		    mm_matrix<T> mm_op1(static_cast<TPIE_OS_SIZE_T>(op1.rows()), 
				     static_cast<TPIE_OS_SIZE_T>(op1.cols()));
		    mm_matrix<T> mm_op2(static_cast<TPIE_OS_SIZE_T>(op2.rows()), 
				     static_cast<TPIE_OS_SIZE_T>(op2.cols()));
		    mm_matrix<T> mm_res(static_cast<TPIE_OS_SIZE_T>(res.rows()), 
				     static_cast<TPIE_OS_SIZE_T>(res.cols()));
		    
		    // Read in the matrices and solve in main memory.
		    
		    op1.seek(0);
		    for (ii = 0; ii < op1.rows(); ii++ ) {
			for (jj = 0; jj < op1.cols(); jj++ ) {
			    ae = op1.read_item(&tmp_read);
			    if (ae != ami::NO_ERROR) {
				return ae;
			    }
			    mm_op1[ii][jj] = *tmp_read;
			}
		    }
		    
		    op2.seek(0);
		    for (ii = 0; ii < op2.rows(); ii++ ) {
			for (jj = 0; jj < op2.cols(); jj++ ) {
			    ae = op2.read_item(&tmp_read);
			    if (ae != ami::NO_ERROR) {
				return ae;
			    }
			    mm_op2[ii][jj] = *tmp_read;
			}
		    }
		    
#if QUICK_MATRIX_MULT
		    quick_matrix_mult_in_place(mm_op1, mm_op2, mm_res);
#elif defined(AGGARWAL_MATRIX_MULT)
		    aggarwal_matrix_mult_in_place(mm_op1, mm_op2, mm_res);
#else            
		    perform_mult_in_place((matrix_base<T> &)mm_op1,
					  (matrix_base<T> &)mm_op2,
					  (matrix_base<T> &)mm_res);
#endif
		    
		    // Write out the result.
		    res.seek(0);
		    for (ii = 0; ii < res.rows(); ii++ ) {
			for (jj = 0; jj < res.cols(); jj++ ) {
			    ae = res.write_item(mm_res[ii][jj]);
			    if (ae != ami::NO_ERROR) {
				return ae;
			    }
			}
		    }
		    
		    return ami::NO_ERROR;
		}
		
	    }
	    
	    // We now know the problem does not fit in main memory.
	    
	    {
		
		TPIE_OS_SIZE_T num_active_streams = 4 + 4;
		TPIE_OS_SIZE_T mm_matrix_space;
		TPIE_OS_SIZE_T single_stream_usage;
		
		// What is the maximum extent of any matrix we will try to
		// load into memory?  We may have up to four in memory at any
		// given time.  To be safe, let each one have a stream behind
		// it and let there be some additional active streams such as
		// ....
		
		if ((ae = op1.main_memory_usage(&single_stream_usage,
										STREAM_USAGE_MAXIMUM)) !=
		    ami::NO_ERROR) {
		    return ae;
		}
		
		mm_matrix_space = sz_avail - num_active_streams * single_stream_usage;
		
		mm_matrix_space /= 3;
		
#ifdef AGGARWAL_MATRIX_MULT_IN_PLACE
		// Recall that a temporary vector is used, so we solve x^2 + x = m
		// for x, instead of the usual x^2 = m.
		mm_matrix_extent = static_cast<TPIE_OS_SIZE_T>(sqrt(1.0 +
								    4 * static_cast<double>(mm_matrix_space) /
								    sizeof(T)) / 2) - 1;
#else        
		mm_matrix_extent = static_cast<TPIE_OS_SIZE_T>(sqrt(static_cast<double>(mm_matrix_space) /
								    sizeof(T)));    
#endif
		// How many rows and columns of chunks in each matrix?
		
		TPIE_OS_OFFSET chunkrows1 = ((op1.rows() - 1) /
					     mm_matrix_extent) + 1;
		TPIE_OS_OFFSET chunkcols1 = ((op1.cols() - 1) /
					     mm_matrix_extent) + 1;
		TPIE_OS_OFFSET chunkrows2 = ((op2.rows() - 1) /
					     mm_matrix_extent) + 1;
		TPIE_OS_OFFSET chunkcols2 = ((op2.cols() - 1) /
					     mm_matrix_extent) + 1;
		
		// Now shrink the main memory matrix extent as much as possible
		// given the constraint that the number of chunk rows and cols
		// in each matrix cannot decrease.
		
		TPIE_OS_SIZE_T min_rows_per_chunk1 = static_cast<TPIE_OS_SIZE_T>((op1.rows() + chunkrows1 - 1) /
										 chunkrows1);
		TPIE_OS_SIZE_T min_cols_per_chunk1 = static_cast<TPIE_OS_SIZE_T>((op1.cols() + chunkcols1 - 1) /
										 chunkcols1);
		
		TPIE_OS_SIZE_T min_rows_per_chunk2 = static_cast<TPIE_OS_SIZE_T>((op2.rows() + chunkrows2 - 1) /
										 chunkrows2);
		TPIE_OS_SIZE_T min_cols_per_chunk2 = static_cast<TPIE_OS_SIZE_T>((op2.cols() + chunkcols2 - 1) /
										 chunkcols2);
		
		// Adjust the main memory matrix extent so that an integral
		// multiple of it is just a little bit larger than the inputs.
		
		// Note that we are still assuming square chunks.  We can do
		// better than this in some cases if we are willing to allow
		// non-square matrices.
		
		mm_matrix_extent = min_rows_per_chunk1;
		if (mm_matrix_extent < min_rows_per_chunk2)
		    mm_matrix_extent = min_rows_per_chunk2; 
		if (mm_matrix_extent < min_cols_per_chunk1)
		    mm_matrix_extent = min_cols_per_chunk1; 
		if (mm_matrix_extent < min_cols_per_chunk2)
		    mm_matrix_extent = min_cols_per_chunk2; 
		
		// How many rows and cols in padded matrices.
		
		TPIE_OS_OFFSET rowsp1 = mm_matrix_extent * (((op1.rows() - 1) /
							     mm_matrix_extent) + 1);
		TPIE_OS_OFFSET colsp1 = mm_matrix_extent * (((op1.cols() - 1) /
							     mm_matrix_extent) + 1);
		
		TPIE_OS_OFFSET rowsp2 = mm_matrix_extent * (((op2.rows() - 1) /
							     mm_matrix_extent) + 1);
		TPIE_OS_OFFSET colsp2 = mm_matrix_extent * (((op2.cols() - 1) /
							     mm_matrix_extent) + 1);
		
		
		// Padded matrices.
		
		matrix<T> *op1p = new matrix<T>(rowsp1, colsp1);
		matrix<T> *op2p = new matrix<T>(rowsp2, colsp2);
		
		// Scan each matrix to pad it out with zeroes as needed.
		
		{
		    matrix_pad<T> smp1(op1.rows(), op1.cols(), mm_matrix_extent);
		    matrix_pad<T> smp2(op2.rows(), op2.cols(), mm_matrix_extent);
		    
		    ae = scan(&op1, &smp1,op1p);
		    if (ae != ami::NO_ERROR) {
			return ae;
		    }
		    ae = scan(&op2, &smp2, op2p);
		    if (ae != ami::NO_ERROR) {
			return ae;
		    }
		}
		
		// Permuted padded matrices.
		
		matrix<T> *op1pp = new matrix<T>(rowsp1, colsp1);
		matrix<T> *op2pp = new matrix<T>(rowsp2, colsp2);
		
		matrix<T> *respp = new matrix<T>(rowsp1, colsp2);
		
		// Permute each padded matrix into block order.  The blocks
		// are in row major order and the elements within the blocks
		// are in row major order.
		
		{
		    perm_matrix_into_blocks pmib1(rowsp1, colsp1, mm_matrix_extent);
		    perm_matrix_into_blocks pmib2(rowsp2, colsp2, mm_matrix_extent);
		    
		    ae = general_permute(op1p, op1pp, &pmib1); 
		    if (ae != ami::NO_ERROR) {
			return ae;
		    }
		    
		    ae = general_permute(op2p, op2pp, &pmib2); 
		    if (ae != ami::NO_ERROR) {
			return ae;
		    }
		}
		
		// We are done with the padded but unpermuted matrices.
		
		delete op1p;
		delete op2p;
		
#ifdef INTERNAL_TIMING
		
		cpu_timer cput_internal;
		
		cput_internal.reset();
		cput_internal.start();
		
#endif
		
		// Now run the standard matrix multiplication algorithm over
		// the blocks.  To multiply two blocks, we read them into main
		// memory.  The blocks of the result are accumulated one by
		// one in main memory and then written out.
		
		{
		    
		    //Sometimes the mm_matrix_extent value is such that 
		    //it is too large for the available amount of memory
		    //to permit the allocation for the matrices below. I suspect 
		    //that there is a bug in the way mm_matrix_extent is assigned
		    //a value; sz_avail needs to be correctly taken into account.
		    //But the bug doesn't always take place. Need looking at.
		    //--Rakesh on mm_matrix_extent.
		    
		    mm_matrix<T> mm_op1(mm_matrix_extent, mm_matrix_extent);
		    mm_matrix<T> mm_op2(mm_matrix_extent, mm_matrix_extent);
		    mm_matrix<T> mm_accum(mm_matrix_extent, mm_matrix_extent);
		    
		    TPIE_OS_OFFSET ii,jj,kk;
		    T *tmp_read;
		    
		    respp->seek(0);
		    
		    // ii loops over block rows of op1pp.
		    
		    for (ii = 0; ii < TPIE_OS_OFFSET(rowsp1 / mm_matrix_extent); ii++ ) {
			
			// jj loops over block cols of op2pp.
			
			for (jj = 0; jj < TPIE_OS_OFFSET(colsp2 / mm_matrix_extent); jj++ ) {
			    
			    // These are for looping over rows and cols of MM
			    // matrices.
			    TPIE_OS_SIZE_T ii1,jj1;
			    
			    // Clear the temporary result.
			    for (ii1 = 0; ii1 < mm_matrix_extent; ii1++ ) {
				for (jj1 = 0; jj1 < mm_matrix_extent; jj1++ ) {
				    mm_accum[ii1][jj1] = 0;
				}
			    }
			    
			    // kk loops over the cols of op1pp and rows of
			    // op2pp at the same time.
			    
			    tp_assert(rowsp2 == colsp1, "Matrix extent mismatch.");
			    
			    for (kk = 0; kk < TPIE_OS_OFFSET(rowsp2 / mm_matrix_extent); kk++ ) {
				
				// Read a block from op1pp.

				op1pp->seek(ii * colsp1 * mm_matrix_extent +
					    kk * mm_matrix_extent * mm_matrix_extent);
				
				for (ii1 = 0; ii1 < mm_matrix_extent; ii1++ ) {
				    for (jj1 = 0; jj1 < mm_matrix_extent; jj1++ ) {
					ae = op1pp->read_item(&tmp_read);
					if (ae != ami::NO_ERROR) {
					    return ae;
					}
					mm_op1[ii1][jj1] = *tmp_read; 
				    }
				}
				
				// Read a block from op2pp.
				
				op2pp->seek(kk * colsp2 * mm_matrix_extent +
					    jj * mm_matrix_extent * mm_matrix_extent);
				
				for (ii1 = 0; ii1 < mm_matrix_extent; ii1++ ) {
				    for (jj1 = 0; jj1 < mm_matrix_extent; jj1++ ) {
					ae = op2pp->read_item(&tmp_read);
					if (ae != ami::NO_ERROR) {
					    return ae;
					}
					mm_op2[ii1][jj1] = *tmp_read; 
				    }
				}
				
				// Multiply in MM and add to the running sum.
				
#if QUICK_MATRIX_MULT
				quick_matrix_mult_add_in_place(mm_op1, mm_op2,
							       mm_accum);
#elif defined(AGGARWAL_MATRIX_MULT)
				aggarwal_matrix_mult_add_in_place(mm_op1, mm_op2,
								  mm_accum);
#else                        
				perform_mult_add_in_place((matrix_base<T> &)mm_op1,
							  (matrix_base<T> &)mm_op2,
							  (matrix_base<T> &)mm_accum);
#endif                    
			    }
			    
			    // We now have the complete result for a block of
			    // respp, so write it out.
			    
			    for (ii1 = 0; ii1 < mm_matrix_extent; ii1++ ) {
				for (jj1 = 0; jj1 < mm_matrix_extent; jj1++ ) {
				    ae = respp->write_item(mm_accum[ii1][jj1]);
				    if (ae != ami::NO_ERROR) {
					return ae;
				    }
				}
			    }                                        
			}
		    }            
		}
		
#ifdef INTERNAL_TIMING
		
		cput_internal.stop();
		std::cout << cput_internal << ' ';
		
#endif        
		
		// We are done with the padded and permuted operators.
		
		delete op1pp;
		delete op2pp;
		
		// Permute the result from scan block order back into row
		// major order.
		
		matrix<T> *resp = new matrix<T>(rowsp1, colsp2);
		
		{
		    perm_matrix_outof_blocks pmob(rowsp1, colsp1, mm_matrix_extent);
		    
		    ae = general_permute(respp, resp, &pmob); 
		    if (ae != ami::NO_ERROR) {
			return ae;
		    }            
		}
		
		// We are done with the padded and permuted result.
		
		delete respp;
		
		// Scan to strip the padding from the output matrix.
		
		{
		    matrix_unpad<T> smup(op1.rows(), op2.cols(),
					 mm_matrix_extent);
		    
		    ae = scan(resp, &smup, &res);
		    if (ae != ami::NO_ERROR) {
			return ae;
		    }
		    
		}
		
		// We are done with the padded but unpermuted result.
		
		delete resp;
		
	    }
	    
	    return ami::NO_ERROR;
	}
	
    }  //  namespace apps

}  // namespace tpie

#endif // _TPIE_APPS_MATRIX_H 
