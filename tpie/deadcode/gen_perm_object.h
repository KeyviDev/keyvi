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

#ifndef _TPIE_AMI_GEN_PERM_OBJECT_H
#define _TPIE_AMI_GEN_PERM_OBJECT_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// For AMI_err.
#include <tpie/err.h>

namespace tpie {

    namespace ami {

// A class of object that computes permutation destinations.
	class gen_perm_object {
	public:
	    virtual err initialize(TPIE_OS_OFFSET len) = 0;
	    virtual TPIE_OS_OFFSET destination(TPIE_OS_OFFSET src) = 0;
	    virtual ~gen_perm_object() {};
	};

    }  //  ami namespace

}  //  tpie namespace
 
#endif // _TPIE_AMI_GEN_PERM_OBJECT_H 
