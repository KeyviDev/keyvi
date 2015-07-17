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
// File:         striped.h
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      01/24/99
// Description:  
//
// $Id: striped.h,v 1.2 2004-08-12 12:38:53 jan Exp $
//
#ifndef _STRIPED_H
#define _STRIPED_H

#include <portability.h>

#define MAX_STRIPS     128       /* max number of strips */

/***********************************/
/* Settings and internal constants */
/***********************************/

#define C_SIZE 32          /* number of rectangles per linked list element */


/***********************************/
/* Data structure for linked lists */
/***********************************/

typedef struct _ch {
  _ch *next; 
  TPIE_OS_OFFSET num;
  rectangle rects[C_SIZE];    /* array of rectangles */
} chunk;


#endif //_STRIPED_H
