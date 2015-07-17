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

// Copyright (c) 1994 Darren Vengroff
//
// File: list_edge.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//
// $Id: list_edge.h,v 1.5 2004-08-12 12:36:45 jan Exp $
//
// The edge class.  This is what our list ranking function will work on.
//
#ifndef _LIST_EDGE_H
#define _LIST_EDGE_H

#include <portability.h>

class edge {
public:
    TPIE_OS_OFFSET from;        // Node it is from
    TPIE_OS_OFFSET to;          // Node it is to
    TPIE_OS_OFFSET weight;      // Position when ranked.
    bool flag;                  // A flag used to randomly select some edges.

    friend ostream& operator<<(ostream& s, const edge &e);
};    


// Helper functions used to compare to edges to sort them either by 
// the node they are from or the node they are to.

//extern int edgefromcmp(CONST edge &s, CONST edge &t);
//extern int edgetocmp(CONST edge &s, CONST edge &t);
//extern int edgeweightcmp(CONST edge &s, CONST edge &t);

struct edgefromcmp {
  int compare(CONST edge &s, CONST edge &t);
};
struct edgetocmp {
  int compare(CONST edge &s, CONST edge &t);
};
struct edgeweightcmp {
  int compare(CONST edge &s, CONST edge &t);
};
#endif // _LIST_EDGE_H 
