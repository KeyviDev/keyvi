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

///////////////////////////////////////////////////////////////////////////
/// \file cache.h 
/// Declaration and definition of CACHE_MANAGER implementation(s).
/// Provides means to choose and set a specific cache manager/
/// For now the only cache memory manager implemented is 
///  \ref cache_manager_lru.
///////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_AMI_CACHE_H
#define _TPIE_AMI_CACHE_H

#include <tpie/cache_base.h>
#include <tpie/cache_lru.h>

namespace tpie {
    
    namespace ami {
	
/** The only cache manager implementation so far is cache_manager_lru. */
#define CACHE_MANAGER cache_manager_lru

    }  //  ami namespace

}  //  tpie namespace


#endif // _TPIE_AMI_CACHE_H
