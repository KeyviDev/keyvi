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

#ifndef _TPIE_AMI_COLL_H
#define _TPIE_AMI_COLL_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie/coll.h Provides means to choose and set a specific collection type.
/// For now \ref tpie::ami::collection_single is the only implementation.
///////////////////////////////////////////////////////////////////////////

#include <tpie/config.h>

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/coll_base.h>
#include <tpie/coll_single.h>

namespace tpie {

    namespace ami {



///////////////////////////////////////////////////////////////////////////
/// As \ref tpie::ami::collection_single is for now the only collection implementation,
/// define collection to point to collection_single.
///////////////////////////////////////////////////////////////////////////
#define AMI_collection collection_single
	
#ifdef BTE_COLLECTION
#  define AMI_COLLECTION collection_single< BTE_COLLECTION >
#endif

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_COLL_H
