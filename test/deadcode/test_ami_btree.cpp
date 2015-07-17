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

// Test file for class btree.

#include "app_config.h"
#include <tpie/portability.h>
#include <tpie/tpie_log.h>
#include <tpie/coll.h>
#include <tpie/tpie.h>

#include <tpie/cpu_timer.h>
#include <tpie/btree.h>

#define SIZE_OF_STRUCTURE 128

using namespace tpie;
using namespace tpie::ami;
using namespace std;

// Key type.
typedef TPIE_OS_OFFSET bkey_t;

// Element type for the btree.
struct el_t {
  bkey_t key_;
  char data_[SIZE_OF_STRUCTURE - sizeof(bkey_t)];
  el_t(bkey_t k): key_(k) {}
  el_t() {}
};

struct key_from_el {
  bkey_t operator()(const el_t& v) const { return v.key_; }
};
// Temporary distinction btw UN*X and WIN, since there are some
// problems with the MMAP collection implementation.
#ifdef _WIN32
typedef btree< bkey_t,el_t,less<bkey_t>,key_from_el,bte::COLLECTION_UFS > u_btree_t;
#else
typedef btree< bkey_t,el_t,less<bkey_t>,key_from_el > u_btree_t;
#endif

typedef stream< el_t > stream_t;

// Template instantiations (to get meaningful output from gprof)
//template class btree_node<bkey_t,el_t,less<bkey_t>,key_from_el>;
//template class btree_leaf<bkey_t,el_t,less<bkey_t>,key_from_el>;
//template class btree< bkey_t,el_t,less<bkey_t>,key_from_el,BTE_COLLECTION_UFS >;
//template class STREAM< el_t >;


// This is 2**31-1, the max value returned by random().
#define MAX_RANDOM ((double)0x7fffffff)
// This is the max value that we want.
#define MAX_VALUE 1000000000
// The number of deletions to perform.
#define DELETE_COUNT 500

// Global variables.
TPIE_OS_OFFSET bulk_load_count;
TPIE_OS_OFFSET insert_count = 5000;
TPIE_OS_OFFSET range_search_lo = 0;
TPIE_OS_OFFSET range_search_hi = 10000000;

