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

// General permutation.
#ifndef _TPIE_AMI_GEN_PERM_H
#define _TPIE_AMI_GEN_PERM_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
// Get AMI_scan_object.
#include <tpie/scan.h>
// Get AMI_sort
#include <tpie/sort.h>

#include <tpie/gen_perm_object.h>

namespace tpie {

    namespace ami {
	
// (tavi) moved dest_obj definition down due to error in gcc 2.8.1
	template<class T> class dest_obj;
	
// A comparison operator that simply compares destinations (for sorting).
	template<class T>
	int operator<(const dest_obj<T> &s, const dest_obj<T> &t) {
	    return s.dest < t.dest;
	}
	
	template<class T>
	int operator>(const dest_obj<T> &s, const dest_obj<T> &t) {
	    return s.dest > t.dest;
	}
	
    }  //  ami namespace

}  //  tpie namespace

namespace tpie {
    
    namespace ami {
	
	template<class T>
	class gen_perm_add_dest : scan_object {

	private:
	    // Prohibit these
	    gen_perm_add_dest(const gen_perm_add_dest<T>& other);
	    gen_perm_add_dest<T> operator=(const gen_perm_add_dest<T>& other);
	    
	    gen_perm_object *pgp;
	    TPIE_OS_OFFSET input_offset;
	    
	public:
	    gen_perm_add_dest(gen_perm_object *gpo) : pgp(gpo), input_offset(0) {
		//  No code in this constructor.
	    };

	    virtual ~gen_perm_add_dest(void) {
		//  No code in this destructor.
	    };
	    
	    err initialize(void) {
		input_offset = 0; 
		return NO_ERROR; 
	    };

	    err operate(const T &in, SCAN_FLAG *sfin, dest_obj<T> *out, SCAN_FLAG *sfout) {
		if ((*sfout = *sfin) == 0) {
		    return SCAN_DONE;
		}

		*out = dest_obj<T>(in, pgp->destination(input_offset++));

		return SCAN_CONTINUE;
	    }
	};

    }  //  ami namespace

}  // tpie namespace
    

namespace tpie {

    namespace ami {
	
	template<class T>
	class gen_perm_strip_dest : scan_object {
	    
	public:
	    err initialize(void) { 
		return NO_ERROR; 
	    };
	    
	    err operate(const dest_obj<T> &in, SCAN_FLAG *sfin, T *out, SCAN_FLAG *sfout) {

		if ((*sfout = *sfin)==0) {
		    return SCAN_DONE;
		}

		*out = in.t;
		
		return SCAN_CONTINUE;
	    }
	};

    }  //  ami namespace

}  //  tpie namespace

namespace tpie {

    namespace ami {

	template<class T>
	class dest_obj {

	private:
	    T t;
	    TPIE_OS_OFFSET dest;

	public:
	    dest_obj() : t(), dest(0) {
		//  No code in this constructor.
	    };
	    
	    dest_obj(T t_in, TPIE_OS_OFFSET d) : t(t_in), dest(d) {
		//  No code in this constructor
	    };
	    
	    ~dest_obj() {
		//  No code in this destructor.
	    };

	    
	    // The second alternative caused problems on Win32 (jv)
//#if (__GNUC__ > 2) || (__GNUC__ == 2 &&  __GNUC_MINOR__ >= 8)
	    friend int operator< <> (const dest_obj<T> &s, const dest_obj<T> &t);
	    friend int operator> <> (const dest_obj<T> &s, const dest_obj<T> &t);
//#else
//    friend int operator< (const dest_obj<T> &s, const dest_obj<T> &t);
//    friend int operator> (const dest_obj<T> &s, const dest_obj<T> &t);
//#endif
	    friend err gen_perm_strip_dest<T>::operate(const dest_obj<T> &in,
						       SCAN_FLAG *sfin, T *out,
						       SCAN_FLAG *sfout);
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
	template<class T>
	err general_permute(stream<T> *instream, stream<T> *outstream,
			    gen_perm_object *gpo) {
	    
	    err ae;
	    gen_perm_add_dest<T> gpad(gpo);
	    gen_perm_strip_dest<T> gpsd;
	    stream< dest_obj<T> > sdo_in;
	    stream< dest_obj<T> > sdo_out;
	    
	    // Initialize
	    ae = gpo->initialize(instream->stream_len());
	    if (ae != NO_ERROR) {
		return ae;
	    }
    
	    // Scan the stream, producing an output stream that labels each
	    // item with its destination.
	    ae = scan(instream, &gpad,&sdo_in);	    
	    if (ae != NO_ERROR) {
		return ae;
	    }
	    
	    // Sort by destination.
	    ae = sort(&sdo_in, &sdo_out);
	    if (ae != NO_ERROR) {
		return ae;
	    }
	    
	    // Scan to strip off the destinations.
	    ae = scan(&sdo_out, &gpsd, outstream);	    
	    if (ae != NO_ERROR) {
		return ae;
	    }
	    
	    return NO_ERROR;        
	}
	
    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_GEN_PERM_H 
