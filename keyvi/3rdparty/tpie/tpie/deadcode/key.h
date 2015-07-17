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

///////////////////////////////////////////////////////////////////////////
/// File: ami_key.h
/// Defines keys and key ranges; used only in \ref kb_dist.h.
///////////////////////////////////////////////////////////////////////////
#ifndef _TPIE_AMI_KEY_H
#define _TPIE_AMI_KEY_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {
    
    namespace ami {
	
// CHECK THIS
// Temporary until the configuration script is edited to determine word
// size.
#define UINT32 unsigned long
	
	
/** Radix keys are unsigned 32 bit integers. */
	typedef UINT32 kb_key;
	
#define KEY_MAX 0x80000000
#define KEY_MIN 0
	
	///////////////////////////////////////////////////////////////////////////
	/// A range of keys.  A stream having this range of keys is guaranteed
	/// to have no keys < min and no keys >= max.
	///////////////////////////////////////////////////////////////////////////
	class key_range {
	public:
	    key_range();
	    key_range(kb_key min_key, kb_key max_key);
	    
	    kb_key get_min() const { 
		return m_min; 
	    }
	    
	    void put_min(kb_key min_key) {
		m_min = min_key;
	    }
	    
	    kb_key get_max() const { 
		return m_max; 
	    }
	    
	    void put_max(kb_key max_key) {
		m_max = max_key;
	    }
	    
	private:
	    kb_key m_min;
	    kb_key m_max;
	};
	

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_KEY_H 
