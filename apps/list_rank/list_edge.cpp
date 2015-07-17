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
// File: list_edge.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//

#include <versions.h>
VERSION(list_edge_cpp,"$Id: list_edge.cpp,v 1.4 2003-06-03 16:56:24 tavi Exp $");

// A hack for now until const handling improves.
#define CONST const

#include "list_edge.h"

// An output operator for edges.

ostream& operator<<(ostream& s, const edge &e)
{
    s << e.from << " -> " << e.to << " (" << e.weight << ") ";
    s << '[' << e.flag << ']';
    return s;
}

// Helper functions used to compare to edges to sort them either by 
// the node they are from or the node they are to.

int edgefromcmp::compare(CONST edge &s, CONST edge &t)
{
    return (s.from < t.from) ? -1 : ((s.from > t.from) ? 1 : 0);
}
  
int edgetocmp::compare(CONST edge &s, CONST edge &t)
{
    return (s.to < t.to) ? -1 : ((s.to > t.to) ? 1 : 0);
}

int edgeweightcmp::compare(CONST edge &s, CONST edge &t)
{
    return (s.weight < t.weight) ? -1 : ((s.weight > t.weight) ? 1 : 0);
}
