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

// Copyright (C) 2001 Octavian Procopiuc
//
// File:    app_params.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// Runtime parameters for the kd-tree, K-D-B-tree and B-tree test
// suite.
//
// $Id: app_params.cpp,v 1.7 2004-08-12 12:36:04 jan Exp $

#include "app_config.h"
#include <tpie/config.h>
#include <tpie/portability.h>
#include <algorithm>
#include "app_config.h"
#include <tpie/kd_base.h>


#include <tpie/kd_base.h>
#include "app_params.h"
#include <tpie/stream.h>
#include <tpie/scan.h>
#include <tpie/scan_utils.h>
#include <tpie/coll.h>
#include <tpie/tempname.h>

using namespace tpie;
using namespace tpie::ami;
using namespace std;

// Initialize a global object with all run-time parameters.
app_params_t params;

// static istream& operator>>(istream& s, app_params_t::record_t& p) {
//  for (int i = 0; i < DIM; i++)
//    s >> p[i];
//  return s >> p.id();
//  }
 
void usage(char* argv0) {
  cerr << "Usage: " << argv0 << "\n"
       << "   [-m <memory_size_in_MB>] (TPIE memory size in megabytes)\n"
       << "   [-is <base_name_sorted> | -iu <base_name_unsorted> |\n"
       << "       -ia <ascii_file_name> [...] | -it <base_name_tree>] (input method; choose one)\n"
       << "   [-os <base_name_sorted>] (write sorted files; \"0\" and \"1\" are added to the name)\n"
       << "   [-ou <base_name_unsorted>] (write unsorted file)\n"
       << "   [-ot <base_name_tree>] (write tree to files given by <base_name_tree>)\n"
       << "   [-g[rid] | -b[inary] | -s[ample]]  (kdtree loading method)\n"
       << "   [-t <grid_size>] (for grid method only; a <grid_size>x<grid_size> grid will be used)\n"
       << "   [-lf <leaf_block_factor>] (number of OS pages in a leaf block)\n"
       << "   [-nf <node_block_factor>] (number of OS pages in a node block)\n"
       << "   [-cf <catalog_block_factor>]\n"
       << "   [-lc <leaf_capacity>] (max number of points in a leaf)\n"
       << "   [-nc <node_capacity>] (max number of keys in a node)\n"
       << "   [-lh <leaf_cache_size>] (max number of leaves in the cache)\n"
       << "   [-nh <node_cache_size>] (max number of nodes in the cache)\n"
       << "   [-d[irectio]] (use direct I/O; on solaris and for ufs collections only)\n"
       << "   [-cb <cached_blocks>]  (number of cached OS blocks in logmethod)\n"
       << "   [-cc <child_cache_fill>]  (child cache fill factor for EPS-tree)\n"
       << "   [-bl <bulk_load_fill>]  (fill factor for bulk loading)\n"
       << "   [-L] (use Logmethod)\n"
       << "   [-B  <B_value>] (use LogmethodB, not Logmethod2; set B value for LogmethodB)\n"
       << "   [-qf <file_name>]  (perform queries from given ASCII file)\n"
       << "   [-qt 0|2|3|4]  (query type: 4=4-sided, 3=3-sided, 0=native)\n"
       << "   [-qc]  (only count the points inside the query window)\n"
       << "   [-S]  (verify sorted order)\n"
       << "   [-in [<point_count>]]  (if possible, build tree by inserting the input points)\n"
       << "   [-e[t] [<file_name>]] (eliminate duplicate keys, if implemented; restrict TIGER data to continental US; write the resulting data to file_name)\n"
       << "   [-h C|L|R]  (choose K-D-B-tree split heuristic)\n" 
       << endl;
}

void print_statistics(ostream& os) {
  os << params.stats << endl;
}

