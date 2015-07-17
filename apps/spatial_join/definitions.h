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

// Copyright (c) 2002 Octavian Procopiuc
//
// File:         definitions.h
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      06/11/97
// Description:  
//
// $Id: definitions.h,v 1.2 2005-11-10 10:35:57 adanner Exp $

#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H

#include <limits.h>
#include <float.h>	    // FreeBSD has DBL_MAX in float.h.

#ifndef MAX_NUMBER_OF_SLABS
#define MAX_NUMBER_OF_SLABS 20
#endif

#define STRIPED_SWEEP
#define SHORT_QUEUE

// #ifndef MIN
// #define MIN(a,b) ((a) <= (b) ? (a) : (b))
// #endif

// #ifndef MAX
// #define MAX(a,b) ((a) >= (b) ? (a) : (b))
// #endif

#define INT 1
#define FLOAT 2
#define DOUBLE 3

#define COORD_T INT

#if (COORD_T==INT)
#define TP_INFINITY      (INT_MAX-1)
#define MINUSINFINITY (1-INT_MAX)
typedef int           coord_t;
#elif (COORD_T==FLOAT)
#define TP_INFINITY (FLT_MAX-1.0)
#define MINUSINFINITY (1.0-FLT_MAX)
typedef float         coord_t;
#elif (COORD_T==DOUBLE)
#define TP_INFINITY (DBL_MAX-1.0)
#define MINUSINFINITY (1.0-DBL_MAX)
typedef double         coord_t;
#endif

typedef unsigned int  oid_t;
//const unsigned short  fanOut = 256;

#endif //_DEFINITIONS_H
