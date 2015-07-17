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

///////////////////////////////////////////////////////////////////
/// \file cache_base.h
/// Provides the base class \ref cache_manager_base for all cache manager
/// implementations.
///////////////////////////////////////////////////////////////////

#ifndef _TPIE_AMI_CACHE_BASE_H
#define _TPIE_AMI_CACHE_BASE_H

#include <tpie/portability.h>

namespace tpie {
    
    namespace ami {

    ////////////////////////////////////////////////////////////////////
    /// Base class for all cache manager implementations.
    /// For now the only cache memory manager implemented is 
    ///  \ref cache_manager_lru.
    ////////////////////////////////////////////////////////////////////
    class cache_manager_base {

	protected:
	    /** Max size in bytes. */
	    TPIE_OS_SIZE_T capacity_;
	    
	    /** Associativity */
	    TPIE_OS_SIZE_T assoc_;
	    
	    /** Behavior. */
	    int behavior_;
	    
	    ////////////////////////////////////////////////////////////////////
	    ///  Construct a fully-associative cache manager with the given capacity.
	    ////////////////////////////////////////////////////////////////////
	    cache_manager_base(TPIE_OS_SIZE_T capacity, 
			       TPIE_OS_SIZE_T assoc):
		capacity_(capacity), 
		assoc_(assoc), 
		behavior_(0) {
		//  No code in this constructor.
	    }
	    
	public:
	    /** Set behavior. TODO: Expand. */
	    int behavior(int b) { 
		behavior_ = b; 
		return behavior_; 
	    }
	    
	    /**  Inquire behavior. */
	    int behavior() const { 
		return behavior_; 
	    }
	};

    }  //  ami namespace

} //  tpie namespace


#endif // _TPIE_AMI_CACHE_BASE_H
