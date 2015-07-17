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
// File: fill_upper_tri.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/12/94
//
// $Id: fill_upper_tri.h,v 1.7 2005-11-16 17:04:32 jan Exp $
//
#ifndef _FILL_UPPER_TRI_H
#define _FILL_UPPER_TRI_H

#include <tpie/portability.h>

#include "matrix_fill.h"

namespace tpie {

    namespace apps {

	template<class T>
	class fill_upper_tri : public matrix_filler<T> {

	private:
	    T val;
	public:

	    fill_upper_tri(T t) : val(t) {};
	    virtual ~fill_upper_tri() {};
	    
	    ami::err initialize(TPIE_OS_OFFSET /*rows*/, TPIE_OS_OFFSET /*cols*/) {
		return ami::NO_ERROR;
	    };
	    
	    T element(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col) {
		return (row <= col) ? val : 0;
	    };
	};

    }  //  namespace apps

}  // namespace tpie

#endif // _FILL_UPPER_TRI_H 
