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

#ifndef _TPIE_APPS_MM_COLREF_H
#define _TPIE_APPS_MM_COLREF_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {
    
    namespace apps { 
	
	template<class T> class mm_matrix_base;
	template<class T> class mm_matrix;
	
	template<class T> 
	class mm_colref {

	private:
	    mm_matrix_base<T> &m;
	    TPIE_OS_SIZE_T c;

	public:
	    mm_colref(mm_matrix_base<T> &amatrix, TPIE_OS_SIZE_T col);
	    ~mm_colref(void);
	    
	    T &operator[](const TPIE_OS_SIZE_T col) const;
	    
	    mm_colref<T> &operator=(const mm_colref<T> &rhs);

		friend class mm_matrix_base<T>;
	    friend class mm_matrix<T>;
	};



	template<class T>
	mm_colref<T>::mm_colref(mm_matrix_base<T> &amatrix, TPIE_OS_SIZE_T col) :
	    m(amatrix),
	    c(col) {
	}
	
	template<class T>
	mm_colref<T>::~mm_colref(void) {
	}
	
	template<class T>
	T &mm_colref<T>::operator[](const TPIE_OS_SIZE_T row) const {
	    return m.elt(row,c);
	}

	template<class T>
	mm_colref<T> &mm_colref<T>::operator=(const mm_colref<T> &rhs) {

	    if (c != rhs.c ) {
			tp_assert(0, "Range error.");
	    }
	    	    
		m = rhs.m;

	    return *this;
	}
	
    }  //  namespace apps

}  //  namespace tpie 

#endif // _TPIE_APPS_MM_COLREF_H 
