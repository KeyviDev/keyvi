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

// -*- C++ -*-
//
//  Description:     declarations for Hilbert values
//  Created:         02.02.1998
//  Author:          Jan Vahrenhold
//  mail:            jan.vahrenhold@math.uni-muenster.de
//  $Id: hilbert.h,v 1.2 2004-02-05 17:53:53 jan Exp $
//  Copyright (C) 1998 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

#ifndef _TPIE_AMI_HILBERT_H
#define _TPIE_AMI_HILBERT_H

#include <tpie/portability.h>

#include <tpie/scan.h>

#include "rstarnode.h"

namespace tpie {

    namespace ami {

	////////////////////////////////////////////////////////////////////////////
        ///  Compare by increasing Hilbert value. Note that the order has to be
        ///  "reversed" as std::priority_queue sorts by "decreasing" order.
	////////////////////////////////////////////////////////////////////////////
	template<class coord_t, class BTECOLL>
	struct hilbert_priority {

	    ////////////////////////////////////////////////////////////////////////
	    ///  The larger Hilbert value has a higher priority.
	    ////////////////////////////////////////////////////////////////////////
	    inline bool operator()(
		const std::pair<rstarnode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>& t1, 
		const std::pair<rstarnode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>& t2) const {
		return t1.second > t2.second;
	    }
	};
	
    }  //  ami namespace

}  //  tpie namespace

namespace tpie {

    namespace ami {
	
	////////////////////////////////////////////////////////////////////////
	/// Compute the Hilbert value of (x,y) on an integer grid with side 
        /// length side. The code is taken from: \par
        ///  Jagadish, H.V.: "Linear Clustering of Objects with Multiple Attributes", 
        ///  in: Proceedings of the 1990 ACM SIGMOD International Conference on 
        ///  Management of Data (1990), 332-342.
	////////////////////////////////////////////////////////////////////////
	TPIE_OS_LONGLONG compute_hilbert_value(TPIE_OS_LONGLONG x, 
					       TPIE_OS_LONGLONG y, 
					       TPIE_OS_LONGLONG side);

    }  //  ami namespace

}  //   tpie namespace


namespace tpie {

    namespace ami {
	
	////////////////////////////////////////////////////////////////////////
        ///  Scan all data and compute the Hilbert value of each bounding box
        ///  w.r.t. a "size = 2**k" grid that encloses the data translated to the 
        ///  origin and scaled by "factor". The translation is determined by
        ///  "xOffset" and "yOffset".
	////////////////////////////////////////////////////////////////////////
	template<class coord_t>
	class scan_scale_and_compute_hilbert_value : public scan_object {

	private:
	    coord_t   xOffset_;
	    coord_t   yOffset_;
	    coord_t   factor_;
	    TPIE_OS_LONGLONG  side_;

	public:
	    
	    ////////////////////////////////////////////////////////////////////
	    /// Initialize the grid.
	    /// \param[in] xOffset left boundary of the grid
	    /// \param[in] yOffset lower boundary of the grid
	    /// \param[in] factor scaling factor to obtain integer coordinates
	    /// \param[in] side side length of the grid
	    ////////////////////////////////////////////////////////////////////
	    scan_scale_and_compute_hilbert_value(coord_t xOffset, coord_t yOffset, 
						 coord_t factor, TPIE_OS_LONGLONG side) :
		xOffset_(xOffset), yOffset_(yOffset), factor_(factor), side_(side) {};
	    
	    ////////////////////////////////////////////////////////////////////
	    ///  Nothing happens here
	    ////////////////////////////////////////////////////////////////////
	    err initialize() {
		return NO_ERROR;
	    }
	    
	    ////////////////////////////////////////////////////////////////////
	    /// Translate the rectangle by the given offset and
	    /// compute the midpoint in scaled integer coordinates.
	    /// \param[in] in input rectangle
	    /// \param[in] sfin scan flag
	    /// \param[out] out rectangle augmented by Hilbert value
	    /// \param[out] sfout scan flag
	    ////////////////////////////////////////////////////////////////////
	    err operate(const rectangle<coord_t, bid_t>& in, 
			SCAN_FLAG* sfin,
			std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>* out,
			SCAN_FLAG* sfout) {
		
		if ((*sfout = *sfin) != 0) {
		    
		    TPIE_OS_LONGLONG x = (TPIE_OS_LONGLONG)(factor_ * (TPIE_OS_LONGLONG)((in.get_left() + in.get_right()) / 2.0 - xOffset_));
		    TPIE_OS_LONGLONG y = (TPIE_OS_LONGLONG)(factor_ * (TPIE_OS_LONGLONG)((in.get_lower() + in.get_upper()) / 2.0 - yOffset_));
		    
		    *out = std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>(in, compute_hilbert_value(x, y, side_));
		    
		    return SCAN_CONTINUE;
		} 
		else {
		    return SCAN_DONE;
		}
	    }
	};
	
    }  //  ami namespace

}  //  tpie namespace


#endif
