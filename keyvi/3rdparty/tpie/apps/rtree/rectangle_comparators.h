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

#ifndef _TPIE_AMI_RECTANGLE_COMPARATORS_H
#define _TPIE_AMI_RECTANGLE_COMPARATORS_H

namespace tpie {

    namespace ami {

	////////////////////////////////////////////////////////////////////////////
        /// Comparator for sorting by lexicographical x-order.
	////////////////////////////////////////////////////////////////////////////
	template<class coord_t>
	struct sort_boxes_along_x_axis {

	    ////////////////////////////////////////////////////////////////////////
	    /// Sort by x-coordinate of left boundary, use x-coordinate of right
            /// boundary to break ties.
            ////////////////////////////////////////////////////////////////////////
	    inline bool operator()(const rectangle<coord_t, bid_t>& t1, 
				   const rectangle<coord_t, bid_t>& t2) const {
		if (t1.get_left() == t2.get_left()) {
		    return (t1.get_right() < t2.get_right());
		}
		else {
		    return (t1.get_left() < t2.get_left());
		}
	    }
	};
	
    }  //  ami namespace

}  //  tpie namespace

namespace tpie {

    namespace ami {

	////////////////////////////////////////////////////////////////////////////
        ///  Comparator for sorting by lexicographical y-order.
	////////////////////////////////////////////////////////////////////////////
	template<class coord_t>
	struct sort_boxes_along_y_axis {

	    ////////////////////////////////////////////////////////////////////////
	    /// Sort by y-coordinate of lower boundary, use y-coordinate of upper
            /// boundary to break ties.
            ////////////////////////////////////////////////////////////////////////
	    inline bool operator()(const rectangle<coord_t, bid_t>& t1, 
				   const rectangle<coord_t, bid_t>& t2) const {
		if (t1.get_lower() == t2.get_lower()) {
		    return (t1.get_upper() < t2.get_upper());
		}
		else {
		    return (t1.get_lower() < t2.get_lower());
		}
	    }
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

	////////////////////////////////////////////////////////////////////////////
        /// Comparator for sorting by a secondary key.
        /// Used in forced reinsertion with distance to center as the key (hence 
        /// the name).
	////////////////////////////////////////////////////////////////////////////
	template<class coord_t>
	struct sort_by_center_distance {

	    ////////////////////////////////////////////////////////////////////////
	    /// Sort by the second component of the pair.
	    ////////////////////////////////////////////////////////////////////////
	    bool operator()(const std::pair<TPIE_OS_SIZE_T, coord_t>& t1, 
			    const std::pair<TPIE_OS_SIZE_T, coord_t>& t2) {
		return (t1. second < t2.second);
	    }
	};

    }  //  ami namespace

}  //  tpie namespace


#endif
