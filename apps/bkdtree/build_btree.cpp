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
// File:    build_btree.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
//
// $Id: build_btree.cpp,v 1.2 2004-08-12 12:36:05 jan Exp $
//

#define DIRECTIO_STREAMS 0

// Configuration: choose BTE, block size, etc.
#include "app_config.h"

#include <tpie/portability.h>
#include <tpie/tpie.h>

// STL files.
#include <vector>
#include <functional>
// TPIE streams.
#include <tpie/stream.h>
// The btree class.
#include <tpie/btree.h>
// TPIE timer.
#include <tpie/cpu_timer.h>
// The logarithmic method.
#include "tpie/logmethod.h"
// Point.
#include "tpie/point.h"
// Run-time parameters.
#include "app_params.h"

using namespace std;
using namespace tpie;
using namespace tpie::ami;

typedef btree<app_params_t::point_t, app_params_t::record_t, app_params_t::point_t::cmp, app_params_t::record_key_t >BTREEint2;
typedef  Logmethod2<app_params_t::point_t, app_params_t::record_t, BTREEint2, btree_params, BTREEint2, btree_params> LOGMETHOD2int2;

// Filter to get 3-sided queries from 2-sided range queries.
class filter_hiy_t {
  app_params_t::coord_t hiy_;
public:
  void initialize(app_params_t::coord_t hiy) { hiy_ = hiy; }
  // Filter out everything higher than hiy_.
  bool operator()(const app_params_t::record_t& r) const {
    return !(hiy_ < r[1]);
  }
};


