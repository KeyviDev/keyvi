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
// File: matrix.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//
// $Id: matrix.h,v 1.12 2005-11-17 17:11:25 jan Exp $
//
#ifndef _TPIE_APPS_MM_MATRIX_H
#define _TPIE_APPS_MM_MATRIX_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <iostream>

#include <tpie/tpie_assert.h>

#include "mm_matrix_base.h"
#include "mm_submatrix.h"

namespace tpie {

    namespace apps { 

	template<class T> class mm_matrix;

	template<class T>
	mm_matrix<T> operator+(const mm_matrix_base<T> &op1,
			       const mm_matrix_base<T> &op2)
	{
	    if ((op1.rows() != op2.rows()) || (op1.cols() != op2.cols())) {
		tp_assert(0, "Range error.");
	    }	    
	    
	    mm_matrix<T> temp(op1);
	    
	    return temp += op2;
	}
	
	template<class T>
	mm_matrix<T> operator*(const mm_matrix_base<T> &op1,
			       const mm_matrix_base<T> &op2)
	{
	    if (op1.cols() != op2.rows()) {
		tp_assert(0, "Range error.");
	    }
	    
	    mm_matrix<T> temp(op1.rows(),op2.cols());
	    
	    perform_mult_in_place(op1, op2, temp);
	    
	    return temp;
	}
	
// The matrix class itself.
	template<class T>
	class mm_matrix : public mm_matrix_base<T> {
	private:
	    using mm_matrix_base<T>::r;
	    using mm_matrix_base<T>::c;
  
	    T *data;
	public:
	    using mm_matrix_base<T>::rows;
	    using mm_matrix_base<T>::cols;

	    // Construction/destruction.
	    mm_matrix(TPIE_OS_SIZE_T arows, TPIE_OS_SIZE_T acols);
	    mm_matrix(const mm_matrix<T> &rhs);
	    mm_matrix(const mm_matrix_base<T> &rhs);
	    mm_matrix(const mm_submatrix<T> &rhs);
	    mm_matrix(const mm_rowref<T> umrr);
	    mm_matrix(const mm_colref<T> umcr);

	    virtual ~mm_matrix(void);

	    // We need an assignement operator that copies data by explicitly
	    // calling the base class's assignment operator to do elementwise
	    // copying.  Otherwise, the data pointer is just copied.
	    mm_matrix<T> &operator=(const mm_matrix<T> &rhs);
    
	    // We also want to be able to assign from submatrices.
	    mm_matrix<T> &operator=(const mm_submatrix<T> &rhs);
    
	    // Access to elements.
	    T &elt(TPIE_OS_SIZE_T row, TPIE_OS_SIZE_T col) const;

	};


	template<class T>
	mm_matrix<T>::mm_matrix(TPIE_OS_SIZE_T arows, TPIE_OS_SIZE_T acols) :
	    mm_matrix_base<T>(arows, acols), data(NULL)	{

	    data = new T[arows * acols];

	    // Initialize the contents of the mm_matrix.
	    memset(data, 0, arows * acols * sizeof(T));
	}

	template<class T>
	mm_matrix<T>::mm_matrix(const mm_matrix<T> &rhs) :
	    mm_matrix_base<T>(rhs.rows(), rhs.cols()), data(NULL) {

	    TPIE_OS_SIZE_T ii;
    
	    data = new T[r*c];

	    for (ii = r*c; ii--; ) {
		data[ii] = rhs.data[ii];
	    }
	}

	template<class T>
	mm_matrix<T>::mm_matrix(const mm_matrix_base<T> &rhs) :
	    mm_matrix_base<T>(rhs.rows(), rhs.cols()), data(NULL) {

	    TPIE_OS_SIZE_T ii,jj;
    
	    data = new T[r*c];

	    for (ii = r; ii--; ) {
		for (jj = c; jj--; ) {
		    data[c*ii+jj] = ((mm_matrix_base<T> &)rhs).elt(ii,jj);
		}
	    }
	}

	template<class T>
	mm_matrix<T>::mm_matrix(const mm_submatrix<T> &rhs) :
	    mm_matrix_base<T>(rhs.rows(), rhs.cols()), data(NULL) {

	    TPIE_OS_SIZE_T ii,jj;
    
	    data = new T[r*c];

	    for (ii = r; ii--; ) {
		for (jj = c; jj--; ) {
		    data[c*ii+jj] = ((mm_submatrix<T> &)rhs).elt(ii,jj);
		}
	    }
	}

	template<class T>
	mm_matrix<T>::mm_matrix(const mm_rowref<T> umrr) :
	    mm_matrix_base<T>(1, umrr.m.cols()), data(NULL) {

	    data = new T[c];

	    mm_matrix_base<T>::operator=(umrr);
	}

	template<class T>
	mm_matrix<T>::mm_matrix(const mm_colref<T> umcr) :
	    mm_matrix_base<T>(umcr.m.rows(),1), data(NULL) {

	    data = new T[r];

	    mm_matrix_base<T>::operator=(umcr);
	}

