// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=8 sts=4 sw=4 noet :
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

#ifndef _TPIE_STATS_TREE_H
#define _TPIE_STATS_TREE_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/stats.h>

namespace tpie {

    enum stats_tree_id {
	LEAF_FETCH = 0,
	LEAF_RELEASE,
	LEAF_READ,
	LEAF_WRITE,
	LEAF_CREATE,
	LEAF_DELETE,
	LEAF_COUNT,
	NODE_FETCH,
	NODE_RELEASE,
	NODE_READ,
	NODE_WRITE,
	NODE_CREATE,
	NODE_DELETE,
	NODE_COUNT,
	NUMBER_OF_TREE_STATISTICS
    };
    
    typedef stats<NUMBER_OF_TREE_STATISTICS> stats_tree;

}  //  tpie namespace

#endif // _TPIE_STATS_TREE_H
