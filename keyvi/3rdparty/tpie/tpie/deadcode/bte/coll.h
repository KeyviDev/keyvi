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

// Front end for the BTE collection classes.
#ifndef _TPIE_BTE_COLL_H
#define _TPIE_BTE_COLL_H

// Get the base class and various definitions.
#include <tpie/bte/coll_base.h>

// The MMAP implementation.
#include <tpie/bte/coll_mmap.h>

// The UFS implementation.
#include <tpie/bte/coll_ufs.h>

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {

    namespace bte {
		
#define _BTE_COLL_IMP_COUNT (defined(COLLECTION_IMP_UFS) +		\
                             defined(COLLECTION_IMP_MMAP) +		\
                             defined(COLLECTION_IMP_USER_DEFINED))
	
// Multiple implem. are included, but we have to choose a default one.
#if (_BTE_COLL_IMP_COUNT > 1)
#  define COLLECTION_IMP_MMAP
#elif (_BTE_COLL_IMP_COUNT == 0)
#  define COLLECTION_IMP_MMAP
#endif
	
#define COLLECTION_MMAP collection_mmap<TPIE_BLOCK_ID_TYPE>
#define COLLECTION_UFS  collection_ufs<TPIE_BLOCK_ID_TYPE>
	
#if defined(COLLECTION_IMP_MMAP)
#  define COLLECTION COLLECTION_MMAP
#elif defined(COLLECTION_IMP_UFS)
#  define COLLECTION COLLECTION_UFS
#elif defined(COLLECTION_IMP_USER_DEFINED)
   // Do not define BTE_COLLECTION. The user will define it.
#endif

    }  //  bte namespace

}  //  tpie namespace
	
#endif // _TPIE_BTE_COLL_H
