// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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
#ifndef __tpie_file_accessor_file_accossor_h__
#define __tpie_file_accessor_file_accossor_h__

///////////////////////////////////////////////////////////////////////////////
/// \file file_accessor.h Declare default file accessor.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/file_accessor/stream_accessor.h>

#ifdef WIN32

#include <tpie/file_accessor/win32.h>
namespace tpie {
namespace file_accessor {
typedef win32 raw_file_accessor;
typedef stream_accessor_base<win32> file_accessor;
}
}

#else // WIN32

#include <tpie/file_accessor/posix.h>
namespace tpie {
namespace file_accessor {
typedef posix raw_file_accessor;
typedef stream_accessor_base<posix> file_accessor;
}
}

#endif // WIN32

namespace tpie {
typedef file_accessor::raw_file_accessor default_raw_file_accessor;
typedef file_accessor::stream_accessor<default_raw_file_accessor> default_file_accessor;
} // namespace tpie

#endif //__tpie_file_accessor_file_accossor_h__
