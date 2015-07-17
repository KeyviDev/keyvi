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
// File: fill_value.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: fill_value.h,v 1.5 2005-11-17 17:07:40 jan Exp $
//
#ifndef _TPIE_APPS_FILL_VALUE_H
#define _TPIE_APPS_FILL_VALUE_H

#include <tpie/portability.h>

#include "matrix_fill.h"

namespace tpie {

    namespace apps {
	
	template<class T>
	class fill_value : public matrix_filler<T> {
	    
	private:
	    T value;
	public:
	    
	    fill_value() : value() {};

	    void set_value(const T &v) {
		value = v;
	    };

	    ami::err initialize(TPIE_OS_OFFSET /*rows*/, TPIE_OS_OFFSET /*cols*/) {
		return ami::NO_ERROR;
	    };
	    
	    T element(TPIE_OS_OFFSET /*row*/, TPIE_OS_OFFSET /*col*/) {
		return value;
	    };
	};

    }  //  namespace apps

}  //  namespace tpie

#endif // _TPIE_APPS_FILL_VALUE_H 
