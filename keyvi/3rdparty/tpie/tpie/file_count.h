// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, 2012, The TPIE development team
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
#ifndef __TPIE_FILE_CONUT_H__
#define __TPIE_FILE_CONUT_H__
#include <tpie/types.h>
#include <tpie/deprecated.h>
////////////////////////////////////////////////////////////////////////////////
/// \file file_count.h
/// \brief Count the number of open files.
////////////////////////////////////////////////////////////////////////////////

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief Deprecated; does nothing.
///////////////////////////////////////////////////////////////////////////////
TPIE_DEPRECATED(inline void increment_open_file_count()) {
	// Does nothing.
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Deprecated; does nothing.
///////////////////////////////////////////////////////////////////////////////
TPIE_DEPRECATED(inline void decrement_open_file_count()) {
	// Does nothing.
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Return the current number of open files.
/// \return The current number of open files.
////////////////////////////////////////////////////////////////////////////////
memory_size_type open_file_count();

////////////////////////////////////////////////////////////////////////////////
/// \brief Return the additional number of files that can be opened before
/// running out of file descriptors.
////////////////////////////////////////////////////////////////////////////////
memory_size_type available_files();
}
#endif //__TPIE_FILE_CONUT_H__
