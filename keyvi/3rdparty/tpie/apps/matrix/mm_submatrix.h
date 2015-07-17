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
#ifndef _TPIE_APPS_MM_SUBMATRIX_H
#define _TPIE_APPS_MM_SUBMATRIX_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/tpie_assert.h>

#include "mm_matrix_base.h"

namespace tpie {

    namespace apps { 

// A submatrix class.
	template<class T>
	class mm_submatrix : public mm_matrix_base<T> {

	private:	    
	    mm_matrix_base<T> &m;
	    TPIE_OS_SIZE_T r1,r2,c1,c2;

	public:
	    using mm_matrix_base<T>::rows;
	    using mm_matrix_base<T>::cols;
	    
	    // Construction/destruction.
	    mm_submatrix(mm_matrix_base<T> &amatrix,
		      TPIE_OS_SIZE_T row1, TPIE_OS_SIZE_T row2,
		      TPIE_OS_SIZE_T col1, TPIE_OS_SIZE_T col2);
	    
	    virtual ~mm_submatrix(void);
	    
	    // We need an assignement operator that copies data by explicitly
	    // calling the base class's assignment operator to do elementwise
	    // copying.  Otherwise, m, r1, r2, c1, and c2 are just copied.
	    mm_submatrix<T> &operator=(const mm_submatrix<T> &rhs);
	    
	    // We also want to be able to assign from matrices.
	    mm_submatrix<T> &operator=(const mm_matrix<T> &rhs);
	    
	    // Access to elements.
	    T& elt(TPIE_OS_SIZE_T row, TPIE_OS_SIZE_T col) const;
	};
	
	template<class T>
	mm_submatrix<T>::mm_submatrix(mm_matrix_base<T> &amatrix,
				TPIE_OS_SIZE_T row1, TPIE_OS_SIZE_T row2,
				TPIE_OS_SIZE_T col1, TPIE_OS_SIZE_T col2) :
	    mm_matrix_base<T>(row2 - row1 + 1,
			   col2 - col1 + 1),
	    m(amatrix),
	    r1(row1), r2(row2),
	    c1(col1), c2(col2) {
	}
	
	template<class T>
	mm_submatrix<T>::~mm_submatrix(void) {
	}
	
	template<class T>
	mm_submatrix<T> &mm_submatrix<T>::operator=(const mm_submatrix<T> &rhs) {

	    // Call the assignement operator from the base class to do range
	    // checking and elementwise assignment.
	    (mm_matrix_base<T> &)(*this) = (mm_matrix_base<T> &)rhs;
	    
	    return *this;
	}
	
	template<class T>
	mm_submatrix<T> &mm_submatrix<T>::operator=(const mm_matrix<T> &rhs) {

	    // Call the assignement operator from the base class to do range
	    // checking and elementwise assignment.
	    (mm_matrix_base<T> &)(*this) = (mm_matrix_base<T> &)rhs;
	    
	    return *this;
	}
	
	template<class T>
	T& mm_submatrix<T>::elt(TPIE_OS_SIZE_T row, TPIE_OS_SIZE_T col) const {

	    if ((row >= rows()) || (col >= cols())) {
		tp_assert(0, "Range error.");
	    }

	    return m.elt(row + r1, col + c1);
	}
	
    }  //  namespace apps

}  //  namespace tpie


#endif // _TPIE_APPS_MM_SUBMATRIX_H
