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
#ifndef _TPIE_APPS_MM_MATRIX_BASE_H
#define _TPIE_APPS_MM_MATRIX_BASE_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <iostream>

#include <tpie/tpie_assert.h>

#include "mm_rowref.h"
#include "mm_colref.h"

namespace tpie {

    namespace apps { 
	
// A base class for matrices and submatrices.
	template<class T> class mm_matrix_base {

	protected:
	    TPIE_OS_SIZE_T r,c;
	    
	public:

	    mm_matrix_base(TPIE_OS_SIZE_T rows, TPIE_OS_SIZE_T cols);
	    virtual ~mm_matrix_base(void);
	    
	    // What is the size of the matrix?
	    TPIE_OS_SIZE_T rows(void) const;
	    TPIE_OS_SIZE_T cols(void) const;
	    
	    // Access to the contents of the matrix.
	    virtual T &elt(TPIE_OS_SIZE_T row, TPIE_OS_SIZE_T col) const  = 0;
	    
	    mm_rowref<T> row(TPIE_OS_SIZE_T row) ;
	    mm_colref<T> col(TPIE_OS_SIZE_T col) ;
	    
	    mm_rowref<T> operator[](TPIE_OS_SIZE_T row) ;
	    
	    // Assignement.
	    mm_matrix_base<T> &operator=(const mm_matrix_base<T> &rhs);
	    mm_matrix_base<T> &operator=(const mm_rowref<T> &rhs);
	    mm_matrix_base<T> &operator=(const mm_colref<T> &rhs);
	    
	    // Addition in place.
	    mm_matrix_base<T> &operator+=(const mm_matrix_base<T> &rhs);
	};
	
	
	template<class T>
	mm_matrix_base<T>::mm_matrix_base(TPIE_OS_SIZE_T rows, TPIE_OS_SIZE_T cols) :
	    r(rows),
	    c(cols) {
	}
	
	template<class T>
	mm_matrix_base<T>::~mm_matrix_base(void) {
	}
	
	template<class T>
	TPIE_OS_SIZE_T mm_matrix_base<T>::rows(void) const {
	    return r;
	}

	template<class T>
	TPIE_OS_SIZE_T mm_matrix_base<T>::cols(void) const {
	    return c;
	}
	
	template<class T>
	mm_rowref<T> mm_matrix_base<T>::row(TPIE_OS_SIZE_T row) {
	    if (row >= r) {
		tp_assert(0, "Range error.");
	    }
	    	    
	    return mm_rowref<T>(*this, row);
	}

	template<class T>
	mm_colref<T> mm_matrix_base<T>::col(TPIE_OS_SIZE_T col) {
	    if (col >= c) {
		tp_assert(0, "Range error.");
	    }
	    	    
	    return mm_colref<T>(*this, col);
	}
	
	template<class T>
	mm_rowref<T> mm_matrix_base<T>::operator[](TPIE_OS_SIZE_T row) {
	    return this->row(row);
	}
		
