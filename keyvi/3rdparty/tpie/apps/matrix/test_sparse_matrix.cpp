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

// Copyright (c) 1995 Darren Vengroff
//
// File: test_ami_sm.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/2/95
//

#include <tpie/tpie.h>

#include <tpie/portability.h>

#include "app_config.h"        
#include "parse_args.h"

// Define it all.
#include <tpie/scan.h>

// Utitlities for ascii output.
#include <tpie/scan_utils.h>

// Get matrices.

#include "matrix.h"
#include "matrix_fill.h"
#include "fill_value.h"

#include "sparse_matrix.h"

#include "scan_uniform_sm.h"

#include <tpie/cpu_timer.h>

using namespace tpie;

static char def_crf[] = "osc.txt";
static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char def_brf[] = "isb.txt";

static char *count_results_filename = def_crf;
static char *intermediate_results_filename = def_irf;
static char *final_results_filename = def_frf;

static char *banded_read_filename = def_brf;

static bool report_results_count = false;
static bool report_results_intermediate = false;
static bool report_results_final = false;

static bool read_banded_matrix = false;

static bool call_mult = false;

static double density = 0.10;

struct options app_opts[] = {
    { 10, "count-results-filename", "", "C", 1 },
    { 11, "report-results-count", "", "c", 0 },
    { 12, "intermediate-results-filename", "", "I", 1 },
    { 13, "report-results-intermediate", "", "i", 0 },
    { 14, "final-results-filename", "", "F", 1 },
    { 15, "report-results-final", "", "f", 0 },
    { 16, "density", "", "d", 1 },
    { 17, "call-multiply-directly", "", "D", 0 },
    { 20, "banded-read-filename", "", "B", 1 },
    { 21, "read_banded_matrix", "", "b", 0 },
    { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg)
{
    switch (idx) {
    case 20:
	banded_read_filename = opt_arg;
    case 21:
	read_banded_matrix = true;
	break;
    case 10:
	count_results_filename = opt_arg;
    case 11:
	report_results_count = true;
	break;
    case 12:
	intermediate_results_filename = opt_arg;
    case 13:
	report_results_intermediate = true;
	break;
    case 14:
	final_results_filename = opt_arg;
    case 15:
	report_results_final = true;
	break;
    case 16:
	density = atof(opt_arg);
	break;
    case 17:
	call_mult = true;
	break;      
    }
}


int main(int argc, char **argv)
{
	tpie_init();

    parse_args(argc, argv, app_opts, parse_app_opts);

    if (verbose) {
	std::cout << "test_size = " << test_size << "." << std::endl;
	std::cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << std::endl;
	std::cout << "random_seed = " << random_seed << "." << std::endl;
	std::cout << "density = " << density << "." << std::endl;
	std::cout << "Call mult directly = " << call_mult << "." << std::endl;
    } else {
        std::cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    get_memory_manager().set_limit (test_mm_size);

    // A sparse matrix.
    apps::sparse_matrix<double> esm0(test_size, test_size);
    apps::sparse_matrix<double> esm0b(test_size, test_size);

    // A vector to multiply the sparse matrix by
    apps::matrix<double> ev0(test_size, 1);

    // The result vector
    apps::matrix<double> ev1(test_size, 1);

    // Streams for reporting values to ascii streams.
    
    std::ofstream *osc = NULL;
    std::ofstream *osi = NULL;
    std::ofstream *osf = NULL;
    ami::cxx_ostream_scan< apps::sm_elem<double> > *rptc = NULL;
    ami::cxx_ostream_scan< apps::sm_elem<double> > *rpti = NULL;
    ami::cxx_ostream_scan<double> *rptf = NULL;

    std::istream *isb = NULL;
    ami::cxx_istream_scan<apps::sm_elem<double> > *readb = NULL;    
    
    if (report_results_count) {
        osc = new std::ofstream(count_results_filename);
        rptc = new ami::cxx_ostream_scan< apps::sm_elem<double> >(osc);
    }

    if (read_banded_matrix) {
        isb = new std::ifstream(banded_read_filename);
        readb = new ami::cxx_istream_scan< apps::sm_elem<double> >(isb);
    }
    
    if (report_results_intermediate) {
        osi = new std::ofstream(intermediate_results_filename);
        rpti = new ami::cxx_ostream_scan< apps::sm_elem<double> >(osi);
    }
    
    if (report_results_final) {
        osf = new std::ofstream(final_results_filename);
        rptf = new ami::cxx_ostream_scan<double>(osf);
    }
    
    // Write some elements into the sparse matrix.

    if (!read_banded_matrix) {
	apps::scan_uniform_sm susm(test_size, test_size, density, random_seed);
    
	ami::scan(&susm, &esm0);

        if (report_results_count) {
            ami::scan(&esm0, rptc);
        }
    }
    
    // Write the elements of the vector.

    apps::fill_value<double> fv;

    fv.set_value(1.0);

    apps::matrix_fill(&ev0, &fv);

    // Multiply the two

    if (call_mult) {
        apps::sparse_mult(esm0, ev0, ev1);
    } else {

        cpu_timer cput0, cput1;
        
		TPIE_OS_SIZE_T rows_per_band=0;
        TPIE_OS_OFFSET total_bands;
        
        if (read_banded_matrix) {

            cput1.reset();
            cput1.start();

            if (verbose) {
                std::cout << "Reading banded matrix from \""
			  << banded_read_filename << "\"" << std::endl;
            }

            // Read in the banded order matrix from an input file.            
            TPIE_OS_SIZE_T file_test_mm_size;
	    TPIE_OS_OFFSET file_test_size;
            TPIE_OS_SIZE_T file_rows_per_band;

            *isb >> file_test_mm_size >> file_test_size
		 >> file_rows_per_band;

            rows_per_band = file_rows_per_band;
            
            ami::scan(readb, &esm0b);

            cput1.stop();

            std::cout << cput1 << std::endl;
            
        } else {

            if (verbose) {
		std::cout << "Generating banded matrix." << std::endl;
            }

            cput0.reset();
            cput0.start();

            // Compute band information.

            apps::sparse_band_info(esm0, rows_per_band, total_bands);

            // Bandify the sparse matrix.
            
            apps::sparse_bandify(esm0, esm0b, rows_per_band);

            cput0.stop();

            std::cout << cput0 << std::endl;
            
        }
        
        if (report_results_intermediate) {
            *osi << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size)
		 << ' ' 
		 << test_size 
		 << ' '
                 << static_cast<TPIE_OS_OUTPUT_SIZE_T>(rows_per_band) << std::endl;
            ami::scan(&esm0b, rpti);
        }

        // Do the multiplication.

        if (read_banded_matrix) {
            cput1.reset();
            cput1.start();
        }
        
        apps::sparse_mult_scan_banded(esm0b, ev0, ev1, test_size,
					   test_size, rows_per_band);

        if (read_banded_matrix) {
            cput1.stop();
            std::cout << cput1 << std::endl;
        }
                
    }
    
    if (verbose) {
	std::cout << "Multiplied them." << std::endl;
        std::cout << "Stream length = " << ev1.stream_len() << std::endl;
    }
    
    if (report_results_final) {
        ami::scan(&ev1, rptf);
    }
    
    tpie_finish();
    
    return 0;
}
