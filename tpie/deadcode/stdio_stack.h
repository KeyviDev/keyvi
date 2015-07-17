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

// Defining a stack based on bte_stdio separately specifically
// for use in block collection class and related apps.
// The reason we don't want to use ami_stack is because 
// then the stack would be implemented as a BTE_STREAM, which
// may have large block size etc. which is undesirable for 
// stacks related to block collections since such a stack is only
// a meta data structure accessed no more than once every block
// is created or destroyed. 

#ifndef _TPIE_AMI_STDIO_STACK_H_
#define _TPIE_AMI_STDIO_STACK_H_

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <bte/stream_stdio.h>

namespace tpie {

    namespace ami {

	template<class T>
	class stdio_stack : public bte::stream_stdio<T> {
	public:
	    
	    stdio_stack(std::string& path, bte::stream_type type = bte::WRITE_STREAM); 
	    
	    ~stdio_stack(void);
	    
	    err push(const T &t);
	    
	    err pop(T **t);
	    
	};
	
	
	template<class T>
	stdio_stack<T>::stdio_stack(std::string& path, 
				    bte::stream_type type) :
	    bte::stream_stdio<T>(path, type) {
	    //  No code in this constructor.
	}
	
	template<class T>
	stdio_stack<T>::~stdio_stack(void) {
	    //  No code in this destructor.
	}
	
	template<class T>
	err stdio_stack<T>::push(const T &t) {
	    bte::err be;
	    TPIE_OS_OFFSET slen;
	    
	    be = truncate((slen = stream_len())+1);
	    if (be != bte::NO_ERROR) {
		return BTE_ERROR;
	    }
	    
	    be = seek(slen);
	    if (be != NO_ERROR) {
		return BTE_ERROR;
	    }
	    
	    be = write_item(t);

	    if (be != NO_ERROR) {
		return BTE_ERROR;
	    }
	    
	    return NO_ERROR;
	}
	
	
	template<class T>
	err stdio_stack<T>::pop(T **t) {
	    bte::err be;
	    TPIE_OS_OFFSET slen;
	    
	    slen = stream_len();

	    be = seek(slen-1);
	    if (be != bte::NO_ERROR) {
		return BTE_ERROR;
	    }
	    
	    be = read_item(t);
	    if (be != bte::NO_ERROR) {
		return BTE_ERROR;
	    }
	    
	    br = truncate(slen-1);
	    if (be != bte::NO_ERROR) {
		return BTE_ERROR;
	    }

	    return NO_ERROR;
	    
	}

    }  //  ami namespace

}  //  tpie namespaxe

#endif // _TPIE_AMI_STDIO_STACK_H_ 
