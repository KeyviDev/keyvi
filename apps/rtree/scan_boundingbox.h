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

#ifndef _TPIE_AMI_SCAN_BOUNDINGBOX_H
#define _TPIE_AMI_SCAN_BOUNDINGBOX_H

#include <tpie/portability.h>
#include <tpie/scan.h>
#include <utility>

#include "rectangle.h"

namespace tpie {

    namespace ami {
	
	///////////////////////////////////////////////////////////////////////////
	/// Scan all rectangles in the input stream and compute the mininum 
        /// bounding box.
        ///////////////////////////////////////////////////////////////////////////
	template<class coord_t, class bid_t>
	class scan_computeBoundingBox : public scan_object {
	    
#define MAGIC_NUMBER_UNINITIALIZED_RECTANGLE (bid_t)17
	    
	private:
	    rectangle<coord_t, bid_t>** mbr_;
	    
	public:
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// The constructor expects a mutable pointer to a rectangle. 
	    /// \param mbr pointer to the rectangle representing the minimum bounding box
	    ///////////////////////////////////////////////////////////////////////////
	    scan_computeBoundingBox(rectangle<coord_t, bid_t>** mbr) :
		mbr_(mbr) {
		//  Do nothing.
	    };
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// This method initalizes the minimum bounding rectangle.
            /// If the pointer is null, a new rectangle is created (make sure to
	    /// delete it afterwards).
	    ///////////////////////////////////////////////////////////////////////////
	    err initialize() {

		assert(mbr_ != NULL);

		if (*mbr_ == NULL) {
		    *mbr_ = new rectangle<coord_t, bid_t>();
		}

		assert(MAGIC_NUMBER_UNINITIALIZED_RECTANGLE != 0);
		// ...otherwise the hack won't work.

		(*mbr_)->set_id(MAGIC_NUMBER_UNINITIALIZED_RECTANGLE);

		return NO_ERROR;
	    };
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// The current minimum bounding rectangle is extended to enclose the
	    /// rectangle passed to this method.
	    /// \param[in] in current rectangle read from input stream
	    /// \param[in] sfin (scan flag)
	    /// \param[out] sfout (scan flag)
	    ///////////////////////////////////////////////////////////////////////////
	    err operate(const rectangle<coord_t, bid_t>& in, 
			SCAN_FLAG* sfin,
			std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>* /* out */, 
			SCAN_FLAG* sfout) {
		
		//  Write nothing to the output stream.
		*sfout = false;
		if (*sfin) {
		    if ((*mbr_)->get_id() == MAGIC_NUMBER_UNINITIALIZED_RECTANGLE) {
			(*mbr_)->set_left (in.get_left());
			(*mbr_)->set_right(in.get_right());
			(*mbr_)->set_lower(in.get_lower());
			(*mbr_)->set_upper(in.get_upper());

			// ...un-hack.
			(*mbr_)->set_id(0);
		    }
		    else {
			(*mbr_)->extend(in);
		    }

		    return SCAN_CONTINUE;
		} 
		else {
		    return SCAN_DONE;
		}
	    };
	};

    }  //  ami namespace

}  //  tpie namespace


#endif