void print_configuration(ostream& os) {

  os   << "MEMORY_SIZE:          " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(params.memory_limit/(1024*1024)) << " MB\n" 
       << "POINT_COUNT:          " << params.point_count << "\n"
       << "DATA_STRUCTURE:       " << params.structure_name << (params.do_logmethod ? " + LOG.METHOD": "") << endl
       << "OPERATION:            " << (params.do_insert ? "INSERT": (params.do_load ? "BULK_LOAD" : (params.do_wquery_from_file || params.wquery_count ? "WINDOW_QUERY" : "UNKNOWN"))) << endl;

  os   << "BTE_STREAM:           "
#if defined(BTE_STREAM_IMP_MMAP)
       << "MMAP " << STREAM_MMAP_BLOCK_FACTOR
#elif defined(BTE_STREAM_IMP_STDIO)
       << "STDIO"
#elif defined(BTE_STREAM_IMP_UFS)
       << "UFS " << STREAM_UFS_BLOCK_FACTOR
#else
       << "UNKNOWN"
#endif
       << endl;

  os   << "BTE_COLLECTION        " 
#if defined(BTE_COLLECTION_IMP_MMAP)
       << "MMAP" << " (LAZY_WRITE=" << BTE_COLLECTION_MMAP_LAZY_WRITE << ")"
#elif defined(BTE_COLLECTION_IMP_UFS)
       << "UFS"
#else
       << "UNKNOWN"
#endif
       << " L" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(params.leaf_block_factor) << " N" 
	   << static_cast<TPIE_OS_OUTPUT_SIZE_T>(params.node_block_factor) << endl;

  if (params.do_logmethod) {
    os << "Cached os blocks:     " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(params.cached_blocks) << endl;
    if (params.B_for_LMB != 0)
      os << "B (for LogMethodB)    " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(params.B_for_LMB) << endl;
  }

#if (defined(__sun__) && defined(BTE_COLLECTION_IMP_UFS))
  os << "Directio bcc:         " << (params.direct_io_bcc ? "ON": "OFF") << endl;
#endif
  os << "BLOCK_CACHE:          " 
     << static_cast<TPIE_OS_OUTPUT_SIZE_T>((params.leaf_cache_size + params.node_cache_size))
     << " \tL" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(params.leaf_cache_size) 
     << " \tN" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(params.node_cache_size) << endl;

}


