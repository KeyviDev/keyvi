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

// A scan management object to write a stream of random integers.

// Get information on the configuration to test.
#include "app_config.h"

#include "scan_random.h"

scan_random::scan_random(TPIE_OS_OFFSET count, int seed) :
    m_max(count), m_remaining(count) {

    TP_LOG_APP_DEBUG("scan_random seed = ");
    TP_LOG_APP_DEBUG(static_cast<TPIE_OS_LONGLONG>(seed));
    TP_LOG_APP_DEBUG('\n');

    TPIE_OS_SRANDOM(seed);
}

scan_random::~scan_random(void) {

    //  No code in this destructor.
}


ami::err scan_random::initialize(void) {
    m_remaining = m_max;

    return ami::NO_ERROR;
};

ami::err scan_random::operate(int *out1, ami::SCAN_FLAG *sf) {
    if ((*sf = (m_remaining-- != 0))) {
        *out1 = TPIE_OS_RANDOM();
        return ami::SCAN_CONTINUE;
    } 
    else {
        return ami::SCAN_DONE;
    }
};

