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

// Copyright (c) 1999 Octavian Procopiuc
//
// File:         rectangle.cpp
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      02/04/99
// Description:  Definition of output operator for rectangle.
//
// $Id: rectangle.cpp,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
#include <iostream>
using std::ostream;
using std::istream;
using std::endl;
#include "rectangle.h"

ostream& operator<<(ostream& s, const rectangle& r)
{
  return s << r.id << " " << r.xlo << " " << r.ylo << " " 
	   << r.xhi << " " << r.yhi << endl;
}

istream& operator>>(istream& s, rectangle& r) {
#ifdef SEQUOIA_RECTANGLE 
  //sequoia rectangles have the id on the 1st position.
  s >> r.id >> r.xlo >> r.ylo >> r.xhi >> r.yhi;
#else
  s >> r.xlo >> r.ylo >> r.xhi >> r.yhi >> r.id;
#endif
  return s;
}
