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

// BTE error codes, moved here from bte/base_stream.h
#ifndef _TPIE_BTE_ERR_H
#define _TPIE_BTE_ERR_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#ifdef TPIE_USE_EXCEPTIONS
#include <stdexcept>
#endif

namespace tpie {

    namespace bte {

	#ifdef TPIE_USE_EXCEPTIONS
	class out_of_space_error : public std::runtime_error {
		public:
			out_of_space_error(const std::string& what) 
			: std::runtime_error(what) 
			{ ; }
	};
	#endif 
		
	
//
// BTE error codes.
//
	enum err {
	    NO_ERROR = 0,
	    IO_ERROR,
	    END_OF_STREAM,
	    READ_ONLY,
	    OS_ERROR,
	    BASE_METHOD,
	    MEMORY_ERROR,
	    PERMISSION_DENIED,
	    OFFSET_OUT_OF_RANGE,
	    OUT_OF_SPACE,
	    STREAM_IS_SUBSTREAM,
	    WRITE_ONLY,
	    BAD_HEADER,
	    INVALID_PLACEHOLDER
	};
    
    }  //  bte namespace
    
}  //  tpie namespace

#endif // _TPIE_ERR_H
