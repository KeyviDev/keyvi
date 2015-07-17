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

#ifndef _TPIE_AMI_SCAN_UTILS_H
#define _TPIE_AMI_SCAN_UTILS_H

///////////////////////////////////////////////////////////////////////////
/// \file scan_utils.h 
/// Declares scan_object subclasses for writing a C++ stream into a TPIE
/// stream and for writing a TPIE stream to a C++ stream.
///////////////////////////////////////////////////////////////////////////

#include <iostream>

// Get definitions for working with Unix and Windows.
#include <tpie/portability.h>
// Get the AMI_scan_object definition.
#include <tpie/scan.h>

namespace tpie {

    namespace ami {

  ///////////////////////////////////////////////////////////////////////////
  /// A scan object class template for reading the contents of an
  /// ordinary C++ input stream into a TPIE stream.  It works with
  /// streams of any type for which an >> operator is defined for C++
  /// stream input.
  ///////////////////////////////////////////////////////////////////////////
	template<class T> class cxx_istream_scan : scan_object {
	    
	private:
	    // Prohibit these.
	    cxx_istream_scan(const cxx_istream_scan<T>& other);
	    cxx_istream_scan<T>& operator=(const cxx_istream_scan<T>& other);
	    std::istream *is;

	public:
	  ///////////////////////////////////////////////////////////////////////////
	  /// Create a scan management ob ject for scanning the contents of C++ stream 
	  /// *instr. The actual scanning is done using AMI_scan with no input streams
	  /// and one output stream.
	  ///////////////////////////////////////////////////////////////////////////
	  cxx_istream_scan(std::istream *instr = &std::cin);
	    err initialize(void);
	    err operate(T *out, SCAN_FLAG *sfout);
	};
	
	template<class T>
	cxx_istream_scan<T>::cxx_istream_scan(std::istream *instr) : is(instr) {
	    //  No code in this constructor.
	};

	template<class T>
	err cxx_istream_scan<T>::initialize(void) {
	    return NO_ERROR;
	};

	template<class T>
	err cxx_istream_scan<T>::operate(T *out, SCAN_FLAG *sfout) {
	    if (*is >> *out) {
		*sfout = true;
		return SCAN_CONTINUE;
	    } 
	    else {
		*sfout = false;
		return SCAN_DONE;
	    }
	};

    }  //  ami namespace

}  //  tpie namespace 


namespace tpie {
    
    namespace ami {
	
  ///////////////////////////////////////////////////////////////////////////
  /// A scan management class template for writing the contents of a TPIE
  /// stream into an ordinary C++ output stream. It works with streams of
  /// any type for which a << operator is defined for C++ stream output. 
  ///////////////////////////////////////////////////////////////////////////
	template<class T> class cxx_ostream_scan : scan_object {

	private:
	    // Prohibit these.
	    cxx_ostream_scan(const cxx_ostream_scan<T>& other);
	    cxx_ostream_scan<T>& operator=(const cxx_ostream_scan<T>& other);
	    std::ostream *os;

	public:

    ///////////////////////////////////////////////////////////////////////////
	  /// Create a scan management ob ject for scanning into C++ stream *outstr.
	  /// The actual scanning is done using AMI_scan with one input stream and no 
	  /// output streams.
    ///////////////////////////////////////////////////////////////////////////
	  cxx_ostream_scan(std::ostream *outstr = &std::cout);
	    err initialize(void);
	    err operate(const T &in, SCAN_FLAG *sfin);
	};
	
	template<class T>
	cxx_ostream_scan<T>::cxx_ostream_scan(std::ostream *outstr) : os(outstr) {
	    //  No code in this constructor.
	};
	
	template<class T>
	err cxx_ostream_scan<T>::initialize(void) {
	    return NO_ERROR;
	};
	
	template<class T>
	err cxx_ostream_scan<T>::operate(const T &in, SCAN_FLAG *sfin) {
	    if (*sfin) {
		*os << in << '\n';
		return SCAN_CONTINUE;
	    } 
	    else {
		return SCAN_DONE;
	    }
	};
	
    }  //  ami namespace
    
}  //  tpie namespace 


#endif // _TPIE_AMI_SCAN_UTILS_H 
