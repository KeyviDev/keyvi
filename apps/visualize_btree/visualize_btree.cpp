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

// Copyright (C) 2003 Octavian Procopiuc
//
// File:    visualize_btree.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
// Created: May 2003 
//
// $Id: visualize_btree.cpp,v 1.4 2005-01-21 17:16:48 tavi Exp $
//
#include <portability.h>
#include <versions.h>
VERSION(visualize_btree_cpp, "$Id: visualize_btree.cpp,v 1.4 2005-01-21 17:16:48 tavi Exp $");                            

// For std::less.
#include <functional>
// For std::max.
#include <algorithm>
// For std::stack.
#include <stack>

#include "app_config.h"
#include <cpu_timer.h>
#include <ami_btree.h>
#include "getopts.h"

/*
using std::ifstream;
using std::less;
using std::cerr;
using std::cout;
using std::flush;
using std::max;
using std::pair;
using std::stack;
*/

template <class T>
struct ident {
  const T& operator()(const T& t) const { return t; }
};

typedef AMI_btree<long, long, less<long>, ident<long> > btree_t;

struct options opts[] = {
  {1, "input-file", "Input file name (containing space-separated ints)", "i", 1},
  {5, "btree-name", "Read/Write b-tree using the given name", NULL, 1},
  {10, "node-size-max", "Max fanout of a node", "n", 1},
  {11, "leaf-size-max", "Max number of values in a leaf", "l", 1},
  {12, "bulk-load", "Build the btree using bulk loading rather than repeated insertions", "b", 0}, 
  {14, "insert", "Insert space-separated ints", NULL, 1},
  {20, "print-HTML", "Print HTML instead of plain ASCII", "H", 0},
  { 0,  NULL, NULL, NULL, 0 }
};


