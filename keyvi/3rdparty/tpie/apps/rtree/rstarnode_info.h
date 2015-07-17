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

//  Prevent multiple includes.
#ifndef _TPIE_AMI_RSTARNODE_INFO_H
#define _TPIE_AMI_RSTARNODE_INFO_H

#include <tpie/portability.h>

#include <tpie/block.h>

namespace tpie {

    namespace ami {

	typedef TPIE_OS_SIZE_T children_count_t;

	////////////////////////////////////////////////////////////////////////////
        /// Metadata for a node in an R*-tree.
	////////////////////////////////////////////////////////////////////////////
	struct _rstarnode_info {
	    
	    ////////////////////////////////////////////////////////////////////////
            /// Block in which the parent of the node resides.
            ////////////////////////////////////////////////////////////////////////
	    bid_t            parent;

	    ////////////////////////////////////////////////////////////////////////
            /// Number of children stored in the node.
            ////////////////////////////////////////////////////////////////////////
	    children_count_t children;

	    ////////////////////////////////////////////////////////////////////////
            /// Flag indicating whether the node is the root, an internal node,
	    /// a leaf node, or a leaf.
            ////////////////////////////////////////////////////////////////////////
	    unsigned short   flag;
	};

    }  //  ami namespace

}  //  tpie namespace

#endif
