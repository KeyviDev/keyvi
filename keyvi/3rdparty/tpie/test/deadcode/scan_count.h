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

#ifndef _SCAN_COUNT_H
#define _SCAN_COUNT_H

#include <tpie/portability.h>
#include <tpie/scan.h>

using namespace tpie;

class scan_count : ami::scan_object {
    
private:
    TPIE_OS_OFFSET maximum;
    
public:
    TPIE_OS_OFFSET ii;
    TPIE_OS_OFFSET called;
    
    scan_count(TPIE_OS_OFFSET max = 1000);
    ami::err initialize(void);
    ami::err operate(TPIE_OS_OFFSET *out1, ami::SCAN_FLAG *sf);
};


#endif // _SCAN_COUNT_H 