int main(int argc, char **argv) {
  char input_file[128];
  char btree_name[128];
  char *args;
  char *extra_inserts;
  int idx;
  int html = 0; // whether to print html or simple ASCII.
  bool do_bulk_load = false;
  bool do_keep = false;
  bool do_extra_inserts = false;
  bool do_input_file = false;
  btree_t *btree;
  TPIE_OS_OFFSET input_size = 0;
  AMI_btree_params params;
  params.node_block_factor = 1;
  params.leaf_block_factor = 1;
  params.leaf_cache_size = 32;
  params.node_cache_size = 32;
  params.node_size_max = 0; // default.
  params.leaf_size_max = 100;
  if (argc == 1) {
    getopts_usage(argv[0], opts);
    exit(1);
  }

  while((idx = getopts(argc, argv, opts, &args)) != 0) {
    switch (idx) {
    case 1:
      strncpy(input_file, args, 128);
      do_input_file = true;
      break;
    case 5:
      strncpy(btree_name, args, 128);
      do_keep = true;
      break;
    case 10: 
      params.node_size_max = max(3, atoi(args) - 1);
      break;
    case 11: 
      params.leaf_size_max = max(2, atoi(args));
      break;
    case 12:
      do_bulk_load = true;
      break;
    case 14:
      extra_inserts = new char[strlen(args) + 1];
      strcpy(extra_inserts, args);
      do_extra_inserts = true;
      break;
    case 20:
      html = 1;
      break;
    }
    free(args);
  }


  // Open the B-tree.
  btree = (do_keep ? new btree_t(btree_name, AMI_WRITE_COLLECTION, params) 
	   : new btree_t(params));

  if (!btree->is_valid()) {
    delete btree;
    cout << "Error encountered while opening B-tree.\n";
    return 1;
  }

  input_size = btree->size();

  // Test for existing b-tree.
  if (do_input_file) {

    if (btree->size() > 0) {
      cerr << "Specified btree is not empty. Ignoring input file request.\n"; 

    } else {

      ifstream ifs(input_file);
      if (!ifs) {
	cout << "Error opening input file. B+-tree was not created.\n";
	return 1;
      }

      // Build the btree from the elements in ifs.
      long elem;
      bool error = false;
      
      if (do_bulk_load) {
	
	AMI_STREAM<long>* its = new AMI_STREAM<long>;
	if (!its->is_valid())
	  error = true;
	error = !(ifs >> elem);
	while (!ifs.eof() && !error) {
	  its->write_item(elem);
	  input_size++;
	  // Signal an error other than eof.
	  error = !(ifs >> elem) && !ifs.eof();
	}
	// Bulk load with 100% fill factors.
	if (!error)
	  btree->load(its, 1.0, 1.0);
	delete its;
	
      } else { //repeated insertions
	
	error = !(ifs >> elem);
	while (!ifs.eof() && !error) {
	  btree->insert(elem);
	  input_size++;
	  // Signal an error other than eof.
	  error = !(ifs >> elem) && !ifs.eof();
	}
      }
      
      if (error) {
	delete btree;
	cout << "Error encountered while reading input file. B+-tree was not created.\n";
	return 1;
      }
    }
    
  } 

  btree_t *extra_btree = NULL;

  if (do_extra_inserts) {
    long elem;
    bool error;
    // Another B-tree, containing just the extra inserts.
    extra_btree = new btree_t;
    istringstream iss(extra_inserts);
    error = !(iss >> elem);
    while (!error) {
      btree->insert(elem);
      extra_btree->insert(elem);
      error = !iss.eof() && !(iss >> elem);
    }
  }


  // Print the B-tree.

  long elem;
  int prevlevel = 0;
  int level = -1;
  TPIE_OS_SIZE_T height = btree->height();
  TPIE_OS_SIZE_T fc = max(0, int(9 - height)); // index of first color.
  float avg_leaf_size = 0.0;
  float avg_fanout = 0.0;
  pair<AMI_bid, long> idkey = btree->dfs_preorder(level);
  stack<AMI_bid> stk;
	int i;

  cout << "\n";

  if (html) {
    cout << "<table cellpadding=2 cellspacing=0 width=100%>\n<tr>";
    if (btree->height() > 1) {
      cout << "<th class=nodeheader colspan=" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(height-1) << ">Internal nodes</th>";
      cout << "<th class=leafheader>Leaf nodes</th>";
    } else {
      cout << "<th class=leafheader>Leaf node</th>";
    }
    cout << "</tr>\n";
  }

  while (level != -1) {
    if (level <= prevlevel) { // new row
      // Insert an empty row when backtracking.
      if (prevlevel != 0) {
	if (html) {
	  cout << "<tr>";
	  for (i = 0; i < level - 1; i++)
	    cout << "<td class=node" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(i+fc) << "></td>";
	  cout << "<td class=key" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(level-1+fc) << ">" << idkey.second << "</td>";
	  for (i = level; i < btree->height(); i++)
	    cout << "<td></td>";
	  cout << "</tr>\n";
	} else
	  cout << "\n";
      }
      // Start the row.
      if (html) cout << "<tr>";
      // Insert some empty cells.
      for (i = 0; i < level; i++) {
	if (html)
	  cout << "<td class=node" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(i+fc) << "></td>";
	else
	  cout << "\t";
      }
    } else {// continue the same row

    } 
    if (!html)
      cout << "\t";

    while (stk.size() > level)
      stk.pop();
    
    // Test for (not) leaf.
    if (level < height - 1) { // it's a node

      if (html) {
	// Write the td element (title and style).
	btree_t::node_t* bn = btree->fetch_node(idkey.first);
	avg_fanout += (float) (bn->size() + 1) / btree->node_count();
	cout << "<td title=\"node fanout: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(bn->size() + 1) 
	     << "\" class=node" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(level + fc) << ">";
	cout << "<a id=\"ID" << idkey.first << "\"></a>";
	btree->release_node(bn);

	// Write the id with links to siblings.
	if (stk.size() > 0) {
	  bn = btree->fetch_node(stk.top());
	  size_t i;
	  for (i = 0u; i < bn->size() + 1; i++)
	    if (bn->lk[i] == idkey.first)
	      break;
	  if (i > 0)
	    cout << "<a href=\"#ID" << bn->lk[i-1] << "\">[</a>";
	  else
	    cout << "[";
	  cout << idkey.first;
	  if (i < bn->size())
	    cout << "<a href=\"#ID" << bn->lk[i+1] << "\">]</a>";
	  else
	    cout << "]";
	  btree->release_node(bn);
	} else // stk.size() == 0, write the root id.
	  cout << "[" << idkey.first << "]";
      } else // !html
	cout << "[" << idkey.first << "]";

      if (html)
	cout << "</td>";
    } else { // it's a leaf

      btree_t::leaf_t* bl = btree->fetch_leaf(idkey.first);
      ///      bl->sort();
      avg_leaf_size += (float) bl->size() / btree->leaf_count();
      size_t i;
      if (html)
	cout << "<td title=\"leaf size: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(bl->size()) 
	     << "\" class=leaf" << ">";
      for (i = 0u; i < bl->size(); i++) {
	if (html && extra_btree && extra_btree->find(bl->el[i], elem))
	  cout << "<b>" <<  bl->el[i] << "</b> ";
	else
	  cout << bl->el[i] << " ";
      }
      if (html)
	cout << "</td></tr>";
      cout << "\n";
      btree->release_leaf(bl);
    }

    stk.push(idkey.first);
    prevlevel = level;
    idkey = btree->dfs_preorder(level);
  }
  if (html) cout << "</table>";
  cout << "\n";

  if (html) cout << "<div id=param>";
  if (html) cout << "<strong>Input Parameters</strong><br>\n";
  cout << " Size: " << input_size << " records\n";
  if (html) cout << "<br>"; 
  cout << " Max fanout: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(btree->params().node_size_max + 1) << "\n";
  if (html) cout << "<br>"; 
  cout << " Max leaf size: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(btree->params().leaf_size_max) << "\n";
  if (html) cout << "<br>"; 
  cout << " Build method: " 
       << (do_bulk_load ? "bulk load 100%": "repeated insertion") << "\n";

  if (html) cout << "<br><br>";
  if (html) cout << "<strong>Output Statistics</strong><br>\n";

  cout << " Size: " << btree->size() << " records\n";
  if (html) cout << "<br>"; 
  cout << " Avg fanout: " << avg_fanout << "\n";
  if (html) cout << "<br>"; 
  cout << " Avg leaf size: " << avg_leaf_size << "\n";
  if (html) cout << "<br>";
  cout << " Internal nodes: " << btree->node_count() << "\n";
  if (html) cout << "<br>"; 
  cout << " Leaf nodes: " << btree->leaf_count() << "\n";
  if (html) cout << "<br>"; 
  cout << " Height: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(height) << "\n";
  if (html) cout << "</div>\n"; 

  delete extra_btree;
  delete btree;
  return 0;
}
