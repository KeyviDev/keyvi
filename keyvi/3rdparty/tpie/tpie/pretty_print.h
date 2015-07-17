// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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
/// \file tpie/pretty_print.h
///
/// pretty_class for formatting quantities with binary prefixes
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PRETTY_PRINT_H__
#define __TPIE_PRETTY_PRINT_H__

#include <algorithm>
#include <sstream>

namespace tpie {

namespace bits {

	class pretty_print {
		public:
			static std::string size_type(stream_size_type size) {
				static std::string units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
				stream_size_type i = 0;
				while(size > 1024 && i < 8) {
					size /= 1024;
					++i;
				}

				std::stringstream ss;
				ss << size << units[i];
				return ss.str();
		}
	};

};

};

#endif //__TPIE_PRETTY_PRINT_H__
