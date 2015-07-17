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

#include <tpie/portability.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

#include <tpie/sort.h>
#include <tpie/kb_sort.h>

// Utitlities for ascii output.
#include <tpie/scan_utils.h>

#include <tpie/cpu_timer.h>

#include <tpie/tpie.h>

using namespace tpie;

// This is the type of object we will sort.

union sort_obj
{
    ami::kb_key key_val;
    char filler[24];

    // How to extract the key for key bucket sorting.
    inline operator ami::kb_key(void) const
	{
	    return key_val;
	}
};



// A scan object to generate random keys.
class scan_random_so : ami::scan_object {

private:
    TPIE_OS_OFFSET m_max;
    TPIE_OS_OFFSET m_remaining;

public:
    scan_random_so(TPIE_OS_OFFSET count = 1000, int seed = 17);
    virtual ~scan_random_so(void);
    ami::err initialize(void);
    ami::err operate(sort_obj *out1, ami::SCAN_FLAG *sf);
};


scan_random_so::scan_random_so(TPIE_OS_OFFSET count, int seed) : 
    m_max(count), m_remaining(count) {

    TP_LOG_APP_DEBUG("scan_random_so seed = ");
    TP_LOG_APP_DEBUG(seed);
    TP_LOG_APP_DEBUG('\n');

    TPIE_OS_SRANDOM(seed);
}

scan_random_so::~scan_random_so(void)
{
}


ami::err scan_random_so::initialize(void)
{
    m_remaining = m_max;

    return ami::NO_ERROR;
};

ami::err scan_random_so::operate(sort_obj *out1, ami::SCAN_FLAG *sf)
{
    if ((*sf = (m_remaining-- >0))) {
        out1->key_val = TPIE_OS_RANDOM();
        return ami::SCAN_CONTINUE;
    } else {
        return ami::SCAN_DONE;
    }
};



static char def_srf[] = "oss.txt";
static char def_rrf[] = "osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

static bool kb_sort = false;

struct options app_opts[] = {
    { 10, "random-results-filename", "", "R", 1 },
    { 11, "report-results-random", "", "r", 0 },
    { 12, "sorted-results-filename", "", "S", 1 },
    { 13, "report-results-sorted", "", "s", 0 },
    { 14, "kb-sort", "", "k", 0 },
    { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg)
{
    switch (idx) {
    case 10:
	rand_results_filename = opt_arg;
    case 11:
	report_results_random = true;
	break;
    case 12:
	sorted_results_filename = opt_arg;
    case 13:
	report_results_sorted = true;
	break;
    case 14:
	kb_sort = !kb_sort;
	break;
    }
}

int main(int argc, char **argv) {

	tpie::tpie_init();
    
    ami::err ae;

    cpu_timer cput;
    
    parse_args(argc, argv, app_opts, parse_app_opts);

    if (verbose) {
	std::cout << "test_size = " << test_size << "." << std::endl;
	std::cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << std::endl;
	std::cout << "random_seed = " << random_seed << "." << std::endl;
    } else {
        std::cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed << ' ';
    }
    
    // Set the amount of main memory:
    get_memory_manager().set_limit (test_mm_size);
        
    ami::stream<sort_obj> amis0;
    ami::stream<sort_obj> amis1;
        
    // Write some ints.
    scan_random_so rnds(test_size,random_seed);
    
    ae = ami::scan(&rnds, &amis0);

    if (verbose) {
	std::cout << "Wrote the random values." << std::endl;
        std::cout << "Stream length = " << amis0.stream_len() << std::endl;
    }

    // Streams for reporting random vand/or sorted values to ascii
    // streams.
    
    std::ofstream *oss;
    ami::cxx_ostream_scan<sort_obj> *rpts = NULL;
    std::ofstream *osr;
    ami::cxx_ostream_scan<sort_obj> *rptr = NULL;
    
    if (report_results_random) {
        osr  = new std::ofstream(rand_results_filename);
        rptr = new ami::cxx_ostream_scan<sort_obj>(osr);
    }
    
    if (report_results_sorted) {
        oss  = new std::ofstream(sorted_results_filename);
        rpts = new ami::cxx_ostream_scan<sort_obj>(oss);
    }
    
    if (report_results_random) {
        ae = ami::scan(&amis0, rptr);
    }

    // Make the input stream read-once.

    amis0.persist(PERSIST_READ_ONCE);
    
    cput.reset();
    cput.start();

    if (kb_sort) {
	ami::key_range range(KEY_MIN, KEY_MAX);
        ae = ami::kb_sort(amis0, amis1, range);
    } else {
        ae = ami::sort(&amis0, &amis1);
    }

    cput.stop();

    amis0.persist(PERSIST_DELETE);
    
    if (verbose) {
	std::cout << "Sorted them." << std::endl;
        std::cout << "ae = " << ae << std::endl;
        std::cout << "Sorted stream length = " << amis1.stream_len() << std::endl;
    }

    std::cout << cput;
    
    if (report_results_sorted) {
        ae = ami::scan(&amis1, rpts);
    }
    std::cout << std::endl;

	tpie::tpie_finish();
    
    return 0;
}