	template<class T>
	mm_matrix_base<T> &mm_matrix_base<T>::operator=(const mm_matrix_base<T> &rhs) {

	    if ((rows() != rhs.rows()) || (cols() != rhs.cols())) {
		tp_assert(0, "Range error.");
	    }
	    	    
	    TPIE_OS_SIZE_T ii,jj;
	    
	    for (ii = rows(); ii--; ) {
		for (jj = cols(); jj--; ) {
		    elt(ii,jj) = rhs.elt(ii,jj);
		}
	    }
	    
	    return *this;
	}
	
	
	template<class T>
	mm_matrix_base<T> &mm_matrix_base<T>::operator=(const mm_rowref<T> &rhs) {

	    if ((rows() != 1) || (cols() != rhs.m.cols())) {
		tp_assert(0, "Range error.");
	    }
	    
	    TPIE_OS_SIZE_T ii;
	    
	    for (ii = cols(); ii--; ) {
		elt(0,ii) = rhs[ii];
	    }
	    
	    return *this;
	}
	
	
	template<class T>
	mm_matrix_base<T> &mm_matrix_base<T>::operator=(const mm_colref<T> &rhs) {

	    if ((cols() != 1) || (rows() != rhs.m.rows())) {
		tp_assert(0, "Range error.");
	    }
	    
	    TPIE_OS_SIZE_T ii;
	    T t;
	    
	    for (ii = rows(); ii--; ) {
		t = rhs[ii];
		elt(ii,0) = t;
	    }
	    
	    return *this;
	}
	
	
	template<class T>
	mm_matrix_base<T> &mm_matrix_base<T>::
        operator+=(const mm_matrix_base<T> &rhs) {

	    if ((rows() != rhs.rows()) || (cols() != rhs.cols())) {
		tp_assert(0, "Range error.");
	    }
	    	    
	    TPIE_OS_SIZE_T ii,jj;
	    
	    for (ii = rows(); ii--; ) {
		for (jj = cols(); jj--; ) {
		    elt(ii,jj) = elt(ii,jj) + rhs.elt(ii,jj);
		}
	    }
	    
	    return *this;
	}
		
	template<class T>
	void perform_mult_in_place(const mm_matrix_base<T> &op1,
				   const mm_matrix_base<T> &op2,
				   mm_matrix_base<T> &res)	{
	    if ((op1.cols() != op2.rows()) ||
		(op1.rows() != res.rows()) ||
		(op2.cols() != res.cols())) {
		tp_assert(0, "Range error.");
	    }
	    
	    TPIE_OS_SIZE_T ii,jj,kk;
	    T t;
	    
	    // Iterate over rows of op1.
	    for (ii = op1.rows(); ii--; ) {
		// Iterate over colums of op2.
		for (jj = op2.cols(); jj--; ) {
		    // Iterate through the row of r1 and the column of r2.
		    t = op1.elt(ii,op1.cols()-1) * op2.elt(op2.rows()-1,jj);
		    for (kk = op2.rows() - 1; kk--; ) {
			t += op1.elt(ii,kk) * op2.elt(kk,jj);
		    }
		    // Assign into the result.
		    res.elt(ii,jj) = t;
		}
	    }    
	}                      

	
	template<class T>
	void perform_mult_add_in_place(mm_matrix_base<T> &op1,
				       mm_matrix_base<T> &op2,
				       mm_matrix_base<T> &res)
	{
	    if ((op1.cols() != op2.rows()) ||
		(op1.rows() != res.rows()) ||
		(op2.cols() != res.cols())) {
		tp_assert(0, "Range error.");
	    }
	    
	    TPIE_OS_SIZE_T ii,jj,kk;
	    T t;
	    
	    // Iterate over rows of op1.
	    for (ii = op1.rows(); ii--; ) {
		// Iterate over colums of op2.
		for (jj = op2.cols(); jj--; ) {
		    // Iterate through the row of r1 and the column of r2.
		    t = op1.elt(ii,op1.cols()-1) * op2.elt(op2.rows()-1,jj);
		    for (kk = op2.rows() - 1; kk--; ) {
			t += op1.elt(ii,kk) * op2.elt(kk,jj);
		    }
		    // Add into the result.
		    res.elt(ii,jj) += t;
		}
	    }    
	}                      
	
    }  //  namespace apps

} // namespace tpie

template<class T>
std::ostream &operator<<(std::ostream &s, tpie::apps::mm_matrix_base<T> &m) 	{
    
    TPIE_OS_SIZE_T ii,jj;
    
    // Iterate over rows
    for (ii = 0; ii < m.rows(); ii++) {
	// Iterate over cols
	s << m.elt(ii,0);
	for (jj = 1; jj < m.cols(); jj++) {
	    if (jj) (s << ' ');
	    s << m.elt(ii,jj);
	}
	s << '\n';
    }
    
    return s;
}

#endif // _TPIE_APPS_MM_MATRIX_BASE_H
