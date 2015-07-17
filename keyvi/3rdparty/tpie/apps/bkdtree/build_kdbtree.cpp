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

// Copyright (C) 2001-2003 Octavian Procopiuc
//
// File:    build_kdbtree.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// Not just a test file for K-D-B-trees, but a full suite for
// constructing, querying, printing and benchmarking them. 
// It uses the run-time parameters from app_params.cpp
//
// $Id: build_kdbtree.cpp,v 1.2 2004-08-12 12:36:05 jan Exp $
//

#include <tpie/config.h>
#include <tpie/tpie.h>
#include <tpie/portability.h>

// STL stuff.
#include <functional>
// TPIE configuration: choose BTE, block size, etc.
#include "app_config.h"
// TPIE streams and collections.
#include <tpie/stream.h>
#include <tpie/coll.h>
// TPIE timer.
#include <tpie/cpu_timer.h>
// The K-D-B-tree implementation.
#include <tpie/kdbtree.h>

#include <tpie/portability.h>

// STL stuff.
#include <functional>
// TPIE configuration: choose BTE, block size, etc.
#include "app_config.h"
// TPIE streams and collections.
#include <tpie/stream.h>
#include <tpie/coll.h>
// TPIE timer.
#include <tpie/cpu_timer.h>
// The K-D-B-tree implementation.
#include <tpie/kdbtree.h>
// Run-time parameters.
#include "app_params.h"

using namespace tpie;
using namespace tpie::ami;
using namespace std;

// Template instantiations.
typedef kdbtree<int, 2, kdtree_bin_node_small<int, 2> > KDBTREEint2;

bool tiger_constrained(const record<int, size_t, 2>& p) {
  // Constrain TIGER data to continental US.
  // Numbers represent longitude and latitude times 1 million.
  return (p[0] > -128000000 && p[0] < -65000000 && 
	  p[1] > 21000000 && p[1] < 50000000);
}

