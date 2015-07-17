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

// A scan object to square numeric types.

#ifndef _SCAN_SQUARE_H
#define _SCAN_SQUARE_H

#include <tpie/portability.h>

using namespace tpie;

template<class T> class scan_square : ami::scan_object {
public:
    T ii;
    TPIE_OS_OFFSET called;
    scan_square() : ii(), called(0) {};
    ami::err initialize(void);
    ami::err operate(const T &in, 
		     ami::SCAN_FLAG *sfin,
		     T *out, 
		     ami::SCAN_FLAG *sfout);
};

template<class T>
ami::err scan_square<T>::initialize(void) {

    ii = 0;
    called = 0;

    return ami::NO_ERROR;
};

template<class T>
ami::err scan_square<T>::operate(const T &in, 
				 ami::SCAN_FLAG *sfin,
				 T *out, 
				 ami::SCAN_FLAG *sfout) {

    called++;
    
    if ( (*sfout = *sfin) != 0) {
        ii = in;
        *out = in * in;

	return ami::SCAN_CONTINUE;

    } 
    else {
        return ami::SCAN_DONE;
    }
};


#endif // _SCAN_SQUARE_H 
