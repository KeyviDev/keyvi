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

// A scanner to diff two streams of the same type of object, to verify that
// they are the same.

#ifndef _SCAN_DIFF_H
#define _SCAN_DIFF_H

using namespace tpie;

template<class T> struct scan_diff_out {
    unsigned int index;
    T t0, t1;
};

template<class T> class scan_diff : ami::scan_object {
private:
    unsigned int input_index;
    T null_t;
public:
    scan_diff(const T &nt);
    virtual ~scan_diff(void);
    ami::err initialize();
    ami::err operate(const T &in1, const T &in2, ami::SCAN_FLAG *sfin,
		     scan_diff_out<T> *out,
		     ami::SCAN_FLAG *sfout);
};

template<class T>
scan_diff<T>::scan_diff(const T &nt) : input_index(0), null_t(nt) {
}

template<class T>
scan_diff<T>::~scan_diff()
{
}

template<class T>
ami::err scan_diff<T>::initialize(void)
{
    input_index = 0;
    return ami::NO_ERROR;
}

template<class T>
ami::err scan_diff<T>::operate(const T &in1, const T &in2,
			       ami::SCAN_FLAG *sfin,
			       scan_diff_out<T> *out,
			       ami::SCAN_FLAG *sfout)
{
    ami::err ret;
    
    if (sfin[0] && sfin[1]) {
        if (in1 == in2) {
            *sfout = false;
        } else {
            *sfout = true;
            out->index = input_index;
            out->t0 = in1;
            out->t1 = in2;
        }
        ret =  ami::SCAN_CONTINUE;
    } else if (!sfin[0] && !sfin[1]) {
        *sfout = false;
        ret = ami::SCAN_DONE;
    } else {
        ret =  ami::SCAN_CONTINUE;
        *sfout = true;
        out->index = input_index;
        if (!sfin[0]) {
            out->t0 = null_t;
            out->t1 = in2;
        } else { 
            out->t1 = null_t;
            out->t0 = in1;
        }
    }
    input_index++;
    return ret;
}   

#endif // _SCAN_DIFF_H 
