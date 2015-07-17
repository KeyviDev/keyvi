// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=8 sts=4 sw=4 noet :
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

// A test for AMI_sort().

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <cstdlib>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <tpie/stream.h>
#include <tpie/scan.h>
#include <tpie/sort.h>
#include <tpie/cpu_timer.h>
#include <tpie/tpie.h>

// Utilities for ascii output.
#include <tpie/scan_utils.h>

#include "scan_random.h"
#include "scan_diff.h" 
#include "merge_random.h"

using namespace tpie;

enum comparison_mode_t {
    COMPARISON_OPERATOR,
    COMPARISON_CLASS
};

static const memory_size_type sort_test_mm_size = 1024*1024*1024; // 1 GB

static char def_srf[] = "sorted.txt";
static char def_rrf[] = "unsorted.txt";

static char istr_name[128]; 
static char ostr_name[128];

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

static bool sort_again = false;
static comparison_mode_t comparison_mode = COMPARISON_OPERATOR;


// The command line options for this application.
struct options app_opts[] = {
    { 10, "input-stream", "Read items from given stream", "i", 1 },
    { 11, "output-sream", "Write sorted items into given stream", "o", 1 },
    { 12, "write-input-ascii", "Write unsorted items as plain text into the given file", NULL, 1 },
    { 13, "write-output-ascii", "Write sorted items as plain text into the given file", NULL, 1 },
    { 14, "comparison", "Comparison device: [o]perator or [c]lass", "c", 1 },
    { 15, "again", "Sort again with different routine (AMI_sort_V1)", "a", 0 },
    { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg) {
    switch (idx) {
    case 10:
	strncpy(istr_name, opt_arg, 128);
	break;
    case 11:
	strncpy(ostr_name, opt_arg, 128);
	break;
    case 12:
	rand_results_filename = opt_arg;
	//  case 'r':
	report_results_random = true;
	break;
    case 13:
	sorted_results_filename = opt_arg;
	//  case 's':
	report_results_sorted = true;
	break;
    case 15:
	sort_again = true;
	break;
    case 14:
	switch (opt_arg[0]) {
	case 'o': case 'O':
	    comparison_mode = COMPARISON_OPERATOR;
	    break;
	case 'c': case 'C':
	    comparison_mode = COMPARISON_CLASS;
	    break;
	default:
	    std::cerr << "Invalid comparison device. Valid options are [o]perator and [c]lass." << std::endl;
	    exit(1);
	}
	break;

    }
}


class int_cmp_class {
public:
    int compare(CONST int &i1, CONST int &i2) {
	return i1 - i2;
    }
};

int main(int argc, char **argv)  {

    int_cmp_class int_cmp_obj;
    cpu_timer timer;
    ami::err ae = ami::NO_ERROR;

    bool random_input;

	tpie_init();

    test_size = 0;
    istr_name[0] = ostr_name[0] = '\0';

    parse_args(argc, argv, app_opts, parse_app_opts);

    if (test_size == 0 && istr_name[0] == '\0') {
	std::cerr << argv[0] << ": No input size or input file specified. Use -h for help." << std::endl;
	exit(1);
    }

    TP_LOG_APP_DEBUG_ID("Boo");

    random_input = (istr_name[0] == '\0');

    // Set the amount of main memory:
    get_memory_manager().set_limit (sort_test_mm_size);

	tempname::set_default_base_name("TEST_AMI_SORT");
    ami::stream<int>* istr = (istr_name[0] == '\0') ? tpie_new<ami::stream<int> >(): tpie_new<ami::stream<int> >(istr_name);
    if (!istr->is_valid()) {
	std::cerr << argv[0] << ": Error while initializing input stream. Aborting." << std::endl;
	exit(2);
    }
    ami::stream<int>* ostr = NULL;

    if (verbose) {
	std::cout << "BTE: ";
#ifdef BTE_STREAM_IMP_MMAP
	std::cout << "BTE_STREAM_IMP_MMAP " << BTE_STREAM_MMAP_BLOCK_FACTOR;
#endif
#ifdef BTE_STREAM_IMP_STDIO
	std::cout << "BTE_STREAM_IMP_STDIO ";
#endif
#ifdef BTE_STREAM_IMP_UFS
	std::cout << "BTE_STREAM_IMP_UFS " << STREAM_UFS_BLOCK_FACTOR;
#endif
#ifdef BTE_MMB_READ_AHEAD
	std::cout << " BTE_MMB_READ_AHEAD ";	  
#endif
	std::cout << std::endl;
	std::cout << "Comparison device: " 
		  << (comparison_mode == COMPARISON_OPERATOR ? "Operator": "Class")
		  << "." << std::endl;
	std::cout << "Input size: " << test_size << " items." << std::endl
		  << "Item size: " << sizeof(int) << " bytes." << std::endl
		  << "TPIE memory size: " 
			  << static_cast<TPIE_OS_LONGLONG>(get_memory_manager().limit())
		  << " bytes." << std::endl;
	std::cout << "TPIE free memory: " 
			  << static_cast<TPIE_OS_LONGLONG>(get_memory_manager().available())
		  << " Bytes2." << std::endl;
    }

    if (random_input) {
	// Write some ints.
	std::cout << "Generating input (" << test_size << " random integers)..." << std::flush;
	timer.start();
	scan_random rnds(test_size,random_seed);
	ae = ami::scan(&rnds, istr);
	timer.stop();
	std::cout << "Done." << std::endl;
	if (ae != ami::NO_ERROR) {
	    std::cerr << argv[0] << ": Error while generating input. Aborting." << std::endl;
	    exit(2);
	} else {
	    if (verbose) {
		std::cout << "Input stream length: " << istr->stream_len() << "\n";
	    }
	}
	if (verbose) {
	    std::cout << "Time taken: " << timer << std::endl;
	}
	timer.reset();
    } else {
	test_size = istr->stream_len();
    }

    // Streams for reporting random and/or sorted values to ascii
    // streams.    
    std::ofstream *oss;
    ami::cxx_ostream_scan<int> *rpts = NULL;
    std::ofstream *osr;
    ami::cxx_ostream_scan<int> *rptr = NULL;
  
    if (report_results_random) {
		osr  = tpie_new<std::ofstream>(rand_results_filename);
		rptr = tpie_new<ami::cxx_ostream_scan<int> >(osr);
    }
  
    if (report_results_sorted) {
		oss  = tpie_new<std::ofstream>(sorted_results_filename);
		rpts = tpie_new<ami::cxx_ostream_scan<int> >(oss);
    }
  
    if (report_results_random) {
	std::cout << "Writing input in ASCII file " << rand_results_filename << " ..." << std::flush;
	ae = ami::scan(istr, rptr);
	std::cout << "Done." << std::endl;
	if (ae != ami::NO_ERROR) {
	    std::cerr << argv[0] << ": Error while writing input ASCII file." << std::endl;
	}
    }

    if (verbose) {
	std::cout << "TPIE free memory: " 
			  << static_cast<TPIE_OS_LONGLONG>(get_memory_manager().available())
		  << " bytes.\n";
    }
    std::cout << "Sorting input..." << std::flush;
    timer.start();  
    ostr = (ostr_name[0] == '\0') ? tpie_new<ami::stream<int> >(): tpie_new<ami::stream<int> >(ostr_name); 
    if (comparison_mode == COMPARISON_OPERATOR) {
	ae = ami::sort(istr, ostr);
    } else if (comparison_mode == COMPARISON_CLASS) {
	ae = ami::sort(istr, ostr, &int_cmp_obj);
    }
    timer.stop();
    std::cout << "Done." << std::endl;
    if (ae != ami::NO_ERROR) {
	std::cerr << argv[0] << ": Error during sort (check the log). Aborting." << std::endl;
	exit(3);
    }
    if (verbose) {
	std::cout << "Sorted stream length: " << ostr->stream_len() << std::endl;
	std::cout << "Time taken: " << timer << std::endl;
    }
    timer.reset();

    if (verbose) {
	std::cout << "TPIE free memory: " 
			  << static_cast<TPIE_OS_LONGLONG>(get_memory_manager().available()) 
		  << " bytes." << std::endl;
    }
    if (report_results_sorted) {
	std::cout << "Writing sorted items in ASCII file " 
		  << sorted_results_filename << " ..." << std::flush;
	ae = ami::scan(ostr, rpts);
	std::cout << "Done.\n";
	if (ae != ami::NO_ERROR) {
	    std::cerr << argv[0] << ": Error during writing of sorted ASCII file." << std::endl;
	}
	if (verbose) {
	    std::cout << "TPIE free memory: " << 
			static_cast<TPIE_OS_LONGLONG>(get_memory_manager().available())
		      << " bytes." << std::endl;
	}
    }
  
    if (sort_again) {
    
	ami::stream<int> amis2;
	ami::stream<int> amis3;
	ami::stream<scan_diff_out<int> > amisd;
    
	merge_random<int> mr;
	scan_diff<int> sd(-1);
    
	std::cout << "Sorting again using old sorting routine." << std::endl;
	if (verbose) {
	    std::cout << "TPIE free memory: " 
				  << static_cast<TPIE_OS_LONGLONG>(get_memory_manager().available()) 
		      << " bytes." << std::endl;
	}
    
	std::cout << "Sorting input..." << std::flush;
	timer.start();  
	if (comparison_mode == COMPARISON_OPERATOR) {
	    ae = ami::sort(istr, &amis3);
	} else if (comparison_mode == COMPARISON_CLASS) {
	    ae = ami::sort(istr, &amis3, &int_cmp_obj);
	}
	timer.stop();
	std::cout << "Done." << std::endl;

	if (ae != ami::NO_ERROR) {
	    std::cerr << argv[0] << "Error during sort (check the log). Aborting." << std::endl;
	    exit(3);
	}
	std::cout << "Sorted stream length: " << amis3.stream_len() << std::endl;
	if (verbose) {
	    std::cout << "Time taken: " << timer << std::endl;
	    std::cout << "TPIE free memory: " 
				  << static_cast<TPIE_OS_LONGLONG>(get_memory_manager().available()) 
		      << " bytes.\n";
	}

	ae = ami::scan(ostr, &amis3, &sd, &amisd);
    
	std::cout << "Length of diff stream: " << amisd.stream_len() << "." << std::endl;
    }

    tpie_delete(istr);
    tpie_delete(ostr);
    return 0;
}
