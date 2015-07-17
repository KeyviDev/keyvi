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

// Test for AMI_BMMC_permute(). See the Tutorial for an explanation of 
// this particular example.

#include <portability.h>




// Get the application defaults.
#include "app_config.h"


#include <scan.h>
#include <bit_permute.h>

#include <scan_utils.h>

#include "parse_args.h"
#include "scan_count.h"


static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char *initial_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_initial = false;
static bool report_results_final = false;

struct options app_opts[] = {
  { 12, "initial-results-filename", "", "I", 1 },
  { 13, "report-results-initial", "", "i", 0 },
  { 14, "final-results-filename", "", "F", 1 },
  { 15, "report-results-final", "", "f", 0 },
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg)
{
    switch (idx) {
        case 12:
            initial_results_filename = opt_arg;
        case 13:
            report_results_initial = true;
            break;
        case 14:
            final_results_filename = opt_arg;
        case 15:
            report_results_final = true;
            break;
    }
}


int main(int argc, char **argv)
{
    AMI_err ae;
    TPIE_OS_SIZE_T number_of_bits;

	verbose = false;
	test_size = 32 * 1024;

    parse_args(argc, argv, app_opts, parse_app_opts);
    
    // Count the bits in test_size.
    for (number_of_bits = 0 ; test_size >= 2; number_of_bits++)
      test_size = test_size >> 1;
    if (number_of_bits == 0)
      number_of_bits = 1;

    // Adjust the test size to be a power of two.
    test_size = 1 << number_of_bits;
    
    if (verbose) {
      std::cout << "test_size = " << test_size << "." << std::endl;
      std::cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << std::endl;
      std::cout << "random_seed = " << random_seed << "." << std::endl;
    } else {
        std::cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }

    TPIE_OS_SRANDOM(random_seed);
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);

    AMI_STREAM<TPIE_OS_OFFSET> amis0;
    AMI_STREAM<TPIE_OS_OFFSET> amis1;

    // Streams for reporting values to ascii streams.
    
    std::ofstream *osi;
    cxx_ostream_scan<TPIE_OS_OFFSET> *rpti = NULL;

    std::ofstream *osf;
    cxx_ostream_scan<TPIE_OS_OFFSET> *rptf = NULL;

    if (report_results_initial) {
        osi = new std::ofstream(initial_results_filename);
        rpti = new cxx_ostream_scan<TPIE_OS_OFFSET>(osi);
    }

    if (report_results_final) {
        osf = new std::ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<TPIE_OS_OFFSET>(osf);
    }

    scan_count my_scan_count(test_size);

    ae = AMI_scan(&my_scan_count, &amis0);

    if (verbose) {
        std::cout << "Initial stream length = " << amis0.stream_len() << std::endl;
    }
    
    if (report_results_initial) {
        ae = AMI_scan(&amis0, rpti);
    }

    amis0.seek(0);

    bit_matrix A(number_of_bits, number_of_bits);
    bit_matrix c(number_of_bits, 1);

    {
        TPIE_OS_SIZE_T ii,jj;

        for (ii = number_of_bits; ii--; ) {
            c[ii][0] = 0;
            for (jj = number_of_bits; jj--; ) {
                A[number_of_bits-1-ii][jj] = (ii == jj);
            }
        }
    }

    if (verbose) {
        std::cout << "A = " << A << std::endl;
        std::cout << "c = " << c << std::endl;
    }
    
    AMI_bit_perm_object bpo(A, c);
    
    ae = AMI_BMMC_permute(&amis0, &amis1, &bpo);

    if (verbose) {
        std::cout << "After permutation, stream length = " 
	     << amis1.stream_len();
    }

    std::cout << std::endl;

    if (report_results_final) {
        ae = AMI_scan(&amis1, rptf);
    }

    return 0;
}
