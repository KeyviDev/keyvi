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

// A stack implemented using BTE_stream_ufs. It is used by
// BTE_collection_base to implement deletions.  
#ifndef _TPIE_BTE_STACK_UFS_H
#define _TPIE_BTE_STACK_UFS_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/bte/stream_ufs.h>

namespace tpie {

    namespace bte {
    
	template<class T>
	class stack_ufs : public stream_ufs<T> {

	public:
	    using stream_ufs<T>::stream_len;
	    using stream_ufs<T>::seek;
	    using stream_ufs<T>::truncate;
		using stream_ufs<T>::write_item;
		using stream_ufs<T>::read_item;

	    // Construct a new stack with the given name and access type.
	    stack_ufs(std::string& path, stream_type type = WRITE_STREAM); 

	    // Destroy this object.
	    ~stack_ufs(void);

	    // Push an element on top of the stack.
	    err push(const T &t);
	
	    // Pop an element from the top of the stack.
	    err pop(T **t);

	};
    
    
	template<class T>
		stack_ufs<T>::stack_ufs(std::string& path, 
								stream_type type) :
	    stream_ufs<T>(path, type, 1) {
	    // No code in this constructor.
	}
    
	template<class T>
	stack_ufs<T>::~stack_ufs(void) {
	    // No code in this destructor.
	}
    
	template<class T>
	err stack_ufs<T>::push(const T &t) {

	    err retval = NO_ERROR;
	    TPIE_OS_OFFSET slen = stream_len();
           
	    if ((retval = truncate(slen+1)) != NO_ERROR) {
		return retval;
	    }

	    if ((retval = seek(slen)) != NO_ERROR) {
		return retval;
	    }
	
	    return write_item(t);
	}
    
    
	template<class T>
	err stack_ufs<T>::pop(T **t) {

	    err retval = NO_ERROR;
	    TPIE_OS_OFFSET slen = stream_len();

	    if ((retval = seek(slen-1)) != NO_ERROR) {
		return retval;
	    }
  
	    if ((retval =  read_item(t)) != NO_ERROR) {
		return retval;
	    }
	
	    return truncate(slen-1);
	}
    
    }  //  bte namespace

}  //  tpie namespace

#endif // _TPIE_BTE_STACK_UFS_H 
