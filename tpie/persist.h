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

// Persistence flags for TPIE streams.
#ifndef _TPIE_PERSIST_H
#define _TPIE_PERSIST_H

///////////////////////////////////////////////////////////////////////////////
/// \file persist.h Persistence tags for deprecated TPIE AMI streams.
///////////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {

	///////////////////////////////////////////////////////////////////////////
	/// Declares for an AMI stream under which circumstances it should be
	/// deleted.
	///////////////////////////////////////////////////////////////////////////
    enum TPIE_DEPRECATED_CLASS_B persistence {
	/** Delete the stream from the disk when it is destructed. */
	PERSIST_DELETE = 0,
	/** Do not delete the stream from the disk when it is destructed. */
	PERSIST_PERSISTENT = 1,
	/** Delete each block of data from the disk as it is read.
	 * If not supported by the OS (see portability.h), delete
	 * the stream when it is destructed (see PERSIST_DELETE). */
	PERSIST_READ_ONCE = 0
    } TPIE_DEPRECATED_CLASS_C;

}  //  tpie namespace 

#endif // _TPIE_PERSIST_H 
