// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2016, The TPIE development team
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
/// \file tpie/resources.h Defines all types of managed resources.
/// Currently only FILES and MEMORY.
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_RESOURCES_H__
#define __TPIE_RESOURCES_H__

#include <iostream>

namespace tpie {

enum resource_type {
	// These should be ordered by when the resource
	// assigned at runtime
	FILES,
	MEMORY,

	// Special values for internal use
	TOTAL_RESOURCE_TYPES,
	NO_RESOURCE
};

std::ostream & operator<<(std::ostream & os, const resource_type t);

} //namespace tpie

#endif //__TPIE_RESOURCES_H__