int main(int argc, char **argv) {

  TPIE_OS_OFFSET i;
  TPIE_OS_OFFSET j;
  el_t s[DELETE_COUNT], ss;
  cpu_timer wt;
  char *base_file = NULL;

  // Log debugging info from the application, but not from the library. 
  tpie_init();
 
  get_memory_manager().set_limit(64*1024*1024);
  get_memory_manager().set_enforcement(memory_manager::ENFORCE_THROW);

  if (argc > 1) {
    bulk_load_count = atol(argv[1]);
    if (argc > 2)
      base_file = argv[2];
  } else {
    std::cerr << "Usage: " << argv[0] << " <point_count> [base_file_name]\n";
    exit(1);
  }

  std::cout << "\n";
  std::cout << "Element size: " << sizeof(el_t) << " bytes. "
       << "Key size: " << sizeof(bkey_t) << " bytes.\n";
  TPIE_OS_SRANDOM((unsigned int)TPIE_OS_TIME(NULL));

  // Timing stream write.
  std::cout << "BEGIN Stream write\n";
  stream_t* is = (base_file == NULL) ? new stream_t:
    new stream_t(base_file);
  std::cout << "\tCreating stream with " << bulk_load_count << " random elements.\n";
  wt.start();
  for (j = 0; j < bulk_load_count; j++) {
    is->write_item(el_t(long((TPIE_OS_RANDOM()/MAX_RANDOM) * MAX_VALUE)));
  }
  wt.stop();
  std::cout << "END Stream write " << wt << "\n";
  wt.reset();


  //////  Testing Bulk loading. ///////

  u_btree_t *u_btree;
  btree_params params;
  params.node_block_factor = 1;
  params.leaf_block_factor = 1;
  params.leaf_cache_size = 32;
  params.node_cache_size = 64;

  u_btree = (base_file == NULL) ? new u_btree_t(params): 
    new u_btree_t(base_file, WRITE_COLLECTION, params);
  
  if (!u_btree->is_valid()) {
    std::cerr << argv[0] << ": Error initializing btree. Aborting.\n";
    delete u_btree;
    exit(1);
  }
  
  if (u_btree->size() == 0) {
    std::cout << "BEGIN Bulk Load\n";
    //    std::cout << "\tBulk loading from the stream created.\n";
    std::cout << "\tSorting... " << std::flush;
    wt.start();
    stream_t *os = new stream_t;
    if (u_btree->sort(is, os) != NO_ERROR) {
      std::cerr << argv[0] << ": Error during sort.\n";
    } else {
      wt.stop();
      std::cout << "Done. " << wt << "\n";
      wt.reset();
      wt.start();
      std::cout << "\tLoading... " << std::flush;
      if (u_btree->load_sorted(os) != NO_ERROR)
	cerr << argv[0] << ": Error during bulk loading.\n";
      else
	cout << "Done. " << wt << "\n";
      wt.stop();
    }
    os->persist(PERSIST_DELETE);
    delete os;
    ///    wt.stop();
    std::cout << "END Bulk Load " /*<< wt*/ << "\n";
  }

  delete is;

  std::cout << "Tree size: " << u_btree->size() << " elements. Tree height: " 
       << static_cast<TPIE_OS_OUTPUT_SIZE_T>(u_btree->height()) << ".\n";
  wt.reset();

  delete u_btree;

  //////  Testing insertion.  //////

  //  u_btree_t *u_btree;
  params.leaf_cache_size = 16;
  params.node_cache_size = 64;

  u_btree = (base_file == NULL) ? new u_btree_t(params): 
    new u_btree_t(base_file, WRITE_COLLECTION, params);

  if (!u_btree->is_valid()) {
    std::cerr << argv[0] << ": Error reinitializing btree. Aborting.\n";
    delete u_btree;
    exit(1);
  }

  std::cout << "BEGIN Insert\n";
  std::cout << "\tInserting " << insert_count << " elements.\n";
  std::cout << "\t" << std::flush;
  wt.start();
  for (i = 1; i <= insert_count; i++) {
    if (i <= DELETE_COUNT)
      s[i-1] = ss = el_t(i+100000);
    else
      ss = el_t(long((TPIE_OS_RANDOM()/MAX_RANDOM) * MAX_VALUE));
    u_btree->insert(ss);
    if (i % (insert_count/10) == 0)
      std::cout << i << " " << std::flush;
  }
  std::cout << "\n";
  wt.stop();
  std::cout << "END Insert " << wt << "\n";
  
  std::cout << "Tree size: " << u_btree->size() << " elements. Tree height: " 
       << static_cast<TPIE_OS_OUTPUT_SIZE_T>(u_btree->height()) << ".\n";
  wt.reset();


  //////  Testing range query.  ///////

  std::cout << "BEGIN Search\n";
  std::cout << "\tSearching with range [" << range_search_lo << ", " 
       << range_search_hi << "]\n";

  stream_t* os = new stream_t;
  wt.start();
  u_btree->range_query(range_search_lo, range_search_hi, os);
  wt.stop();
  std::cout << "\tFound " << os->stream_len() << " elements.\n";
  delete os;
  std::cout << "END Search " << wt << "\n";


  ///////  Testing erase.  ///////

  std::cout << "BEGIN Delete\n";
  std::cout << "\tDeleting " << key_from_el()(s[0]) << " through " 
       <<  key_from_el()(s[DELETE_COUNT-1]) << ": \n";
  j = 0;
  for (i = 0; i < DELETE_COUNT ; i++) {
    if (u_btree->erase(key_from_el()(s[i])))
      j++;
  }

  std::cout << "\t\tfound " << j << " keys. ";
  if (j == DELETE_COUNT)
    std::cout << "(OK)\n";
  else
    std::cout << "(Potential problem!)\n";
  
  std::cout << "\tDeleting " << (long)-1 << std::flush;
  if (u_btree->erase((long)-1))
    std::cout << ": found. (Potential problem!)\n";
  else
    std::cout << ": not found. (OK)\n";

  std::cout << "\tDeleting " <<  key_from_el()(s[0]) << std::flush;
  if (u_btree->erase(key_from_el()(s[0])))
    std::cout << ": found. (Potential problem!)\n";
  else
    std::cout << ": not found. (OK)\n";
  std::cout << "END Delete\n";
  

  std::cout << "Tree size: " << u_btree->size() << " elements. Tree height: " 
       << static_cast<TPIE_OS_OUTPUT_SIZE_T>(u_btree->height()) << ".\n";

  stats_tree bts = u_btree->stats();
  delete u_btree;
  
  std::cout << "Block collection statistics (global):\n"
       << "\tGET BLOCK:    "
       << collection_single<bte::COLLECTION>::gstats().get(BLOCK_GET) << "\n"
       << "\tPUT BLOCK:    "
       << collection_single<bte::COLLECTION>::gstats().get(BLOCK_PUT) << "\n"
       << "\tNEW BLOCK     "
       << collection_single<bte::COLLECTION>::gstats().get(BLOCK_NEW) << "\n"
       << "\tDELETE BLOCK: "
       << collection_single<bte::COLLECTION>::gstats().get(BLOCK_DELETE) << "\n"
    ;
  std::cout << "Tree statistics:\n"
       << "\tREAD (LEAF+NODE):    " 
       << bts.get(LEAF_READ) + bts.get(NODE_READ) << "\n"
       << "\tCREATE (LEAF+NODE):  " 
       << bts.get(LEAF_CREATE) + bts.get(NODE_CREATE) << "\n"
       << "\tFETCH (LEAF+NODE):   " 
       << bts.get(LEAF_FETCH) + bts.get(NODE_FETCH) << "\n"
       << "\tWRITE (LEAF+NODE):   " 
       << bts.get(LEAF_WRITE) + bts.get(NODE_WRITE) << "\n"
       << "\tDELETE (LEAF+NODE):  " 
       << bts.get(LEAF_DELETE) + bts.get(NODE_DELETE) << "\n"
       << "\tRELEASE (LEAF+NODE): " 
       << bts.get(LEAF_RELEASE) + bts.get(NODE_RELEASE) << "\n"
    ;
  std::cout << "Stream statistics (global):\n"
       << "\tREAD ITEM:    "
       << stream_t::gstats().get(ITEM_READ) << "\n"
       << "\tWRITE ITEM:   "
       << stream_t::gstats().get(ITEM_WRITE) << "\n"
       << "\tSEEK ITEM:    "
       << stream_t::gstats().get(ITEM_SEEK) << "\n"
       << "\tREAD BLOCK:   "
       << stream_t::gstats().get(BLOCK_READ) << "\n"
       << "\tWRITE BLOCK:  "
       << stream_t::gstats().get(BLOCK_WRITE) << "\n"
    ;

  return 0;
}
