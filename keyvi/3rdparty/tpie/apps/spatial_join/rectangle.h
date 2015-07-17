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

// Copyright (c) 1997 Octavian Procopiuc
//
// File:         rectangle.h
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      03/22/97
// Description:  Classes rectangle and pair_of_rectangles
//
// $Id: rectangle.h,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include <string.h>
#include <iostream>
#include "definitions.h"

//
// Class rectangle:
// An abstraction of a 2-dim. rectangle.
//
class rectangle {
public:
  oid_t    id;
  coord_t xlo;
  coord_t ylo; 
  coord_t xhi;
  coord_t yhi;

  // Constructor.
  inline rectangle(const oid_t aid = 0, 
	    const coord_t axlo = (coord_t) 0, const coord_t aylo = (coord_t) 0,
	    const coord_t axhi = (coord_t) 0, const coord_t ayhi = (coord_t) 0);

  // Copy constructor.
  inline rectangle(const rectangle& r); 
  
  inline bool intersects(const rectangle &r) const;

  // Comparison operators. Comparison is done by ylo.
  inline bool operator<(const rectangle &rhs) const;
  inline bool operator>(const rectangle &rhs) const;

  // Not equal operator.
  inline bool operator!=(const rectangle &rhs) const;

  // Equality operator. All coordinates must be equal.
  inline bool operator==(const rectangle &rhs) const;

  // Assignment operator.
  inline rectangle& operator=(const rectangle& rhs);

  // The coordinates of the rectangle.
  inline coord_t left() const;
  inline coord_t right() const;
  inline coord_t lower() const;
  inline coord_t upper() const;

  //. The width and height and the area (as the product of width and 
  //. height) can be inquired. One can compute the "extended" area, 
  //. that is the bounding box of the union of the current object and
  //. another instance of BoundingBox, and the "overlap" area,
  //. that is the bounding box of the intersection of two bounding
  //. boxes.
  inline coord_t width() const;
  inline coord_t height() const;
  inline coord_t area() const;
  inline coord_t extendedArea(const rectangle& r) const;
  inline coord_t overlapArea(const rectangle& r) const;

  // Set the id to a new value.
  inline void setID(oid_t ID);

  // Inquire the id.
  inline oid_t getID() const;

  // Checks whether the projections on the x-axis of 
  // the 2 rectangles have a non-empty intersection.
  inline bool xOverlaps(const rectangle& r) const;

  // Checks whether the projections on the y-axis of 
  // the 2 rectangles have a non-empty intersection.  
  inline bool yOverlaps(const rectangle& r) const;

  // Extend the rectangle to include the given point.
  inline void extend(coord_t x, coord_t y);

  // Extend the rectangle to include the given rectangle.
  inline void extend(const rectangle& r);

};

// Output operator.
ostream& operator<<(ostream& s, const rectangle& r);
// Input operator.
istream& operator>>(istream& s, rectangle& r);

//
// class pair_of_rectangles:
// A pair of rectangles for reporting intersections.
//
class pair_of_rectangles {
public:
  oid_t first;		// id of first rectangle
  oid_t second;		// id of second rectangle

public:

  // Comparison operators. Comparison is done by 1st key, then 2nd key.
  inline bool operator<(const pair_of_rectangles &rhs) const;
  inline bool operator>(const pair_of_rectangles &rhs) const;

  // Equality operator.
  inline bool operator==(const pair_of_rectangles &rhs) const;
};


///////////////////// Definitions ////////////////////////

inline rectangle::rectangle(const oid_t aid, 
			    const coord_t axlo, const coord_t aylo, 
			    const coord_t axhi, const coord_t ayhi) {
//  : id(aid), xlo(axlo), ylo(aylo), xhi(axhi), yhi(ayhi) 
    id = aid;
    xlo = (axlo<axhi ? axlo : axhi);
    xhi = (axlo<axhi ? axhi : axlo);
    ylo = (aylo<ayhi ? aylo : ayhi);
    yhi = (aylo<ayhi ? ayhi : aylo);
}

inline rectangle::rectangle(const rectangle& r)
{
  *this = r;
}

inline bool rectangle::intersects(const rectangle &r) const
{
  if (((xlo <= r.xlo && r.xlo <= xhi) || 
       (r.xlo <= xlo && xlo <= r.xhi) 
    ) && ((ylo <= r.ylo && r.ylo <= yhi) || 
	  (r.ylo <= ylo && ylo <= r.yhi) )) {
    return true;
  } else {
    return false;
  }
}

