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

#ifndef _TPIE_APPS_MM_ROWREF_H
#define _TPIE_APPS_MM_ROWREF_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {

    namespace apps { 

	template<class T> class mm_matrix_base;
	template<class T> class mm_matrix;

	template<class T>
	class mm_rowref {
	    
	private:
	    mm_matrix_base<T> &m;
	    TPIE_OS_SIZE_T r;

	public:
	    mm_rowref(mm_matrix_base<T> &amatrix, TPIE_OS_SIZE_T row);
	    ~mm_rowref(void);
	    
	    T &operator[](const TPIE_OS_SIZE_T col) const;
	    
	    mm_rowref<T> &operator=(const mm_rowref<T> &rhs);

	    friend class mm_matrix_base<T>;
	    friend class mm_matrix<T>;
	};


	template<class T>
	mm_rowref<T>::mm_rowref(mm_matrix_base<T> &amatrix, TPIE_OS_SIZE_T row) :
	    m(amatrix),
	    r(row) {
	}

	template<class T>
	mm_rowref<T>::~mm_rowref(void) {
	}
	
	template<class T>
	T &mm_rowref<T>::operator[](const TPIE_OS_SIZE_T col) const {
	    return m.elt(r,col);
	}

	template<class T>
	mm_rowref<T> &mm_rowref<T>::operator=(const mm_rowref<T> &rhs) {

	    if (r != rhs.r ) {
			tp_assert(0, "Range error.");
	    }
	    	    
		m = rhs.m;

	    return *this;
	}


    }  //  namespace apps
    
} // namespace ami

#endif // _TPIE_APPS_MM_ROWREF_H
