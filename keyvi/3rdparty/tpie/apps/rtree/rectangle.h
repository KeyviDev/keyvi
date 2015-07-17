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
// File: rectangle.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 03/22/97
// Last Modified: 02/04/99
//
// $Id: rectangle.h,v 1.2 2004-02-05 17:54:14 jan Exp $
//
// rectangle and pair_of_rectangles classes for rectangle intersection
//


#ifndef _TPIE_AMI_RECTANGLE_H_
#define _TPIE_AMI_RECTANGLE_H_

#include "app_config.h"
#include <tpie/portability.h>
#include <tpie/stream.h>

#include <iostream>
// Include min and max from STL.
#include <algorithm>

namespace tpie {

    namespace ami {

	////////////////////////////////////////////////////////////////////////////
        /// An abstraction of a 2-dimensional rectangle for use with R*-trees.
	////////////////////////////////////////////////////////////////////////////
	template <typename coord_t, typename oid_t>
	class rectangle {
	public:

	    ///////////////////////////////////////////////////////////////////////
	    /// The default constructor initializes all values with zeros.
	    /// It is ensured that lower <= upper and left <= right.
	    ///////////////////////////////////////////////////////////////////////
	    rectangle(oid_t aid = (oid_t) 0, 
		      coord_t axlo = (coord_t) 0.0, 
		      coord_t aylo = (coord_t) 0.0,
		      coord_t axhi = (coord_t) 0.0, 
		      coord_t ayhi = (coord_t) 0.0): 
		id_(aid) {
		xlo_ = (axlo<axhi ? axlo : axhi);
		xhi_ = (axlo<axhi ? axhi : axlo);
		ylo_ = (aylo<ayhi ? aylo : ayhi);
		yhi_ = (aylo<ayhi ? ayhi : aylo);
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// The copy constructor copies all attributes.
	    ///////////////////////////////////////////////////////////////////////
	    rectangle(const rectangle<coord_t, oid_t>& r) {
		*this = r;  
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Comparison is done on the lower boundary (for plane-sweeping).
	    ///////////////////////////////////////////////////////////////////////
	    bool operator<(const rectangle<coord_t, oid_t> &rhs) const {
		return  get_lower() < rhs.get_lower();
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Comparison is done on the lower boundary (for plane-sweeping).
	    ///////////////////////////////////////////////////////////////////////
	    bool operator>(const rectangle<coord_t, oid_t> &rhs) const {
		return get_lower() > rhs.get_lower();
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Two rectangles are identical iff all coordinates and the id match.
	    ///////////////////////////////////////////////////////////////////////
	    bool operator==(const rectangle<coord_t, oid_t> &rhs) const {
		return ((get_id()    == rhs.get_id()) && 
			(get_left()  == rhs.get_left()) && 
			(get_lower() == rhs.get_lower()) && 
			(get_right() == rhs.get_right()) && 
			(get_upper() == rhs.get_upper()));
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Two rectangles are identical iff all coordinates and the id match.
	    ///////////////////////////////////////////////////////////////////////
	    bool operator!=(const rectangle<coord_t, oid_t> &rhs) const {
		return !(*this == rhs);
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Assignment is done by assigning all attributes.
	    ///////////////////////////////////////////////////////////////////////
	    rectangle<coord_t, oid_t>& operator=(const rectangle<coord_t, oid_t>& rhs) {
		if (this != &rhs) {
		    id_  = rhs.get_id();
		    xlo_ = rhs.get_left();
		    xhi_ = rhs.get_right();
		    ylo_ = rhs.get_lower();
		    yhi_ = rhs.get_upper();
		}
		return (*this);
	    }


	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the x-coordinate of the left boundary.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t get_left() const {
		return xlo_; 
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method sets the x-coordinate of the left boundary while
	    /// ensuring that left <= right.
	    /// \param[in] left the new x-coordinate of the left boundary
	    ///////////////////////////////////////////////////////////////////////
	    void set_left(coord_t left) {
		xlo_ = std::min(get_right(), left);
	    }
	    
	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the x-coordinate of the right boundary.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t get_right() const { 
		return xhi_; 
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method sets the x-coordinate of the right boundary while
	    /// ensuring that left <= right.
	    /// \param[in] right the new x-coordinate of the right boundary
	    ///////////////////////////////////////////////////////////////////////
	    void set_right(coord_t right) {
		xhi_ = std::max(get_left(), right);
	    }
	    
	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the y-coordinate of the lower boundary.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t get_lower() const { 
		return ylo_;
	    }
	    
	    ///////////////////////////////////////////////////////////////////////
	    /// This method sets the y-coordinate of the lower boundary while
	    /// ensuring that lower <= upper.
	    /// \param[in] lower the new y-coordinate of the lower boundary
	    ///////////////////////////////////////////////////////////////////////
	    void set_lower(coord_t lower) {
		ylo_ = std::min(get_upper(), lower);
	    }
	    
	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the y-coordinate of the upper boundary.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t get_upper() const { 
		return yhi_;
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method sets the y-coordinate of the upper boundary while
	    /// ensuring that lower <= upper.
	    /// \param[in] upper the new y-coordinate of the upper boundary
	    ///////////////////////////////////////////////////////////////////////
	    void set_upper(coord_t upper) {
		yhi_ = std::max(get_lower(), upper);
	    }
	    
	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the ID.
	    ///////////////////////////////////////////////////////////////////////
	    oid_t get_id() const { 
		return id_; 
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method sets the ID.
	    /// \param[in] id the new id
	    ///////////////////////////////////////////////////////////////////////
	    void set_id(oid_t id) { 
		id_ = id; 
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the width of the rectangle.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t width() const { 
		return (get_right() - get_left()); 
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the height of the rectangle.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t height() const { 
		return (get_upper() - get_lower()); 
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the area of the rectangle.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t area() const { 
		return height() * width();
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the area of the bounding box of the
	    /// rectangle and the given rectangle.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t extended_area(const rectangle<coord_t, oid_t>& r) const {
		return ((std::max(get_right(),r.get_right()) - 
			 std::min(get_left(),r.get_left())) * 
			(std::max(get_upper(),r.get_upper()) - 
			 std::min(get_lower(),r.get_lower())));
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// This method returns the area of the intersection of the
	    /// rectangle and the given rectangle.
	    ///////////////////////////////////////////////////////////////////////
	    coord_t overlap_area(const rectangle<coord_t, oid_t>& r) const {
		return (intersects(r) 
			? ((std::min(get_right(),r.get_right()) - 
			    std::max(get_left(),r.get_left())) * 
			   (std::min(get_upper(),r.get_upper()) - 
			    std::max(get_lower(),r.get_lower()))) 
			: (coord_t) 0.0);
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Checks whether the projections on the x-axis of the rectangle
	    /// and the given rectangle have a non-empty intersection.
	    ///////////////////////////////////////////////////////////////////////
	    bool x_overlaps(const rectangle<coord_t, oid_t>& r) const {
		return ((get_left() <= r.get_left() && r.get_left() <= get_right()) ||
			(r.get_left() <= get_left() && get_left() <= r.get_right()));
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Checks whether the projections on the y-axis of the rectangle
	    /// and the given rectangle have a non-empty intersection.
	    ///////////////////////////////////////////////////////////////////////
	    bool y_overlaps(const rectangle<coord_t, oid_t>& r) const {
		return ((get_lower() <= r.get_lower() && r.get_lower() <= get_upper()) ||
			(r.get_lower() <= get_lower() && get_lower() <= r.get_upper()));
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Checks whether the rectangle and the given rectangle have a 
	    /// non-empty intersection. The method returns false iff the rectangles
	    /// (considered as closed objects) are disjoint.
	    ///////////////////////////////////////////////////////////////////////
	    bool intersects(const rectangle<coord_t, oid_t>& r) const {
		return (x_overlaps(r) && y_overlaps(r));
	    }


	    ///////////////////////////////////////////////////////////////////////
	    /// Extend the rectangle to include the given point.
	    ///////////////////////////////////////////////////////////////////////
	    void extend(coord_t x, coord_t y) {
		set_left (std::min(x, get_left()));
		set_lower(std::min(y, get_lower()));
		set_right(std::max(x, get_right()));
		set_upper(std::max(y, get_upper()));
	    }

	    ///////////////////////////////////////////////////////////////////////
	    /// Extend the rectangle to include the given rectangle.
	    ///////////////////////////////////////////////////////////////////////
	    void extend(const rectangle<coord_t, oid_t>& r) {
		set_left (std::min(get_left(),  r.get_left()));
		set_lower(std::min(get_lower(), r.get_lower()));
		set_right(std::max(get_right(), r.get_right()));
		set_upper(std::max(get_upper(), r.get_upper()));
	    }

	private:
	    oid_t   id_;
	    coord_t xlo_;
	    coord_t ylo_; 
	    coord_t xhi_;
	    coord_t yhi_;

	};
	
// Output operator.
	template <class coord_t, class oid_t>
	std::ostream& operator<<(std::ostream& s, const rectangle<coord_t, oid_t>& r) {
	    return s << r.get_id() << " " << r.get_left() << " " << r.get_lower() << " " << r.get_right() << " " << r.get_upper() << "\n";
	}
	
// Input operator.
	template <class coord_t, class oid_t>
	std::istream& operator>>(std::istream& s, rectangle<coord_t, oid_t>& r) {
	    s.precision(7);
	    coord_t xlo, ylo, xhi, yhi;
	    oid_t id;
	    s >> xlo >> ylo >> xhi >> yhi >> id;
	    r = tpie::ami::rectangle<coord_t, oid_t>(id, xlo, ylo, xhi, yhi);
	    return s;
	}
	
    }  //  namespace ami

}  //  namespace tpie



namespace tpie {

    namespace ami {

	///////////////////////////////////////////////////////////////////////////
        /// A pair of rectangles for reporting intersections.
	///////////////////////////////////////////////////////////////////////////
	template<class oid_t> 
	class pair_of_rectangles {
	public:
	    ///////////////////////////////////////////////////////////////////////
	    /// ID of the first rectangle
	    ///////////////////////////////////////////////////////////////////////
	    oid_t first;

	    ///////////////////////////////////////////////////////////////////////
	    /// ID of the second rectangle
	    ///////////////////////////////////////////////////////////////////////
	    oid_t second;	

	};

    }  //  ami namespace

}  //  tpie namespace



#endif //_TPIE_AMI_RECTANGLE_H_
