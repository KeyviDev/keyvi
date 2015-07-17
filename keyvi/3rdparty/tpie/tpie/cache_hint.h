// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012 The TPIE development team
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
#ifndef __TPIE_CACHE_HINT_H__
#define __TPIE_CACHE_HINT_H__

///////////////////////////////////////////////////////////////////////////////
/// \file cache_hint.h  Different hints for OS file caching.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

enum cache_hint {
	/** Neither sequential access nor random access is intended.
	 * Corresponds to POSIX_FADV_NORMAL. */
	access_normal,

	/** Sequential access is intended. Default for file_stream.
	 * Corresponds to POSIX_FADV_SEQUENTIAL and FILE_FLAG_SEQUENTIAL_SCAN
	 * (Win32). */
	access_sequential,

	/** Random access is intended.
	 * Corresponds to POSIX_FADV_RANDOM and FILE_FLAG_RANDOM_ACCESS (Win32). */
	access_random
};

} // namespace tpie

#endif // __TPIE_CACHE_HINT_H__