void parse_args(int argc, char** argv) {
  int i;
  //  int j;
  assert(DIM >= 2);
  if (DIM > 2)
    cerr << "Dimension: " << DIM << endl;
  params.stats << "COMMAND_LINE:         ";
  for (i = 0; i < argc; i++)
    params.stats << argv[i] << " ";
  params.stats << endl;

  if (argc == 1) {
    usage(argv[0]);
    exit(1);
  }
  char tmpname[MAX_PATH_LENGTH];
  app_params_t::stream_t* random_stream = NULL;
  //AMI_err err;
  bool keep_input = false;
  bool output_unsorted = false;
  bool initialization_only = false;
  //  bool do_compute_mbr = true;

  // The default base name for the tree files.
  strcpy(params.base_file_name_t, tempname::tpie_name("AMI").c_str());
  strcpy(params.file_name_stats, "results.txt");
  params.base_file_name_s[0] = 0;
  params.base_file_name[0] = 0;
  i = 1;
  while (i < argc) {
    if (argv[i][0] != '-') {
      cerr << argv[0] << ": " << argv[i] 
	   << ": Unrecognized option." << endl;
      usage(argv[0]);
      exit(1);
    }
    switch (argv[i][1]) {
    case 'g':
      params.load_method = TPIE_AMI_KDTREE_LOAD_SORT|TPIE_AMI_KDTREE_LOAD_GRID;
      break;
    case 'b':
      if (argv[i][2] == 'l')
	params.bulk_load_fill = (float)min(atof(argv[++i]), 1.0);
      else
	params.load_method = TPIE_AMI_KDTREE_LOAD_SORT|TPIE_AMI_KDTREE_LOAD_BINARY;
      break;
    case 'h':
      // Split heuristic for the K-D-B-tree.
      switch (argv[++i][0]) {
      case 'C':
	params.split_heuristic = CYCLICAL;
	break;
      case 'L':
	params.split_heuristic = LONGEST_SPAN;
	break;
      case 'R':
	params.split_heuristic = RANDOM;
	break;
      }
      break;
    case 's':
      params.do_sort = false;
      params.load_method = TPIE_AMI_KDTREE_LOAD_SAMPLE;
      break;
    case 'm':
      params.memory_limit = atoi(argv[++i])*1024*1024;
      break;
    case 'I': // initialization only.
      initialization_only = true;
      break;
    case 'B':
      params.B_for_LMB = atoi(argv[++i]);
      break;
    case 'L':
      params.do_logmethod = true;
      params.do_sort = false;
      params.do_load = false;
      break;
    case 'S': // Verify the sorting routine.
      params.do_verify_sorting = true;
      break;
    case 'P':
      params.do_print_tree = true;
      break;
    case 'i':
      switch (argv[i][2]) {
      case 'n': // insert.
	params.do_insert = true;
	params.do_sort = false;
	params.do_load = false;
	if ((i+1 < argc) && (argv[i+1][0] != '-'))
	  params.point_count = atoi(argv[++i]);
	break;
      case 's': // sorted.
	params.do_sort = false;
	strncpy(params.base_file_name_s, argv[++i], MAX_PATH_LENGTH-1);
	break;
      case 'u': // unsorted
	keep_input = true;
	strncpy(params.base_file_name, argv[++i], MAX_PATH_LENGTH);
	params.in_stream = new app_params_t::stream_t(params.base_file_name, 
					tpie::ami::READ_STREAM);
	if (!params.in_stream->is_valid()) {
	  cerr << "Invalid input stream." << endl;
	  delete params.in_stream;
	  params.in_stream = NULL;
	} else 
	  if (params.point_count == 0)
	    params.point_count = params.in_stream->stream_len();
	break;
      case 'r': // random.
#if 0
	params.point_count = atoi(argv[++i]);
	params.in_stream = new app_params_t::stream_t;
	TPIE_OS_SRANDOM(time(NULL));
	for (j = 0; j < params.point_count; j++) {
	  for (int jj = 0; jj < DIM; jj++)
	    p[jj] = int((TPIE_OS_RANDOM()/MAX_RANDOM) * MAX_VALUE);
	  params.in_stream->write_item(p);
	}
#else
	cerr << "This option is no longer supported. Use datagen to generate points." << endl;
#endif
	break;
      case 'a':
#if 0
	if (params.in_stream == NULL)
	  params.in_stream = new app_params_t::stream_t;

	while (argv[i+1][0] != '-') {
	  ifstream ifs(argv[++i]);
	  if (!ifs) {
	    cerr << argv[0] << ": Error opening file " << argv[i] << endl;
	    exit(1); 
	  }
	  cerr << "Reading ascii input..." << flush;
	  cxx_istream_scan<app_params_t::point_t> read_input(&ifs);
	  // Read ascii input.
	  err = AMI_scan(&read_input, params.in_stream);

	  if (err != tpie::ami::NO_ERROR) {
	    cerr << "\n" << argv[0] << ": Error reading ascii input." << endl;
	    exit(1);
	  }
	  cerr << "done." << endl;
	}
	params.point_count = params.in_stream->stream_len();
#else
	cerr << "This option is no longer supported. Use datagen to generate points." << endl;
#endif
	break;
      case 't':
	params.do_sort = false;
	params.do_load = false;
	params.keep_tree = true;
	strncpy(params.base_file_name_t, argv[++i], MAX_PATH_LENGTH);
	break;
      default:
	cerr << argv[0] << ": " << argv[i] 
	     << ": Unrecognized option." << endl;
	usage(argv[0]);
	exit(1);
      }
      break;
    case 'o':
      switch (argv[i][2]) {
      case 's':
	params.do_sort = true;
	strncpy(params.base_file_name_s, argv[++i], MAX_PATH_LENGTH-1);
	break;
      case 'u':
	keep_input = true;
	output_unsorted = true;
	strncpy(params.base_file_name, argv[++i], MAX_PATH_LENGTH);
	unlink(params.base_file_name);
	break;
      case 't': // tree file name
	params.keep_tree = true;
	strncpy(params.base_file_name_t, argv[++i], MAX_PATH_LENGTH);
	break;
      case 'r':
	random_stream = new app_params_t::stream_t(argv[++i]);
	random_stream->persist(PERSIST_PERSISTENT);
	initialization_only = true;
	break;
      default:
	cerr << argv[0] << ": " << argv[i] 
	     << ": Unrecognized option." << endl;
	usage(argv[0]);
	exit(1);
	break;
      }
      break;
    case 'w':
    case 'q':
      switch (argv[i][2]) {
      case 'r': // random queries.
	params.wquery_count = atoi(argv[++i]);
	break;
      case 'x':
	params.wquery_size_x = atof(argv[++i]);
	break;
      case 'y':
	params.wquery_size_y = atof(argv[++i]);
	break;
      case 'f':
	params.do_wquery_from_file = true;
	strncpy(params.file_name_wquery, argv[++i], MAX_PATH_LENGTH);
	break;
      case 'o':
      case 'c':
	params.do_query_count_only = true;
	break;
      case 't':
	params.query_type = atoi(argv[++i]);
	break;
      }
      break;
    case 't':
      params.grid_size = atoi(argv[++i]);
      break;
    case 'l': 
      switch (argv[i][2]) {
      case 'f':
	params.leaf_block_factor = atoi(argv[++i]);
	break;
      case 'c':
	params.leaf_size_max = atoi(argv[++i]);
	break;
      case 'h':
	params.leaf_cache_size = atoi(argv[++i]);
	break;
      }
      break;
    case 'n':
      switch (argv[i][2]) {
      case 'f':
	params.node_block_factor = atoi(argv[++i]);
	break;
      case 'c':
	params.node_size_max = atoi(argv[++i]);
	break;
      case 'h':
	params.node_cache_size = atoi(argv[++i]);
	break;
      }
      break;
    case 'd': 
      params.direct_io_bcc = true;
#if defined(__sun__)
      BTE_COLLECTION::direct_io = true;
#endif
      break;
    case 'c':
      switch (argv[i][2]) {
      case 'f':
	params.catalog_block_factor = atoi(argv[++i]);
	break;
      case 'b':
	params.cached_blocks = atoi(argv[++i]);
	break;
      case 'c':
	params.child_cache_fill = (float)atof(argv[++i]);
	break;
      }
      break;
    case 'r':
      params.max_intraroot_height = atoi(argv[++i]);
      break;
    case 'e':
      params.do_eliminate_duplicates = true;
      if (argv[i][2] == 't')
	params.do_constrain_tiger_data = true;
      // Check if we have a file name.
      if ((i+1 < argc) && (argv[i+1][0] != '-'))
	strncpy(params.nodup_file_name, argv[++i], MAX_PATH_LENGTH);
      break;
    default:
      break;
    }
    
    i++;
  }

  // Set the TPIE memory limit (at least 4MB).
  params.memory_limit = max(params.memory_limit, size_t(4*1024*1024));
  get_memory_manager().set_limit(params.memory_limit);
  get_memory_manager().set_enforcement(memory_manager::ENFORCE_THROW);

  if (params.in_stream != NULL) {
    params.in_stream->persist(keep_input? PERSIST_PERSISTENT : PERSIST_DELETE);
    
    if (output_unsorted) {
      params.in_stream->persist(PERSIST_PERSISTENT);
	  std::string mv_command =
		"/bin/nmv " + params.in_stream->name()
		+ std::string(" ") + params.base_file_name;
      delete params.in_stream;
	  if (system(mv_command.c_str()) == -1) {
		  perror("Error renaming the file");	
		  exit(1);
      }

      params.in_stream = new app_params_t::stream_t(params.base_file_name,
				      tpie::ami::READ_STREAM);
      params.in_stream->persist(PERSIST_PERSISTENT);
    }

#if 0 // Commented out.
    if (random_stream != NULL) {
      size_t i, j;
      size_t l = params.in_stream->stream_len();
      app_params_t::point_t *pp1, *pp2, p2;
      for (i = 0; i < l; i++) {
	params.in_stream->read_item(&pp1);
	random_stream->write_item(*pp1);
      }
      params.in_stream->seek(0);
      cout << "Shuffling... " << flush;
      for (i = 0; i < l/2; i++) {
	if (i%1000==0)
	  cout << i << " " << flush;
	// Read item from position 2*i in in_stream.
	params.in_stream->read_item(&pp1);
	// Go to a random odd position in random_stream.
	j = (TPIE_OS_RANDOM() % (l/2)) * 2 + 1;
	random_stream->seek(j);
	random_stream->read_item(&pp2);
	p2 = *pp2;
	random_stream->seek(j);
	random_stream->write_item(*pp1);
	random_stream->seek(2 * i);
	random_stream->write_item(p2);
	// skip odd positions.
	params.in_stream->read_item(&pp1);
      }
      cout << endl;
    }
#endif
    delete random_stream;
  }

  if (initialization_only) {
    delete params.in_stream;
    exit(0);
  }

  if (params.base_file_name_s[0]) {

    for (int jj = 0; jj < DIM; jj++) {
      strcpy(tmpname, params.base_file_name_s);
      size_t len = strlen(tmpname);
      tmpname[len] = 'A' + (char)jj;
      tmpname[len+1] = '\0';
      if (params.do_sort)
	unlink(tmpname);
      params.streams_sorted[jj] = new app_params_t::stream_t(tmpname);
      params.streams_sorted[jj]->persist(PERSIST_PERSISTENT);
    }

    if (!params.do_sort && params.point_count == 0)
      params.point_count = params.streams_sorted[0]->stream_len();
  }

}

