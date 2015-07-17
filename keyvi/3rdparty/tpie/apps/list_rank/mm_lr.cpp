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

// Copyright (c) 1994 Darren Vengroff
//
// File: mm_lr.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/30/94
//

#include "app_config.h"
#include <ami.h>
VERSION(mm_lr_cpp,"$Id: mm_lr.cpp,v 1.4 2004-02-05 17:55:38 jan Exp $");

#include "list_edge.h"
#include "mm_lr.h"

// quick sort
#include <quicksort.h>

////////////////////////////////////////////////////////////////////////
// main_mem_list_rank()
//
// This function ranks a list that can fit in main memory.  It is used
// when the recursion bottoms out.
//
////////////////////////////////////////////////////////////////////////

int main_mem_list_rank(edge *edges, size_t count)
{
    edge *edges_copy;
    unsigned int ii,jj,kk;
    unsigned int head_index, tail_index;
    unsigned long int head_node, tail_node;
    long int total_weight;
    
    // Copy the array.
    edges_copy = new edge[count];
    for (ii = count; ii--; ) {
        edges_copy[ii] = edges[ii];
    }
    
	edgefromcmp efc;
    // Sort the original set by the from fields.
    quick_sort_obj(edges, count, &efc); 
    
	edgetocmp etc;
    // Sort the copy by to.
    quick_sort_obj(edges_copy, count, &etc); 

    // Find the head of this list, which is the unique node number
    // that appears in the list sorted by from but not by to.  At the
    // same time, we can find the tail.
    {
        bool head_found = false, tail_found = false;
        
        for (ii = kk = 0; (ii < count) || (kk < count); ) {
            tp_assert((ii - kk <= 1) || (kk - ii <= 1),
                      "Indeces more than 1 out of sync.");            
            if (edges[ii].from == edges_copy[kk].to) {
                ii++; kk++;
            } else if (ii == count) {
                tp_assert(head_found, "We should have found the head by now.");
                tp_assert(!tail_found, "We already found the tail.");
                tp_assert(kk == count-1, "kk is too far behind.");
                tail_index = kk;
                tail_node = edges_copy[kk++].to;
                tail_found = true;
                break;
            } else if (kk == count) {
                tp_assert(tail_found, "We should have found the head by now.");
                tp_assert(!head_found, "We already found the tail.");
                tp_assert(ii == count-1, "ii is too far behind.");
                head_index = ii;
                head_node = edges[ii++].to;
                head_found = true;
                break;
            } else if (edges[ii].from < edges_copy[kk].to) {
                // ii is the index of the head of the list.
                tp_assert(!head_found, "We already found the head.");
                head_index = ii;
                head_node = edges[ii++].from;
                if ((head_found = true) && tail_found)
                    break;
            } else if (edges[ii].from > edges_copy[kk].to) {
                // kk is the index of the tail of the list.
                tp_assert(!tail_found, "We already found the tail.");
                tail_index = kk;
                tail_node = edges_copy[kk++].to;
                if ((tail_found = true) && head_found)
                    break;
            }
        }

        tp_assert(head_found, "Head of list not found.");
        tp_assert(tail_found, "Tail of list not found.");
    }

    // Reduce the to fields to integers in the range [0...count-1].
    // After this is done, we will resort by source, and then we can
    // walk through the list starting at head_index.

    // There are two problems, however, one related to each end of the
    // list.  These problems arise from the fact that what we are
    // actually indexing into is an array of edges, not of nodes.  One
    // node, the head, appears only once, as a source.  Another node,
    // the tail, appears only once, as a destination.  All other nodes
    // appear exactly twice, once as a source and once as a
    // destination.

    // The first edge (the one whose source is the head) of the list
    // will be embedded somewhere in the middle of the array.  If k is
    // smaller than the index of the head edge in the array sorted by
    // source then the k'th largest destination is index k - 1.  If,
    // however, k is greater than or equal to the index of the head,
    // then the k'th largest destination is index k.  This is because
    // there is no edge whose destination is the head of the list.

    // The tail of the list it the i'th largest destination for some
    // value of i.  This means that for j >= 0, the (i + j)'th
    // destination is actually the source of the edge in position (i +
    // j - 1) when the edges are sorted by source.

    // The situation is even a slight bit more complicated than it is
    // described in the preceding paragraphs, since the two end
    // effects can interact.  Luckily, however, their effects are
    // simply additive.
    
    for (ii = count; ii--; ) { 
        edges_copy[ii].to = ii;
        if (ii >= head_index) {
            edges_copy[ii].to++;
        }
        if (ii >= tail_index) {
            edges_copy[ii].to--;
        }            
    }

    // Sort the copy back by source edge.
    
    quick_sort_obj(edges_copy, count, &efc); 

    // Traverse the reduced copy by taking count - 1 steps, starting
    // from the index of the head.  We use jj to keep track of the
    // number of iterations.
    for (ii = head_index, jj = count, total_weight = 0;
         jj--; ii = edges_copy[ii].to) {

        tp_assert(ii < count, "ii (= " << ii <<
                  ") out of range (jj = " << jj << ").");

        tp_assert((ii != head_index) || !total_weight || (ii == tail_node),
                  "Cycled back to the head somehow (jj = " << jj << ").");
        
        // The original array edges is sorted by source node, thus the
        // edges are in the same positions within the array as the
        // reduced copies in edge_copy.  All we have to to is add the
        // weight of all edges before this one to this one.

        total_weight = (edges[ii].weight += total_weight);
        
    }

    // We should have ended up at the tail of the list.
    tp_assert(ii = tail_node,
              "Did not end up at the tail of the list.");
    
    // Free the copy.
    delete edges_copy;

    // Done 
    return 0;  
}

