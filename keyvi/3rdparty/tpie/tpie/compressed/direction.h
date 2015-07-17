// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#ifndef TPIE_COMPRESSED_DIRECTION_H
#define TPIE_COMPRESSED_DIRECTION_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/direction.h  Read/write direction enumeration.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Class enum designating a read/write direction.
///////////////////////////////////////////////////////////////////////////////
struct read_direction {
	enum type {
		forward,
		backward
	};
};

} // namespace tpie

#endif // TPIE_COMPRESSED_DIRECTION_H
