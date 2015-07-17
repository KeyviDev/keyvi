// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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
/// \file deprecated.h  Macros for deprecating classes, methods and typedefs.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_DEPRECATED_H__
#define __TPIE_DEPRECATED_H__

// The config knows whether or not to use deprecation warnings
#include <tpie/config.h>

#ifdef TPIE_DEPRECATED_WARNINGS
#ifdef __GNUC__
#define TPIE_DEPRECATED(func) func __attribute__ ((deprecated))
#define TPIE_DEPRECATED_CLASS_B
#define TPIE_DEPRECATED_CLASS_C __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define TPIE_DEPRECATED(func) __declspec(deprecated) func
#define TPIE_DEPRECATED_CLASS_B  __declspec(deprecated)
#define TPIE_DEPRECATED_CLASS_C
#else
#define TPIE_DEPRECATED(func) func
#define TPIE_DEPRECATED_CLASS_B 
#define TPIE_DEPRECATED_CLASS_C
#endif
#else
#define TPIE_DEPRECATED(func) func
#define TPIE_DEPRECATED_CLASS_B 
#define TPIE_DEPRECATED_CLASS_C
#endif

#define TPIE_DEPRECATED_CLASS_A(func) func TPIE_DEPRECATED_CLASS_C

#endif // __TPIE_DEPRECATED_H__