inline bool rectangle::operator<(const rectangle &rhs) const 
{
  return  ylo < rhs.ylo;
}

inline bool rectangle::operator>(const rectangle &rhs) const 
{
  return ylo > rhs.ylo;
}

inline bool rectangle::operator==(const rectangle &rhs) const
{
  return ((id == rhs.id) && (xlo == rhs.xlo) && 
	  (ylo == rhs.ylo) && (xhi == rhs.xhi) && (yhi == rhs.yhi));
}

inline bool rectangle::operator!=(const rectangle &rhs) const
{
  return !(*this == rhs);
}

inline rectangle& rectangle::operator=(const rectangle& rhs)
{
  if (this != &rhs) {
    //    id  = rhs.id;
    //    xlo = rhs.xlo;
    //    ylo = rhs.ylo;
    //    xhi = rhs.xhi;
    //    yhi = rhs.yhi;
    memcpy((void*) this, &rhs, sizeof(rectangle));
    //  memcpy((void*) &id, &rhs.id, sizeof(id) + 4*sizeof(coord_t));
  }
  return (*this);
}

inline coord_t rectangle::left() const
{
  return xlo;
}

inline coord_t rectangle::right() const
{
  return xhi;
}

inline coord_t rectangle::lower() const
{
  return ylo;
}

inline coord_t rectangle::upper() const
{
  return yhi;
}

inline coord_t rectangle::width() const
{
  return (xhi - xlo);
}

inline coord_t rectangle::height() const 
{
  return (yhi - ylo);
}

inline coord_t rectangle::area() const
{
  return (xhi - xlo) * (yhi - ylo);
}

inline coord_t rectangle::extendedArea(const rectangle& r) const 
{
  return (((xhi > r.xhi ? xhi: r.xhi) - (xlo < r.xlo? xlo: r.xlo)) * ((yhi > r.yhi? yhi: r.yhi) - (ylo < r.ylo ? ylo: r.ylo)));
}

inline coord_t rectangle::overlapArea(const rectangle& r) const 
{
  return (intersects(r) ? (((xhi < r.xhi ? xhi: r.xhi) - (xlo > r.xlo ? xlo: r.xlo)) *
			 ((yhi < r.yhi ? yhi: r.yhi) - (ylo > r.ylo ? ylo: r.ylo))) : (coord_t) 0);
}

inline void rectangle::setID(oid_t ID)
{
  id = ID;
}

inline oid_t rectangle::getID() const
{
  return id;
}

inline bool rectangle::xOverlaps(const rectangle& r) const
{
  return ((xlo <= r.xlo && r.xlo <= xhi) ||
	  (r.xlo <= xlo && xlo <= r.xhi));
}

inline bool rectangle::yOverlaps(const rectangle& r) const
{
  return ((ylo <= r.ylo && r.ylo <= yhi) ||
	  (r.ylo <= ylo && ylo <= r.yhi));
}

inline void rectangle::extend(coord_t x, coord_t y)
{
  xlo = (x < xlo ? x: xlo); //min(x, xlo);
  ylo = (y < ylo ? y: ylo); //  min(y, ylo);
  xhi = (x > xhi ? x: xhi); //max(x, xhi);
  yhi = (y > yhi ? y: yhi); // max(y, yhi);
}

inline void rectangle::extend(const rectangle& r)
{
  xlo = (xlo < r.xlo ? xlo: r.xlo); //min(xlo, r.xlo);
  ylo = (ylo < r.ylo ? ylo: r.ylo); //min(ylo, r.ylo);
  xhi = (xhi > r.xhi ? xhi: r.xhi); //max(xhi, r.xhi);
  yhi = (yhi > r.yhi ? yhi: r.yhi); //max(yhi, r.yhi);
}


inline bool pair_of_rectangles::operator<(const pair_of_rectangles &rhs) const 
{
  return  (first == rhs.first) ? (second < rhs.second): (first < rhs.first);
}

inline bool pair_of_rectangles::operator>(const pair_of_rectangles &rhs) const 
{
  return (first == rhs.first) ? (second > rhs.second): (first > rhs.first);
}

inline bool pair_of_rectangles::operator==(const pair_of_rectangles &rhs) const 
{
  return (first == rhs.first && second == rhs.second);
}

#endif //_RECTANGLE_H_
