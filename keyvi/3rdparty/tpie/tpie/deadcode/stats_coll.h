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

// Statistics for block collections.

#ifndef _TPIE_STATS_COLL_H
#define _TPIE_STATS_COLL_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie_stats_coll.h
/// Enum type declarations for the statistics object for TPIE collections.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/stats.h>

namespace tpie {
    
/** Statistics tags for profiling a) a single collection or b)
 * all collections in an application instance. */
    enum stats_collection_id {
	/** Number of block reads */ 
	BLOCK_GET = 0, 
	/** Number of block writes */ 
	BLOCK_PUT,
	/** Number of block creates */ 
	BLOCK_NEW,
	/** Number of block deletes */ 
	BLOCK_DELETE,
	/** Number of block sync operations */ 
	BLOCK_SYNC,
	/** Number of collection open operations */ 
	COLLECTION_OPEN,
	/** Number of collection close operations */ 
	COLLECTION_CLOSE,
	/** Number of collection create operations */ 
	COLLECTION_CREATE,
	/** Number of collection delete operations */ 
	COLLECTION_DELETE,
	NUMBER_OF_COLLECTION_STATISTICS
    };
    
/** Typedef for the statistics object for TPIE collections. */
    typedef stats<NUMBER_OF_COLLECTION_STATISTICS> stats_collection;

}  //  tpie namespace

#endif //_TPIE_STATS_COLL_H
