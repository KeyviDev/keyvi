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

// A test for AMI_partition_and_merge().

#include <tpie/portability.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

#include <tpie/merge.h>
#include <tpie/mergeheap.h>

// Utitlities for ascii output.
#include <tpie/scan_utils.h>

#include "scan_random.h"

using namespace tpie;

// A merge object to merge sorted streams.  This code looks a lot like
// what is included as part of the TPIE system for sorting in
// ami_sort_single.h.

class s_merge_manager : public ami::merge_base<int> {
private:
    ami::merge_heap_op<int> *mheap;
    TPIE_OS_SIZE_T input_arity;
#if DEBUG_ASSERTIONS
    TPIE_OS_OFFSET input_count, output_count;
#endif    
    // Prohibit using the next two.
    s_merge_manager(const s_merge_manager& other);
    s_merge_manager& operator=(const s_merge_manager& other);
public:
    s_merge_manager(void);
    virtual ~s_merge_manager(void);
    ami::err initialize(TPIE_OS_SIZE_T arity, int **in,
			ami::merge_flag *taken_flags,
			int &taken_index);
    ami::err operate(int **in, ami::merge_flag *taken_flags,
		     int &taken_index, int *out);
    ami::err main_mem_operate(int* mm_stream, TPIE_OS_SIZE_T len);
    TPIE_OS_SIZE_T space_usage_overhead(void);
    TPIE_OS_SIZE_T space_usage_per_stream(void);
};


s_merge_manager::s_merge_manager(void) : mheap(NULL), input_arity(0)
#if DEBUG_ASSERTIONS
				       , input_count(0), output_count(0)
#endif
{
    // Do nothing.
}


s_merge_manager::~s_merge_manager(void)
{
    if (mheap != NULL) {
        mheap->deallocate();
        delete mheap;
    }
}


ami::err s_merge_manager::initialize(TPIE_OS_SIZE_T arity, int **in,
				     ami::merge_flag *taken_flags,
				     int &taken_index)
{
    TPIE_OS_SIZE_T ii;

    input_arity = arity;

    tp_assert(arity > 0, "Input arity is 0.");
    
    if (mheap != NULL) {
        mheap->deallocate();
        delete mheap;
    }
    mheap = new ami::merge_heap_op<int>();
    mheap->allocate(arity);

#if DEBUG_ASSERTIONS
    input_count = output_count = 0;
#endif    
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
            mheap->insert(in[ii],ii);
#if DEBUG_ASSERTIONS
            input_count++;
#endif                  
        } else {
            taken_flags[ii] = 0;
        }
    }

    taken_index = -1;
    return ami::MERGE_READ_MULTIPLE;
}


TPIE_OS_SIZE_T s_merge_manager::space_usage_overhead(void)
{
    return mheap->space_overhead();
}


TPIE_OS_SIZE_T s_merge_manager::space_usage_per_stream(void)
{
    return sizeof(TPIE_OS_SIZE_T) + sizeof(int);
}


ami::err s_merge_manager::operate(int **in,
				  ami::merge_flag * /*taken_flags*/,
				  int &taken_index,
				  int *out)
{
    // If the queue is empty, we are done.  There should be no more
    // inputs.
    if (!mheap->sizeofheap()) {

#if DEBUG_ASSERTIONS
        TPIE_OS_SIZE_T ii;
        
        for (ii = input_arity; ii--; ) {
            tp_assert(in[ii] == NULL, "Empty queue but more input.");
        }

        tp_assert(input_count == output_count,
                  "Merge done, input_count = " << input_count <<
                  ", output_count = " << output_count << '.');
#endif        

        return ami::MERGE_DONE;

    } else {
        TPIE_OS_SIZE_T min_source;
        int min_t;

        mheap->extract_min(min_t, min_source);
        *out = min_t;
        if (in[min_source] != NULL) {
            mheap->insert(in[min_source], min_source);
            taken_index = (int)min_source;
            //taken_flags[min_source] = 1;
#if DEBUG_ASSERTIONS
            input_count++;
#endif            
        } else {
            taken_index = -1;
        }
#if DEBUG_ASSERTIONS
        output_count++;
#endif        
        return ami::MERGE_OUTPUT;
    }
}


ami::err s_merge_manager::main_mem_operate(int* mm_stream, TPIE_OS_SIZE_T len)
{
	std::sort(mm_stream,mm_stream+len);
    return ami::NO_ERROR;
}


static char def_srf[] = "oss.txt";
static char def_rrf[] = "osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

struct options app_opts[] = {
    { 10, "random-results-filename", "", "R", 1 },
    { 11, "report-results-random", "", "r", 0 },
    { 12, "sorted-results-filename", "", "S", 1 },
    { 13, "report-results-sorted", "", "s", 0 },
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
    }
}


int main(int argc, char **argv)
{
    parse_args(argc, argv, app_opts, parse_app_opts);

    if (verbose) {
	std::cout << "test_size = " << test_size << "." << std::endl;
	std::cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << std::endl;
	std::cout << "random_seed = " << random_seed << "." << std::endl;
    } else {
        std::cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    get_memory_manager().set_limit (test_mm_size);
        
    ami::stream<int> amis0;
    ami::stream<int> amis1;
        
    // Write some ints.
    scan_random rnds(test_size,random_seed);
    
    ami::scan(&rnds, &amis0);

    if (verbose) {
	std::cout << "Wrote the random values." << std::endl;
        std::cout << "Stream length = " << amis0.stream_len() << std::endl;
    }

    // Streams for reporting random and/or sorted values to ascii
    // streams.
    
    std::ofstream *oss;
    ami::cxx_ostream_scan<int> *rpts = NULL;
    std::ofstream *osr;
    ami::cxx_ostream_scan<int> *rptr = NULL;
    
    if (report_results_random) {
        osr  = new std::ofstream(rand_results_filename);
        rptr = new ami::cxx_ostream_scan<int>(osr);
    }
    
    if (report_results_sorted) {
        oss  = new std::ofstream(sorted_results_filename);
        rpts = new ami::cxx_ostream_scan<int>(oss);
    }
    
    if (report_results_random) {
        ami::scan(&amis0, rptr);
    }

    s_merge_manager sm;
    
    ami::partition_and_merge(&amis0, &amis1, &sm);
    
    if (verbose) {
	std::cout << "Sorted them."<< std::endl;
        std::cout << "Sorted stream length = " << amis1.stream_len() << std::endl;
    }
    
    if (report_results_sorted) {
        ami::scan(&amis1, rpts);
    }    

    std::cout << std::endl;

    return 0;
}
