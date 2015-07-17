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

// Include the file that will allow us to use the indicator.
#include <progress_indicator_arrow.h>

int main(int argc, char *argv[]) { 
    
    int upper = 32*1024;

    //  Count from 0 to upper in units of 5 steps.
    progress_indicator_arrow* indicator = new 
	progress_indicator_arrow("Title of an indicator", "Tests so far:",
				    0, upper, 5);

    for (int i=0; i< upper; i++) {
	//  Advance on every fifth step.
	if (!(i % 5)) {
	    indicator->step();
	}
    }
    indicator->done("Done.");

    upper = upper * 1024;
    indicator->set_title("Checking the percentage-based indicator");

    indicator->set_description("Pass 1/3:");
    //  Update the display with every 1.0% of progress
    indicator->set_percentage_range(0,upper);
    indicator->init();
    for (int i=0; i< upper; i++) {
	indicator->step_percentage();
    }

    indicator->set_description("Pass 2/3:");
    //  Update the display with every 0.1% of progress
    indicator->set_percentage_range(0,upper,1000);
    indicator->init();
    for (int i=0; i< upper; i++) {
	indicator->step_percentage();
    }

    indicator->set_description("Pass 3/3:");
    //  Update the display with every 5.0% of progress
    indicator->set_percentage_range(0,upper,20);
    indicator->init();
    for (int i=0; i< upper; i++) {
	indicator->step_percentage();
    }

    indicator->done("Done as well.");
    
    delete indicator;

    return 0;
}
