// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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
/// \file tpie/stream_usage.h stream_usage enum
///////////////////////////////////////////////////////////////////////////
#ifndef __TPIE_STREAM_USAGE_H__
#define __TPIE_STREAM_USAGE_H__

namespace tpie {

	enum stream_usage {
	    /** Overhead of the object without the buffer */
	    STREAM_USAGE_OVERHEAD = 1,
	    /** Max amount ever used by a buffer */
	    STREAM_USAGE_BUFFER,
	    /** Amount currently in use. */
	    STREAM_USAGE_CURRENT,
	    /** Max amount that will ever be used. */
	    STREAM_USAGE_MAXIMUM,
	    /** Maximum additional amount used by each substream created. */
	    STREAM_USAGE_SUBSTREAM
	};

} // namespace tpie

#endif // __TPIE_STREAM_USAGE_H__
