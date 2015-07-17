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

#ifndef _TPIE_BTE_STREAM_BASE_GENERIC_H
#define _TPIE_BTE_STREAM_BASE_GENERIC_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// Get statistics definitions.
#include <tpie/stats_stream.h>

namespace tpie {

    namespace bte {
    
// A base class for the base class :). The role of this class is to
// provide global variables, accessible by all streams, regardless of
// template.
	class stream_base_generic {
	
	protected:
	    static stats_stream gstats_;
		static int current_streams;
	public:
	    // The number of globally available streams.
	    static size_t available_streams() { 
			return get_os_available_fds();
	    }
	
	    // The global stats.
	    static const stats_stream& gstats() { 
			return gstats_; 
	    }
	};

    }  //  bte namespace

}  //  tpie namespace

#endif // _TPIE_BTE_STREAM_BASE_GENERIC_H 
