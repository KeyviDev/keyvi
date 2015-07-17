// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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

///////////////////////////////////////////////////////////////////////////////
/// \file unused.h  Default special unused values for standard types.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_UNUSED_H__
#define __TPIE_UNUSED_H__

#include <limits>
#include <utility>

namespace tpie {

template <typename T>
struct default_unused {
	inline static T v() {return std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity(): std::numeric_limits<T>::max();}	
};

template <typename T1, typename T2>
struct default_unused<std::pair<T1, T2> > {
	inline static std::pair<T1,T2> v() {
		return std::pair<T1, T2>(default_unused<T1>::v(), default_unused<T2>::v());
	}
};

} // namespace tpie

#endif //__TPIE_UNUSED_H__