int main(int argc, char **argv) {

  // Log debugging info from the application, but not from the library.
  tpie_init();
  parse_args(argc, argv);

  params.structure_name = "B+-tree";

  BTREEint2 *btree;
  LOGMETHOD2int2 *lm;
  btree_params btree_parameters;
  err err = tpie::ami::NO_ERROR;
  // Filter for 3-sided queries.
  filter_hiy_t filter3sq;

  btree_parameters.leaf_block_factor = params.leaf_block_factor;
  btree_parameters.node_block_factor = params.node_block_factor;

  if (params.do_logmethod) {
    Logmethod_params<btree_params, btree_params> lm_params;
    lm_params.cached_blocks = params.cached_blocks;
    lm_params.tree_params = btree_parameters;
    lm_params.tree0_params = btree_parameters;

    lm_params.tree0_params.node_cache_size = 64;
    lm_params.tree0_params.leaf_cache_size = (params.cached_blocks+7)/8 * 8;
    lm = new LOGMETHOD2int2(params.base_file_name_t, lm_params);
    lm->persist(params.keep_tree ? PERSIST_PERSISTENT : PERSIST_DELETE);
    params.leaf_block_factor = lm->params().tree_params.leaf_block_factor;
    params.node_block_factor = lm->params().tree_params.node_block_factor;
  } else {
    btree_parameters.node_cache_size = params.node_cache_size;
    btree_parameters.leaf_cache_size = params.leaf_cache_size;
    btree = new BTREEint2(params.base_file_name_t, WRITE_COLLECTION, btree_parameters);
    btree->persist(params.keep_tree ? PERSIST_PERSISTENT : PERSIST_DELETE);
    params.leaf_block_factor = btree->params().leaf_block_factor;
    params.node_block_factor = btree->params().node_block_factor;
  }

  cpu_timer atimer;
  app_params_t::record_t *pp;
  double time_wall, time_io;

  if (params.do_sort && err == tpie::ami::NO_ERROR) {
    cerr << "Sorting..." << endl;
    atimer.start();
    if (params.do_logmethod)
      cerr << "Internal error: options do_logmethod and do_sort are incompatible." << endl;
    else {
      app_params_t::record_t::cmp cmp;
      if (params.streams_sorted[0] == NULL)
	params.streams_sorted[0] = new app_params_t::stream_t;
      sort(params.in_stream, params.streams_sorted[0], &cmp);
      //      btree->sort(params.in_stream, params.streams_sorted[0]);
    }
    atimer.stop();
    cerr << "\tSorting timings: " << atimer << endl;
    time_wall = atimer.wall_time();
    time_io = time_wall - atimer.user_time() - atimer.system_time();

    atimer.reset();
    delete params.in_stream;

    params.stats << "SORT:WALL_IO_%IO     "
		 << double(int(time_wall*1000)) / 1000 << "\t "
		 << double(int(time_io*100))/100 << "\t "
		 << int(time_io*100/time_wall) << endl;
  } 

  if (params.do_verify_sorting && err == tpie::ami::NO_ERROR) {
    app_params_t::record_t p1;
    TPIE_OS_OFFSET ii = 0;
    app_params_t::record_t::cmp comp;
    cerr << "Verifying sorting..." << endl;
    params.streams_sorted[0]->seek(0);
    params.streams_sorted[0]->read_item(&pp);
    p1 = *pp;
    while (params.streams_sorted[0]->read_item(&pp) == tpie::ami::NO_ERROR) {
      if (comp(*pp, p1)) {
	cerr << "\tStream not properly sorted: items " 
	     << ii << " and " << ii+1 << endl;
	//	break;
      }
      ii++;
      p1 = *pp;
    }
  }

  if (params.do_load && err == tpie::ami::NO_ERROR) {
    assert(params.streams_sorted[0] != NULL);
    cerr << "Loading..." << endl;
    atimer.start();
    if (params.do_logmethod)
      cerr << "Internal error: options do_logmethod and do_load are incompatible." << endl;
    else
      err = btree->load_sorted(params.streams_sorted[0], 
			       params.bulk_load_fill, params.bulk_load_fill);
    atimer.stop();
    cerr << "\tLoading timings: " << atimer << endl;
    time_wall = atimer.wall_time();
    time_io = time_wall - atimer.user_time() - atimer.system_time();

    atimer.reset();
    delete params.streams_sorted[0];
    //    delete streams_sorted[1];

    params.stats << "LOAD:WALL_IO_%IO     "
		 << double(int(time_wall*1000)) / 1000 << "\t "
		 << double(int(time_io*100))/100 << "\t "
		 << int(time_io*100/time_wall) << endl;
  }

  if (params.do_insert && err == tpie::ami::NO_ERROR) {
    cerr << "Inserting..." << endl;
    TPIE_OS_OFFSET i = 0;
    assert(params.in_stream != NULL);
    atimer.start();
    params.in_stream->seek(0);
    while (i < params.point_count && 
	   params.in_stream->read_item(&pp) == tpie::ami::NO_ERROR) {
      if (params.do_logmethod)
	lm->insert(*pp);
      else
	btree->insert(*pp);
      i++;
      if (i % 10000 == 0)
	cerr << " " << i << flush;
    }
    atimer.stop();
    cerr << "\tInsert timings: " << atimer << endl;
    time_wall = atimer.wall_time();
    time_io = time_wall - atimer.user_time() - atimer.system_time();

    atimer.reset();
    delete params.in_stream;

    params.stats << "INSERT:WALL_IO_%IO   "
		 << double(int(time_wall*100)) / 100 << "\t "
		 << double(int(time_io*100))/100 << "\t "
		 << int(time_io*100/time_wall) << endl;
  }


  if (params.do_wquery_from_file && err == tpie::ami::NO_ERROR) {
    ifstream ifs(params.file_name_wquery);
    if (!ifs) {
      cerr << argv[0] << ": Error opening window queries file " 
	   << params.file_name_wquery << endl;
    } else {
      TPIE_OS_OFFSET count = 0;
      app_params_t::point_t lop, hip;
      app_params_t::stream_t *tempstr = new app_params_t::stream_t;
      cerr << "Window queries from file " 
	   << params.file_name_wquery << " ..." << endl;
      atimer.start();  

      ifs >> lop[0] >> lop[1] >> hip[0] >> hip[1];
      while (!ifs.eof()) {
	count++;
	if (params.do_logmethod)
	  lm->window_query(lop, hip, tempstr);
	else
	  if (params.query_type == 3) {
	    filter3sq.initialize(max(lop[1], hip[1]));
	    btree->window_query(lop, hip, tempstr, filter3sq);
	  } else
	    btree->window_query(lop, hip, tempstr);
	ifs >> lop[0] >> lop[1] >> hip[0] >> hip[1];
      }
      atimer.stop();
      cerr << "\tQuery timings: " << atimer << endl;
      time_wall = atimer.wall_time();
      time_io = time_wall - atimer.user_time() - atimer.system_time();
      cerr << "\tFound " << tempstr->stream_len() << " points." << endl;
      atimer.reset();
  
      add_to_stats(0, "FQUERY:FILE          ", params.file_name_wquery);
      add_to_stats(0, "FQUERY:COUNT         ", count);
      add_to_stats(0, "FQUERY:TYPE          ", params.query_type);
      add_to_stats(0, "FQUERY:RESULT        ", tempstr->stream_len());
      params.stats << "FQUERY:WALL_IO_%IO   "
		   << double(int(time_wall*1000)) / 1000 << "\t "
		   << double(int(time_io*100))/100 << "\t "
		   << int(time_io*100/time_wall) << endl;
      delete tempstr;
    }
  }
    
  if (params.wquery_count > 0 && err == tpie::ami::NO_ERROR) {
    app_params_t::point_t lop, hip;
    int mbrdx = MAX_VALUE;
    int mbrdy = MAX_VALUE;
    int wqdx = int(params.wquery_size_x / 100 * mbrdx);
    int wqdy = int(params.wquery_size_y / 100 * mbrdy);
    cerr << "Doing " << params.wquery_count << " window queries." << endl;
    cerr << "  width: " << params.wquery_size_x << "% of " << mbrdx
	 << ", height: " << params.wquery_size_y << "% of " << mbrdy << endl;
    app_params_t::stream_t* tempstr = new app_params_t::stream_t;
    TPIE_OS_SRANDOM((unsigned int)TPIE_OS_TIME(NULL));

    atimer.start();  
    for (TPIE_OS_OFFSET i = 0; i < params.wquery_count; i++) {
      lop[0] = TPIE_OS_RANDOM() % mbrdx  - wqdx / 2;
      lop[1] = TPIE_OS_RANDOM() % mbrdy  - wqdy / 2;
      hip[0] = lop[0] + wqdx;
      hip[1] = lop[1] + wqdy;
      if (params.do_logmethod)
	lm->window_query(lop, hip, tempstr);
      else
	btree->window_query(lop, hip, tempstr);
    }
    atimer.stop();
    cerr << "Query timings: " << atimer << endl;
    time_wall = atimer.wall_time();
    time_io = time_wall - atimer.user_time() - atimer.system_time();
    cerr << "Found " << tempstr->stream_len() << " points." << endl;
    atimer.reset();

    add_to_stats(0, "RQUERY:Size_x        ", params.wquery_size_x);
    add_to_stats(0, "RQUERY:Size_y        ", params.wquery_size_y);
    add_to_stats(0, "RQUERY:Count         ", params.wquery_count);
    add_to_stats(0, "RQUERY:Result        ", tempstr->stream_len());
    add_to_stats(4, "RQUERY:Wall          ", double(int(time_wall*100))/100);
    add_to_stats(6, "RQUERY:IO            ", double(int(time_io*100))/100);
    add_to_stats(2, "RQUERY:PercentIO     ", int(time_io*100/time_wall));
    delete tempstr;
  }


  if (params.do_stress_test && err == tpie::ami::NO_ERROR) {
    cerr << "Begin stress test." << endl;
    app_params_t::record_t p;
    app_params_t::record_t pa[100];
    TPIE_OS_SIZE_T insert_count = 50000;
    cerr << "\tInserting " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(insert_count) << " random points..." << flush;
    for (TPIE_OS_SIZE_T i = 0; i < insert_count; i++) {
      p[0] = TPIE_OS_RANDOM() % MAX_VALUE;
      p[1] = TPIE_OS_RANDOM() % MAX_VALUE;
      if (i < 100)
	pa[i] = p;
      btree->insert(p);
    }
    cerr << "Done" << endl;
    cerr << "\tBuilding new tree..." << flush;
    BTREEint2* btree2 = new BTREEint2();
    btree2->load(btree);
    cerr << "Done" << endl;
    cerr << "\tSearching 100 points..." << endl;
    for (int i = 0; i < 100; i++)
      if (!btree->find(pa[i].key, p))
	cerr << "\t\tPoint not found" << endl;

    cerr << "\tDone" << endl;
    delete btree2;
    cerr << "End stress test." << endl;
  }

  stats_tree bts;

  if (params.do_logmethod) {
    params.point_count = lm->size();
    bts = lm->stats();
    delete lm;
  } else {
    params.point_count = btree->size();  
    bts = btree->stats(); 
    delete btree;
  }

  if (err == tpie::ami::NO_ERROR) {
    params.write_block_stats(bts);

    print_configuration();
    print_statistics();
 
    // Write statistics to a file (append).
    ofstream ofs(params.file_name_stats, ios::app);
    
    print_configuration(ofs);
    print_statistics(ofs);
  } else {
    cerr << "An error ocurred. See log for details." << endl;
  }

  return 0;
}

