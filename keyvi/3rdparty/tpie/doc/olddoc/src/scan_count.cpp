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

class scan_count : AMI_scan_object {
private:
    int maximum;
    int nextint;
public:
    scan_count(int max) : maximum(max), ii(0) {};

    AMI_err initialize(void) 
    {
        nextint = 0;
        return AMI_ERROR_NO_ERROR;
    };

    AMI_err operate(int *out1, AMI_SCAN_FLAG *sf)
    {
        *out1 = ++nextint;
        return (*sf = (nextint <= maximum)) ? AMI_SCAN_CONTINUE : 
            AMI_SCAN_DONE;
    };
};

scan_count sc(10000);
AMI_STREAM<int> amis0;    

void f()
{
    AMI_scan(&sc, &amis0);
}

AMI_err AMI_scan(scan_count &sc, AMI_STREAM<int> *pamis)
{
    int nextint;
    AMI_err ae;    
    AMI_SCAN_FLAG sf;

    sc.initialize();    
    while ((ae = sc.operate(&nextint, &sf)) == AMI_SCAN_CONTINUE) {
        if (sf) {
            Write nextint to *pamis;
        }
    }

    if (ae != AMI_SCAN_DONE) {
        Handle error conditions;
    }

    return AMI_ERROR_NO_ERROR;
}