int main(int argc, char** argv) {

  KDBTREEint2* kdbtree;
  KDBTREEint2::point_t *pp;
  kdbtree_params kdb_params;
  cpu_timer atimer;
  double t_wall, t_io;

  cout << "sizeof(region_t)=" << sizeof(region_t<int,2>) 
       << "  sizeof(kdb_item_t)=" << sizeof(kdb_item_t<int,2>) << "\n";

  // <><><><><><><><><><><><><><><><><><><><>
  //    Initialize.
  // <><><><><><><><><><><><><><><><><><><><>

  // Log debugging info from the application, but not from the library. 
  tpie_init();
 
   // Parse command-line arguments (see app_params.cpp)
  parse_args(argc, argv);

  params.structure_name = "KDB-TREE";

  // Set the kdbtree run-time parameters.
  kdb_params.leaf_size_max = params.leaf_size_max;
  kdb_params.node_size_max = params.node_size_max;
  kdb_params.leaf_block_factor = params.leaf_block_factor;
  kdb_params.node_block_factor = params.node_block_factor;
  kdb_params.node_cache_size = params.node_cache_size;
  kdb_params.leaf_cache_size = params.leaf_cache_size;
  kdb_params.split_heuristic = params.split_heuristic;

  // Open a kdb-tree.
  kdbtree = new KDBTREEint2(params.base_file_name_t, 
			    tpie::ami::WRITE_COLLECTION, kdb_params);
  // Check its status.
  if (kdbtree->status() == tpie::ami::KDBTREE_STATUS_INVALID) {
    cerr << "Error opening the kdbtree (see log for details). Aborting.\n";
    return 1;
  }
  // Set the persistency flag.
  kdbtree->persist(params.keep_tree ? PERSIST_PERSISTENT : PERSIST_DELETE);
  // Adjust some parameters to reflect the actual kdb-tree parameters,
  // in the case of an existing tree.
  params.leaf_block_factor = kdbtree->params().leaf_block_factor;
  params.node_block_factor = kdbtree->params().node_block_factor;
  // Statistics.
  add_to_stats(0, "Node capac. (keys):  ", kdbtree->params().node_size_max);
  add_to_stats(0, "Leaf capac. (items): ", kdbtree->params().leaf_size_max);
  params.stats << "Split heuristic:      "
	       << (params.split_heuristic == CYCLICAL ? "CYCLICAL": 
		   params.split_heuristic == LONGEST_SPAN ? 
		   "LONGEST_SPAN": "RANDOM") << "\n";

  cout << "Node size: " 
       << TPIE_OS_BLOCKSIZE()*params.node_block_factor 
       << " bytes. Node capacity: " 
       << static_cast<TPIE_OS_OUTPUT_SIZE_T>(KDBTREEint2::node_t::el_capacity(TPIE_OS_BLOCKSIZE()*params.node_block_factor))
       << " elements." << "\n";




  // <><><><><><><><><><><><><><><><><><><><>
  //     Build from kd-tree.
  // <><><><><><><><><><><><><><><><><><><><>

  if (kdbtree->status() == tpie::ami::KDBTREE_STATUS_KDTREE) {
    cerr << "Kd-tree -> K-D-B-tree... " << flush;
    if (kdbtree->kd2kdb())
      cerr << "Done.\n";
    else {
      cerr << "Error (see log for details).\n";
      cerr << "Structure stored in \"" << params.base_file_name_t 
	   << "\" is now invalid.\nAborting\n";
      delete kdbtree;
      return 2;
    }
  }


  // <><><><><><><><><><><><><><><><><><><><>
  //    Insert from input stream.
  // <><><><><><><><><><><><><><><><><><><><>

  if (params.do_insert) {
    assert(params.in_stream != NULL);

    // Temporary stream (no-duplicates stream).
    stream<KDBTREEint2::point_t> *nodup_stream;
    bool ans;

    if (params.do_eliminate_duplicates) {
      nodup_stream = new stream<KDBTREEint2::point_t>(params.nodup_file_name);
      nodup_stream->persist(PERSIST_PERSISTENT);
    }

    cerr << "Inserting..." << flush;
    TPIE_OS_OFFSET i = 0;
    atimer.start();
    params.in_stream->seek(0);
    while (params.in_stream->read_item(&pp) == tpie::ami::NO_ERROR) {

      // Insert *pp into the K-D-B-tree.
      ans = kdbtree->insert(*pp);

      // If *pp was successfully inserted in the K-D-B-tree, it means
      // it's not a duplicate.
      if (params.do_eliminate_duplicates && ans && 
	  (!params.do_constrain_tiger_data || tiger_constrained(*pp)))
	nodup_stream->write_item(*pp);

      // Print something every 100000 inserts to monitor progress.
      if (++i % 100000 == 0)
	cerr << " " << i << flush;
    }

    cerr << endl;
    atimer.stop();
    t_wall = atimer.wall_time();
    t_io = t_wall - atimer.user_time() - atimer.system_time();
    cerr << "\tInsert timings: " << atimer << "\n";
    params.stats << "INSERT (Wall IO %IO): " 
	  << double(int(t_wall*100)) / 100 << "\t "
	  << double(int(t_io*100)) / 100 << "\t "
	  << (t_wall > 0 ? int(t_io*100/t_wall) : 0) << "\n";

    atimer.reset();
    delete params.in_stream;

    if (params.do_eliminate_duplicates) {
      delete nodup_stream;
    }
  }


  // <><><><><><><><><><><><><><><><><><><><>
  //    Query from ASCII file.
  // <><><><><><><><><><><><><><><><><><><><>

  if (params.do_wquery_from_file) {
    ifstream ifs(params.file_name_wquery);
    if (!ifs) {
      cerr << argv[0] << ": Error opening window query file " << params.file_name_wquery << "\n";
    } else {
      TPIE_OS_OFFSET count = 0, result = 0;
      KDBTREEint2::point_t lop, hip;
      stream<KDBTREEint2::point_t> *tempstr = 
	(params.do_query_count_only ? NULL: new stream<KDBTREEint2::point_t>);
      cerr << "Window queries from file " << params.file_name_wquery << " ...\n";
      atimer.start();  

      ifs >> lop[0] >> lop[1] >> hip[0] >> hip[1];
      while (!ifs.eof()) {
	count++;
	result += kdbtree->window_query(lop, hip, tempstr);
	ifs >> lop[0] >> lop[1] >> hip[0] >> hip[1];
      }
      atimer.stop();
      t_wall = atimer.wall_time();
      t_io = t_wall - atimer.user_time() - atimer.system_time();
      cerr << "\tPerformed " << count << " queries.\n";
      cerr << "\tQuery timings: " << atimer << "\n";
      cerr << "\tFound " << result << " points." << endl;
      if (tempstr != NULL)
	cerr << "\t  (real: " << tempstr->stream_len() << " points)" << endl;
      add_to_stats(0, "FQUERY:FILE          ", params.file_name_wquery);
      add_to_stats(0, "FQUERY:COUNT         ", count);
      add_to_stats(0, "FQUERY:RESULT        ", result);
      params.stats << "FQUERY:WALL_IO_%IO   "
	    << double(int(t_wall*1000))/1000 << " "
	    << double(int(t_io*100))/100 << " "
	    << int(t_io*100/t_wall) << "\n";      
      atimer.reset();

      delete tempstr;
    }
  }


  // <><><><><><><><><><><><><><><><><><><><>
  //    Print tree (DFS in-order).
  // <><><><><><><><><><><><><><><><><><><><>

  if (params.do_display) {
    int level = -1;
    kdb_item_t<int, 2> ki;
    cout << "DFS: ";
    do {
      ki = kdbtree->dfs_preorder(level);
      cout << "(" << ki.bid << (ki.type == BLOCK_NODE? 'B':'L') << level << ") ";
    } while (level > -1);
    cout << endl;
  }


  // <><><><><><><><><><><><><><><><><><><><>
  //    Clean up and print statistics.
  // <><><><><><><><><><><><><><><><><><><><>
  
  stats_tree bts;
  params.point_count = kdbtree->size();
  bts = kdbtree->stats();
  params.stats << "Space utilization:    " 
       << (kdbtree->size() * sizeof(KDBTREEint2::point_t) * 100.00 / 
	   (kdbtree->stats().get(LEAF_COUNT) * kdbtree->leaf_block_size() + 
	    kdbtree->stats().get(NODE_COUNT) * kdbtree->node_block_size()))
	       << "%\n";
  delete kdbtree;

  params.write_block_stats(bts);

  print_configuration();
  print_statistics();

  // Write statistics to a file (append).
  ofstream ofs(params.file_name_stats, ios::app);
  print_configuration(ofs);
  print_statistics(ofs);

  return 0;
}

