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

#ifndef _SCAN_UNIVERSAL_H
#define _SCAN_UNIVERSAL_H

// Get the STL std::min fonction.
#include <algorithm>
// Get the AMI_scan_object definition.
#include <tpie/scan.h>

using namespace tpie;

template <int sz>
struct ifoo_t {
    int i;
    char el[sz-sizeof(int)];
    // Default constructor.
    ifoo_t(): i(0) { }
    // This class also acts as comparison class for MAI_sort.
    int compare(const ifoo_t<sz>& lhi, const ifoo_t<sz>& rhi) const {
	return (lhi.i > rhi.i ? 1: lhi.i < rhi.i ? -1: 0);
    }
    // Comparison operator.
    bool operator<(const ifoo_t<sz>& rhi) const {
	return i < rhi.i;
    }
};

// A scan object to generate random integers.
template <int sz>
class scan_universal : ami::scan_object {
private:
    unsigned int _max, _remaining;
    int _even, _odd;
    int _switches;
    bool _have_prev;
public:
    scan_universal(unsigned int count = 1000, int seed = 17): 
	_max(count), _remaining(count), _even(0), _odd(0), _switches(0), _have_prev(false) {
	TP_LOG_APP_DEBUG_ID("scan_universal random seed:");
	TP_LOG_APP_DEBUG_ID(seed);
	TPIE_OS_SRANDOM(seed);     
    }

    virtual ~scan_universal(void) {}

    ami::err initialize(void) {
	_have_prev = false;
	_remaining = _max;
	_even = _odd = 0;
	_switches = 0;
	return ami::NO_ERROR;
    }

    // Generating random ints.
    ami::err operate(int *out0, ami::SCAN_FLAG *sf) {
	if ((*sf = (_remaining-- != 0))) {
	    *out0 = TPIE_OS_RANDOM();
	    return ami::SCAN_CONTINUE;
	} else {
	    return ami::SCAN_DONE;
	}
    }

    // Generate ifoo_t's with random ints.
    ami::err operate(ifoo_t<sz>* out0, ami::SCAN_FLAG *sf) {
	if ((*sf = (_remaining-- != 0))) {
	    out0->i = TPIE_OS_RANDOM();
	    //    out0->el[0] = char (out0.i % 128);
	    return ami::SCAN_CONTINUE;
	} else {
	    return ami::SCAN_DONE;
	}
    }

    // Counting switches, even, and odd in stream of ints.
    ami::err operate(const int& in0, ami::SCAN_FLAG* sfin) {
	static int prev;
	if (*sfin) {
	    if (in0 % 2 == 0) {
		_even++;
	    } else {
		_odd++;
	    }

	    if (_have_prev && in0 < prev) {
		_switches++;
	    }
	    prev = in0;
	    _have_prev = true;
	    return ami::SCAN_CONTINUE;
	} else {
	    return ami::SCAN_DONE;
	}
    }

    // Counting switches in stream of ifoo_t's
    ami::err operate(const ifoo_t<sz>& in0, ami::SCAN_FLAG* sfin) {
	static ifoo_t<sz> prev;
	if (*sfin) {
	    if (_have_prev && in0 < prev) {
		_switches++;
	    }
	    prev = in0;
	    _have_prev = true;
	    return ami::SCAN_CONTINUE;
	} else {
	    return ami::SCAN_DONE;
	}    
    }

    // Halving each int.
    ami::err operate(const int& in0, ami::SCAN_FLAG *sfin,
		     int *out0, ami::SCAN_FLAG *sfout) {
	if ( (*sfout = *sfin) != 0 ) {
	    *out0 = in0 / 2;
	    return ami::SCAN_CONTINUE;
	} else {
	    return ami::SCAN_DONE;
	}
    }

    // Taking std::min of each pair.
    ami::err operate(const int& in0, const int& in1, ami::SCAN_FLAG *sfin,
		     int *out0, ami::SCAN_FLAG *sfout) {
	if ( ((*sfout = sfin[0]) != 0) || (sfin[1] != 0)) {
	    if (sfin[0] && sfin[1]) {
		*out0 = std::min(in0, in1);
	    } else  {
		*out0 = (sfin[0] ? in0: in1);
	    }
		*sfout = 1; // There was input so we've produced output
	    return ami::SCAN_CONTINUE;
	} else {
	    return ami::SCAN_DONE;
	}
    }

    // Even ints in first stream and odd ints in second stream.
    ami::err operate(const int& in0, const int& in1, ami::SCAN_FLAG *sfin,
		     int *out0, int *out1, ami::SCAN_FLAG *sfout) {
	sfout[0] = sfout[1] = 0;
    
	if (sfin[0]) {
	    if (in0 % 2 == 0) {
		*out0 = in0;
		sfout[0] = 1;
	    } else {
		*out1 = in0;
		sfout[1] = 1;
	    }
	}
    
	if (sfin[1]) {
	    if (in1 % 2 == 0) {
		if (!sfout[0]) {
		    *out0 = in1;
		    sfout[0] = 1;
		} else {
		    sfin[1] = 0;
		}
	    } else { 
		if (!sfout[1]) {
		    *out1 = in1;
		    sfout[1] = 1;
		} else {
		    sfin[1] = 0;
		}
	    }
	}
    
	if (sfout[0] || sfout[1])
	    return ami::SCAN_CONTINUE;
	else
	    return ami::SCAN_DONE;    
    }
  
    // Outputs: avg, std::min, std::max.
    ami::err operate(const int& in0, const int& in1, 
		     const int& in2, const int& in3, ami::SCAN_FLAG *sfin,
		     int *out0, int *out1, int *out2,
		     ami::SCAN_FLAG *sfout) {
	if ( ((sfout[0] = sfout[1] = sfout[2] = sfin[0]) != 0) || 
		(sfin[1] != 0) || 
		(sfin[2] != 0) || 
		(sfin[3] != 0) ) {
	    int c = 0;
	    if (sfin[0]) c++;
	    if (sfin[1]) c++;
	    if (sfin[2]) c++;
	    if (sfin[3]) c++;
      
	    *out0 = (sfin[0] ? in0: 0) / c + (sfin[1] ? in1: 0) / c + 
		(sfin[2] ? in2: 0) / c + (sfin[3] ? in3: 0) / c;
      
	    *out1 = sfin[0] ? in0: sfin[1] ? in1: sfin[2] ? in2: in3;
	    if (sfin[1]) *out1 = std::min(*out1, in1);
	    if (sfin[2]) *out1 = std::min(*out1, in2);
	    if (sfin[3]) *out1 = std::min(*out1, in3);
      
	    *out2 = sfin[0] ? in0: sfin[1] ? in1: sfin[2] ? in2: in3;
	    if (sfin[1]) *out2 = std::max(*out2, in1);
	    if (sfin[2]) *out2 = std::max(*out2, in2);
	    if (sfin[3]) *out2 = std::max(*out2, in3);

		sfout[0] = sfout[1] = sfout[2] = 1; // Make sure we keep writing output even if the first stream is empty    

	    return ami::SCAN_CONTINUE;
	} else
	    return ami::SCAN_DONE;    
    }
  
    int even() const { return _even; }
    int odd() const { return _odd; }
    int switches() const { return _switches; }
};

#endif // _SCAN_UNIVERSAL_H 
