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

////////////////////////////////////////////////////////////////////////////////
/// \file stats_stream.h Status information tags for AMI streams.
/// \sa tpie_stats, stream#stats()
////////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_STATS_STREAM_H
#define _TPIE_STATS_STREAM_H

#include <tpie/stats.h>

namespace tpie {
    
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Status information about a TPIE stream.
    ///////////////////////////////////////////////////////////////////////////
    enum stats_stream_id {
	/** Number of block reads */
	BLOCK_READ = 0,
	/** Number of block writes */
	BLOCK_WRITE,
	/** Number of item reads */
	ITEM_READ,
	/** Number of item writes */ 
	ITEM_WRITE,
	/** Number of item seek operations */ 
	ITEM_SEEK,
	/** Number of stream open operations */ 
	STREAM_OPEN,
	/** Number of stream close operations */ 
	STREAM_CLOSE,
	/** Number of stream create operations */ 
	STREAM_CREATE,
	/** Number of stream delete operations */ 
	STREAM_DELETE,
	/** Number of substream create operations */ 
	SUBSTREAM_CREATE,
	/** Number of substream delete operations */ 
	SUBSTREAM_DELETE,
	NUMBER_OF_STREAM_STATISTICS
    };
    
    ///////////////////////////////////////////////////////////////////////////
    /// Encapsulates statistics about a TPIE stream.
    ///////////////////////////////////////////////////////////////////////////
    typedef stats<NUMBER_OF_STREAM_STATISTICS> stats_stream;

}  //  tpie namespace

#endif //_TPIE_STATS_STREAM_H
