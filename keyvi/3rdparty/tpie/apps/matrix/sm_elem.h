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
#ifndef _TPIE_APPS_SM_ELEM_H
#define _TPIE_APPS_SM_ELEM_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <iostream>

namespace tpie {

    namespace apps {

// A sparse matrix element is labeled with a row er and a column ec.
// A sparse matrix is simply represented by a colletion of these.
	template <class T>
	class sm_elem {

	public:
	    TPIE_OS_OFFSET er;
	    TPIE_OS_OFFSET ec;
	    T val;

	};

	template <class T>
	std::ostream &operator<<(std::ostream& s, const sm_elem<T> &a) {
	    return s << a.er << ' ' << a.ec << ' ' << a.val;
	};
	
	template <class T>
	std::istream &operator>>(std::istream& s, sm_elem<T> &a) {
	    return s >> a.er >> a.ec >> a.val;
	};
	
    }  //  namespace apps

}  //  namespace tpie 



#endif // _TPIE_APPS_SM_ELEM_H