	template<class T>
	mm_matrix<T>::~mm_matrix(void) {
	    delete[] data;
	}


	template<class T>
	mm_matrix<T> &mm_matrix<T>::operator=(const mm_matrix<T> &rhs) {

	    // Call the assignment operator from the base class to do range
	    // checking and elementwise assignment.
	    (mm_matrix_base<T> &)(*this) = (mm_matrix_base<T> &)rhs;
    
	    return *this;
	}

	template<class T>
	mm_matrix<T> &mm_matrix<T>::operator=(const mm_submatrix<T> &rhs) {

	    // Call the assignment operator from the base class to do range
	    // checking and elementwise assignment.
	    (mm_matrix_base<T> &)(*this) = (mm_matrix_base<T> &)rhs;
    
	    return *this;
	}


	template<class T>
	T& mm_matrix<T>::elt(TPIE_OS_SIZE_T row, TPIE_OS_SIZE_T col) const {

	    if ((row >= rows()) || (col >= cols())) {
		tp_assert(0, "Range error.");
	    }

	    return data[row*cols()+col];
	}


// These are needed since template functions accept only exact argument
// type matches.  Base class promotion is not done as it is for
// ordinary functions.

#define MAT_DUMMY_OP(TM1,TM2,OP)		\
	template<class T>			\
	mm_matrix<T> operator OP (const TM1 &op1,	\
			       const TM2 &op2)	\
	{					\
	    return ((mm_matrix_base<T> &)op1) OP	\
		((mm_matrix_base<T> &)op2);	\
	}

	MAT_DUMMY_OP(mm_matrix<T>,mm_matrix<T>,+)
	    MAT_DUMMY_OP(mm_matrix<T>,mm_submatrix<T>,+)
	    MAT_DUMMY_OP(mm_submatrix<T>,mm_matrix<T>,+)
	    MAT_DUMMY_OP(mm_submatrix<T>,mm_submatrix<T>,+)
    
	    MAT_DUMMY_OP(mm_matrix<T>,mm_matrix<T>,*)
	    MAT_DUMMY_OP(mm_matrix<T>,mm_submatrix<T>,*)
	    MAT_DUMMY_OP(mm_submatrix<T>,mm_matrix<T>,*)
	    MAT_DUMMY_OP(mm_submatrix<T>,mm_submatrix<T>,*)
    
	    template<class T>
	std::ostream &operator<<(std::ostream &s, const mm_matrix<T> &m)
	{
	    return s << (mm_matrix_base<T> &)m;
	}

	template<class T>
	std::ostream &operator<<(std::ostream &s, const mm_submatrix<T> &m)
	{
	    return s << (mm_matrix_base<T> &)m;
	}



// Speedups for multiplying matrices.  This is only for use with the
// specific implementation of matrices above.  General purpose
// multiplication still has to be done with perform_mult_in_place or
// perform_mult_add_in_place.

	template<class T>
	void quick_matrix_mult_in_place(const mm_matrix<T> &op1,
					const mm_matrix<T> &op2,
					mm_matrix<T> &res)
	{
	    if ((op1.cols() != op2.rows()) ||
		(op1.rows() != res.rows()) ||
		(op2.cols() != res.cols())) {
#if HANDLE_EXCEPTIONS        
		throw mm_matrix_base<T>::range();
#else
		tp_assert(0, "Range error.");
#endif
	    }

	    TPIE_OS_SIZE_T ii,jj,kk;
	    TPIE_OS_SIZE_T r1,r2,c1,c2/*,cres*/;
	    T t;

	    r1 = op1.rows();
	    r2 = op2.rows();
	    c1 = op1.cols();
	    c2 = op2.cols();
	    /*cres = */res.cols();
    
	    // Iterate over rows of op1.
	    for (ii = r1; ii--; ) {
		// Iterate over colums of op2.
		for (jj = c2; jj--; ) {
		    // Iterate through the row of r1 and the column of r2.
//            t = op1.data[ii*c1+c1-1] * op2.data[(r2-1)*c2+jj];
//                // op1.elt(ii,op1.cols()-1) * op2.elt(op2.rows()-1,jj);
		    t = op1.elt(ii,c1-1) * op2.elt(r2-1,jj);
		    for (kk = r2 - 1; kk--; ) {                
//                t += op1.data[ii*c1+kk] * op2.data[kk*c2+jj];
//                    // op1.elt(ii,kk) * op2.elt(kk,jj);
			t += op1.elt(ii,kk) * op2.elt(kk,jj);
		    }
		    // Assign into the result.
//            res.data[ii*cres+jj] = t;
		    res.elt(ii,jj) = t;
		}
	    }    
	}                      


