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
#ifndef TPIE_BACKTRACE
#define TPIE_BACKTRACE

#include <tpie/config.h>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
/// \file backtrace.h  Linux TPIE debugging helpers.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief Output a function call backtrace for debugging.
///
/// Does not support Windows. On Linux, uses cxxabi.h and execinfo.h to inspect
/// the stack at runtime.
///////////////////////////////////////////////////////////////////////////////
void backtrace(std::ostream & out, int depth=1024);

///////////////////////////////////////////////////////////////////////////////
/// \internal
/// \brief Do not use this directly; use the softassert() macro instead.
/// \sa softassert()
///////////////////////////////////////////////////////////////////////////////
void __softassert(const char * expr, const char * file, int line);

}

#ifndef TPIE_NDEBUG

///////////////////////////////////////////////////////////////////////////////
/// \internal
///////////////////////////////////////////////////////////////////////////////

#define softassert_str(x) #x

///////////////////////////////////////////////////////////////////////////////
/// \brief Soft assertion. If not x, report file and line number to the error
/// log.
///////////////////////////////////////////////////////////////////////////////

#define softassert(x) {if (!(x)) tpie::__softassert(softassert_str(x), __FILE__, __LINE__);}
#else
#define softassert(x)
#endif

#endif //TPIE_BACKTRACE