	template<class T>
	void quick_matrix_mult_add_in_place(const mm_matrix<T> &op1,
					    const mm_matrix<T> &op2,
					    mm_matrix<T> &res)
	{
	    if ((op1.cols() != op2.rows()) ||
		(op1.rows() != res.rows()) ||
		(op2.cols() != res.cols())) {
		tp_assert(0, "Range error.");
	    }

	    TPIE_OS_SIZE_T ii,jj,kk;
	    TPIE_OS_SIZE_T r1,r2,c1,c2/*,cres*/;
	    T t;

	    r1 = op1.rows();
	    r2 = op2.rows();
	    c1 = op1.cols();
	    c2 = op2.cols();
	    /*cres = */res.cols();
    
	    // Iterate over rows of op1.
	    for (ii = r1; ii--; ) {
		// Iterate over colums of op2.
		for (jj = c2; jj--; ) {
		    // Iterate through the row of r1 and the column of r2.
//            t = op1.data[ii*c1+c1-1] * op2.data[(r2-1)*c2+jj];
//                // op1.elt(ii,op1.cols()-1) * op2.elt(op2.rows()-1,jj);
		    t = op1.elt(ii,c1-1) * op2.elt(r2-1,jj);
		    for (kk = r2 - 1; kk--; ) {                
//                t += op1.data[ii*c1+kk] * op2.data[kk*c2+jj];
			t += op1.elt(ii,kk) * op2.elt(kk,jj);
		    }
		    // Assign into the result.
//            res.data[ii*cres+jj] += t;
		    res.elt(ii,jj) += t;
		}
	    }    
	}                      


// Aggarwal et. al.'s algorithm.

	template<class T>
	void aggarwal_matrix_mult_in_place(const mm_matrix<T> &op1,
					   const mm_matrix<T> &op2,
					   mm_matrix<T> &res)
	{
	    if ((op1.cols() != op2.rows()) ||
		(op1.rows() != res.rows()) ||
		(op2.cols() != res.cols())) {
		tp_assert(0, "Range error.");
	    }

	    TPIE_OS_SIZE_T ii,jj,kk;
	    TPIE_OS_SIZE_T r1,r2/*,c1*/,c2/*,cres*/;

	    r1 = op1.rows();
	    r2 = op2.rows();
	    /*c1 = */op1.cols();
	    c2 = op2.cols();
	    /*cres = */res.cols();

	    // Temporary results.

	    T *temp = new T[c2];
	    T op1elt;
    
	    // Iterate over rows of op1.
	    for (ii = r1; ii--; ) {

		// Clear out the temporary sums.
		for (jj = c2; jj--; ) {                
		    temp[jj] = 0;
		}        
        
		// Iterate through the row of r1 and the column of r2.
		for (kk = r2; kk--; ) {                
            
		    // Iterate over columns of op2.
//            op1elt = op1.data[ii*c1+kk];
		    op1elt = op1.elt(ii,kk);
		    for (jj = c2; jj--; ) {                
//                temp[jj] += op1elt * op2.data[kk*c2+jj];
			temp[jj] += op1elt * op2.elt(kk,jj);
		    }
		}

		// Set the results.
		for (jj = c2; jj--; ) {
//            res.data[ii*cres+jj] = temp[jj];
		    res.elt(ii,jj) = temp[jj];
		}
	    }

	    delete [] temp;
	}


	template<class T>
	void aggarwal_matrix_mult_add_in_place(const mm_matrix<T> &op1,
					       const mm_matrix<T> &op2,
					       mm_matrix<T> &res)
	{
	    if ((op1.cols() != op2.rows()) ||
		(op1.rows() != res.rows()) ||
		(op2.cols() != res.cols())) {
		tp_assert(0, "Range error.");
	    }

	    TPIE_OS_SIZE_T ii,jj,kk;
	    TPIE_OS_SIZE_T r1,r2/*,c1*/,c2/*,cres*/;

	    r1 = op1.rows();
	    r2 = op2.rows();
	    /*c1 = */op1.cols();
	    c2 = op2.cols();
	    /*cres = */res.cols();

	    // Temporary results.

	    T *temp = new T[c2];
	    T op1elt;
    
	    // Iterate over rows of op1.
	    for (ii = r1; ii--; ) {

		// Clear out the temporary sums.
		for (jj = c2; jj--; ) {                
		    temp[jj] = 0;
		}        
		// Iterate through the row of r1 and the column of r2.
		for (kk = r2; kk--; ) {                
            
		    // Iterate over columns of op2.
//            op1elt = op1.data[ii*c1+kk];
		    op1elt = op1.elt(ii,kk);
		    for (jj = c2; jj--; ) {                
//                temp[jj] += op1elt * op2.data[kk*c2+jj];
			temp[jj] += op1elt * op2.elt(kk,jj);
		    }
		}

		// Set the results.
		for (jj = c2; jj--; ) {
//            res.data[ii*cres+jj] += temp[jj];
		    res.elt(ii,jj) += temp[jj];
		}
	    }

	    delete [] temp;

	}

    }  //  namespace apps

} // namespace tpie

#endif // _TPIE_APPS_MM_MATRIX_H 
