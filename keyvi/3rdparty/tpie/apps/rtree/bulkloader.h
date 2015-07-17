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

// -*- C++ -*-
//
//  Description:     declarations for class BulkLoader
//  Created:         27.01.1999
//  Author:          Jan Vahrenhold
//  mail:            jan@math.uni-muenster.de
//  $Id: bulkloader.h,v 1.4 2004-08-12 18:02:42 jan Exp $
//  Copyright (C) 1999-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

//  Prevent multiple #includes.
#ifndef _TPIE_AMI_BULKLOADER_H
#define _TPIE_AMI_BULKLOADER_H

#include <tpie/portability.h>

#include <cassert>        //  Debugging.
#include <cmath>          //  For log, pow, etc.
#include <iostream>       //  Debugging output.
#include <fstream>        //  Reading the MBR metafile.
#include <cstdlib>        //  strlen / strcpy
#include <tpie/scan.h>    //  for scan 
#include <tpie/sort.h>    //  for sort
#include "rectangle.h"    //  Data.
#include "rstartree.h"    //  Output data.
#include "rstarnode.h"    //  Needed while bulk loading.
#include "hilbert.h"      //  Computing Hilbert values.

#include "scan_boundingbox.h"

namespace tpie {

    namespace ami {
	
	////////////////////////////////////////////////////////////////////////////
        /// Two mechanisms for bulk loading R-trees from a given stream 
        /// of rectangles.
        ////////////////////////////////////////////////////////////////////////////
	template<class coord_t, class BTECOLL = bte::COLLECTION>
	class bulkloader {

	public:

	    ////////////////////////////////////////////////////////////////////////
	    /// The constructor requires the name of the input stream of rectangles.
	    /// If no fanout is given, a value of 50 is assumed.
	    /// \param[in] input_stream name of the input stream of rectangles
	    /// \param[in] fanout maximum fanout for the tree.
	    ////////////////////////////////////////////////////////////////////////
	    bulkloader(
		const std::string& input_stream,
		children_count_t   fanout = 50);

	    ////////////////////////////////////////////////////////////////////////
	    /// Nothing is done here.
	    ////////////////////////////////////////////////////////////////////////
	    ~bulkloader();
	    
	    ////////////////////////////////////////////////////////////////////////
	    /// Set the name of the input stream of rectangles.
	    /// \param[in] input_stream name of the input stream of rectangles
	    ////////////////////////////////////////////////////////////////////////
	    void set_input_stream(const std::string& input_stream);

	    ////////////////////////////////////////////////////////////////////////
	    /// Get the name of the input stream of rectangles.
	    ////////////////////////////////////////////////////////////////////////
	    const std::string& get_input_stream() const;

	    ////////////////////////////////////////////////////////////////////////
	    /// Set the maximum fanout for the tree to be constructed.
	    ////////////////////////////////////////////////////////////////////////
	    void set_fanout(children_count_t fanout);

	    ////////////////////////////////////////////////////////////////////////
	    /// Get the maximum fanout for the tree to be constructed.
	    ////////////////////////////////////////////////////////////////////////
	    children_count_t get_fanout() const;

	    ////////////////////////////////////////////////////////////////////////
	    /// Print statistics of the tree (if available).
	    ////////////////////////////////////////////////////////////////////////
	    void show_stats();

	    ////////////////////////////////////////////////////////////////////////
	    /// Create a Hilbert tree by sorting all rectangles according to the
	    /// Hilbert value of the center and the building bottom-up.
	    ////////////////////////////////////////////////////////////////////////
	    err create_hilbert_rtree(rstartree<coord_t, BTECOLL>** tree);

	    ////////////////////////////////////////////////////////////////////////
	    /// Create a Hilbert tree by sorting all rectangles according to the
	    /// Hilbert value of the center and the building bottom-up. Additionally,
	    /// perform repacking as suggested by DeWitt et al. for the "Paradise"
	    ///  system [DKL+94].
	    ////////////////////////////////////////////////////////////////////////
	    err create_rstartree(rstartree<coord_t, BTECOLL>** tree);

	protected:   

	    ////////////////////////////////////////////////////////////////////////
	    /// Choose the best split axis and index for R*-tree node 
	    /// splitting [BKSS90]
	    /// \param[in] sorted_vector0 data sorted along the x-axis
	    /// \param[in] sorted_vector1 data sorted along the y-axis
	    ////////////////////////////////////////////////////////////////////////
	    std::pair< std::vector<rectangle<coord_t, bid_t> >*, children_count_t>  choose_split_axis_and_index(
		std::vector<rectangle<coord_t, bid_t> >* sorted_vector0, 
		std::vector<rectangle<coord_t, bid_t> >* sorted_vector1);
	    
	    ////////////////////////////////////////////////////////////////////////
	    /// Caching and repacking as proposed by DeWitt et al. [DKL+94]
	    /// \param[out] last_node pointer to the last non-full node after repacking
	    ////////////////////////////////////////////////////////////////////////
	    void repack_cached_nodes(rstarnode<coord_t, BTECOLL>** last_node);
	    
	    ////////////////////////////////////////////////////////////////////////
	    /// Try to get the minimum bounding rectangle of a stream of rectangles
	    /// using the ".mbr" meta file.
	    /// \param[in] input_filename name of the input stream
	    /// \param[out] mbr pointer to minimum bounding rectangle
	    ////////////////////////////////////////////////////////////////////////
	    void get_minimum_bounding_rectangle(
		const std::string&          input_filename, 
		rectangle<coord_t, bid_t>** mbr);
	    
	private:

	    std::string      input_stream_; //  Name of the input stream of rectangles.
	    children_count_t fanout_;       //  Fan-out for the R-tree.

	    coord_t          xOffset_;      //  Minimun x-coordinate of data set.
	    coord_t          yOffset_;      //  Minimun y-coordinate of data set.
	    TPIE_OS_LONGLONG factor_;       //  Scaling factor for coordinates.
	    TPIE_OS_LONGLONG size_;         //  Size of the integer grid.

	    rstartree<coord_t, BTECOLL>*     tree_;         //  Output object.
	    bool             statistics_;   //  Whether to record stats or not.
	    
	    std::priority_queue<std::pair<rstarnode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>, std::vector<std::pair<rstarnode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG> >, hilbert_priority<coord_t, BTECOLL> >   cachedNodes_; //  Cache

	    bulkloader(const bulkloader& other);
	    bulkloader& operator=(const bulkloader& other);
	};
	
	template<class coord_t, class BTECOLL>
	inline void bulkloader<coord_t, BTECOLL>::set_input_stream(const std::string& input_stream) {	    
	    input_stream_ = input_stream;
	}
	
	template<class coord_t, class BTECOLL>
	inline const std::string& bulkloader<coord_t, BTECOLL>::get_input_stream() const {
	    return input_stream_;
	}
	
	template<class coord_t, class BTECOLL>
	inline void bulkloader<coord_t, BTECOLL>::set_fanout(children_count_t fanout) {
	    if (fanout > ((LOG_BLK_SZ - (2*sizeof(bid_t) + 2*sizeof(children_count_t)))/ sizeof(rectangle<coord_t, bid_t>))) {
		std::cerr << "Warning: fan-out too big (" 
			  << static_cast<TPIE_OS_OUTPUT_SIZE_T>(fanout) << " > " 
			  << (LOG_BLK_SZ - (2*sizeof(bid_t) + 2*sizeof(children_count_t)))/ sizeof(rectangle<coord_t, bid_t>) 
			  <<  ") !" << std::endl;
	    }
	    fanout_ = fanout;    
	}
	
	template<class coord_t, class BTECOLL>
	inline children_count_t bulkloader<coord_t, BTECOLL>::get_fanout() const {
	    return fanout_;
	}
	
	
	template<class coord_t, class BTECOLL>
	inline void bulkloader<coord_t, BTECOLL>::show_stats() {
	    if (tree_) {
		tree_->show_stats();
	    }
	}
	
	
	template<class coord_t, class BTECOLL>
	bulkloader<coord_t, BTECOLL>::bulkloader(const std::string& input_stream, 
						 children_count_t   fanout) 
	    : input_stream_(NULL), fanout_(0),  
	      xOffset_((coord_t) 0), yOffset_((coord_t) 0), 
	      factor_(0), size_(0), tree_(NULL), 
	      statistics_(false), cachedNodes_() {

	    set_input_stream(input_stream);   
	    set_fanout(fanout);

	}
		
	template<class coord_t, class BTECOLL>
	bulkloader<coord_t, BTECOLL>::~bulkloader() {
	    //  Do nothing.
	}
	
// Try to get the mbr of a stream of rectangles
// using the ".mbr" meta file.
	template<class coord_t, class BTECOLL>
	void bulkloader<coord_t, BTECOLL>::get_minimum_bounding_rectangle(
	    const std::string&          input_filename, 
	    rectangle<coord_t, bid_t>** mbr) {
	    
	    // Read the mbr.
	    std::string metadata_filename = input_filename + ".mbr";
	    std::ifstream *mbr_file_stream = new std::ifstream(metadata_filename.c_str());
	    if (!(*mbr_file_stream)) {
		*mbr = NULL;
	    }
	    else {
		*mbr = new rectangle<coord_t, bid_t>;
		mbr_file_stream->read((char *) *mbr, sizeof(rectangle<coord_t, bid_t>));
	    }
	    
	    delete mbr_file_stream;
	}
	
//  Choosing the best split axis and index for R*-tree node splitting
	template<class coord_t, class BTECOLL>
	std::pair<std::vector<rectangle<coord_t, bid_t> >*, children_count_t>  bulkloader<coord_t, BTECOLL>::choose_split_axis_and_index(std::vector<rectangle<coord_t, bid_t> >* sortedVector0, std::vector<rectangle<coord_t, bid_t> >* sortedVector1) {

	    std::vector<rectangle<coord_t, bid_t> >* sortedVector[2];
	    children_count_t counter;
	    children_count_t counter2;
	    
	    coord_t S[2];
	    
	    sortedVector[0] = sortedVector0;
	    sortedVector[1] = sortedVector1;
	    
	    //  Sort vectors according to the coordinates.
	    std::sort(sortedVector[0]->begin(), sortedVector[0]->end(), 
		      sort_boxes_along_x_axis<coord_t>());
	    std::sort(sortedVector[1]->begin(), sortedVector[1]->end(), 
		      sort_boxes_along_y_axis<coord_t>());
	    
	    children_count_t children_ = static_cast<children_count_t>(sortedVector[1]->size());
	    
	    children_count_t firstGroupMinSize = (children_count_t)(fanout_ / MIN_FANOUT_FACTOR);
	    children_count_t distributions = fanout_ - 2*firstGroupMinSize + 1;
	    
	    //  area-value:    area[bb(first group)] +
	    //                 area[bb(second group)]
	    //  margin-value:  margin[bb(first group)] +
	    //                 margin[bb(second group)]
	    //  overlap-value: area[bb(first group) \cap bb(second group)]
	    
	    VarArray2D<coord_t> areaValue(2,distributions);
	    VarArray2D<coord_t> marginValue(2,distributions);
	    VarArray2D<coord_t> overlapValue(2,distributions);
	    
	    //  "For each axis 
	    //     Sort the entries by their lower then by their upper 
	    //     value of their rectangles and determine all 
	    //     distributions as described above. Compute S, the
	    //     sum of all margin-values of the different 
	    //     distributions.
	    //   end."
	    
	    rectangle<coord_t, bid_t> group[2];
	    rectangle<coord_t, bid_t> firstGroup;
	    rectangle<coord_t, bid_t> secondGroup;
	    
	    //  Process x-axis.
	    S[0] = (coord_t) 0;
	    
	    firstGroup = (*sortedVector[0])[0];
	    secondGroup = (*sortedVector[0])[children_-1];
	    
	    //  The first group contains at least the first "firstGroupMinSize" 
	    //  boxes while the second group contains at least the last
	    //  "firstGroupMinSize" boxes. This is true for all distributions.
	    for (counter = 1; counter < firstGroupMinSize; ++counter) {
		firstGroup.extend((*sortedVector[0])[counter]);	
		secondGroup.extend((*sortedVector[0])[(children_-1)-counter]);
	    }
	    
	    //  Iterate over all possible distributions.
	    for (counter = 0; counter < distributions; ++counter) {
		
		//  Initialize groups.
		group[0] = firstGroup;
		group[1] = secondGroup;
		
		//  Update first group.
		for (counter2 = firstGroupMinSize; counter2 < firstGroupMinSize+counter; ++counter2) {
		    group[0].extend((*sortedVector[0])[counter2]);
		} 
		
		//  Update second group.
		for (counter2 = (firstGroupMinSize + counter); counter2 < children_; ++counter2) {
		    group[1].extend((*sortedVector[0])[counter2]);
		}
		
		//  Compute area-value, margin-value and overlap-value.
		areaValue(0,counter) = group[0].area() + group[1].area();
		marginValue(0,counter) = group[0].width() + group[0].height()+
		    group[1].width() + group[1].height();
		overlapValue(0,counter) = group[0].overlap_area(group[1]);
		
		//  Update S.
		S[0] += marginValue(0,counter);
	    }
	    
	    //  Process y-axis.
	    S[1] = (coord_t) 0;
	    
	    firstGroup = (*sortedVector[1])[0];
	    secondGroup = (*sortedVector[1])[children_-1];
	    
	    //  The first group contains at least the first "firstGroupMinSize" 
	    //  boxes while the second group contains at least the last
	    //  "firstGroupMinSize" boxes. This is true for all distributions.
	    for (counter = 1; counter < firstGroupMinSize; ++counter) {
		firstGroup.extend((*sortedVector[1])[counter]);	
		secondGroup.extend((*sortedVector[1])[(children_-1)-counter]);
	    }
	    
	    //  Iterate over all possible distributions.
	    for (counter = 0; counter < distributions; ++counter) {
		
		//  Initialize groups.
		group[0] = firstGroup;
		group[1] = secondGroup;
		
		//  Update first group.
		for (counter2 = firstGroupMinSize; counter2 < firstGroupMinSize+counter; ++counter2) {
		    group[0].extend((*sortedVector[1])[counter2]);
		} 
		
		//  Update second group.
		for (counter2 = (firstGroupMinSize + counter); counter2 < children_; ++counter2) {
		    group[1].extend((*sortedVector[1])[counter2]);
		}
		
		
		//  Compute area-value, margin-value and overlap-value.
		areaValue(1,counter) = group[0].area() + group[1].area();
		marginValue(1,counter) = group[0].width() + group[0].height()+
		    group[1].width() + group[1].height();
		overlapValue(1,counter) = group[0].overlap_area(group[1]);

		//  Update S.
		S[1] += marginValue(1,counter);
	    }
	    
	    
	    //  "Choose the axis with the minimum S as split axis."
	    unsigned short splitAxis = 0;
	    children_count_t bestSoFar = 0;
	    
	    if (S[1] < S[0]) {
		splitAxis = 1;
	    }
	    
	    //  "Along the chosen split axis, choose the
	    //   distribution with the minimum overlap-value.
	    //   resolve ties by choosing the distribution with
	    //   minimum area-value."
	    
	    for(counter = 1; counter < distributions; ++counter) {
		if ((overlapValue(splitAxis,counter) < overlapValue(splitAxis,bestSoFar)) ||
		    ((overlapValue(splitAxis,counter) == overlapValue(splitAxis,bestSoFar)) && 
		     (areaValue(splitAxis,counter) < areaValue(splitAxis,bestSoFar)))) {
		    bestSoFar = counter;
		}
	    }
	    
	    return std::pair<std::vector<rectangle<coord_t, bid_t> >*, children_count_t>(sortedVector[splitAxis], bestSoFar);
	}
	
//  Caching and repacking as proposed by DeWitt et al.
	template<class coord_t, class BTECOLL>
	void bulkloader<coord_t, BTECOLL>::repack_cached_nodes(
	    rstarnode<coord_t, BTECOLL>** lastNode) {
	    
	    children_count_t counter  = 0;
	    
	    rstarnode<coord_t, BTECOLL>*  newNode = NULL;
	    rectangle<coord_t, bid_t>     newBB = (*lastNode)->get_child(0);
	    
	    for(counter = 1; counter < (*lastNode)->children(); ++counter) {
		newBB.extend((*lastNode)->get_child(counter));
	    }
	    
	    //  Translate the rectangle by the given offset and
	    //  compute the midpoint in scaled integer coordinates.
	    TPIE_OS_LONGLONG x = factor_ * 
		(TPIE_OS_LONGLONG)((newBB.get_left() + newBB.get_right()) / 2.0 - xOffset_);
	    TPIE_OS_LONGLONG y = factor_ * 
		(TPIE_OS_LONGLONG)((newBB.get_lower() + newBB.get_upper()) / 2.0 - yOffset_);
	    
	    //  Compute and set the Hilbert value.
	    TPIE_OS_LONGLONG hv = compute_hilbert_value(x, y, size_);
	    
	    //  Add the last node to the stream.
	    cachedNodes_.push(std::pair<rstarnode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>(*lastNode, hv));
	    *lastNode = NULL;
	    
	    //  Wait until there are three or more entries in the cache.
	    if (cachedNodes_.size() < 3) {
		return;
	    }
	    
	    std::vector<rectangle<coord_t, bid_t> >* toSort[2];
	    typename std::vector<rectangle<coord_t, bid_t> >::iterator vi;
	    
	    toSort[0] = new std::vector<rectangle<coord_t, bid_t> >;
	    toSort[1] = new std::vector<rectangle<coord_t, bid_t> >;

	    //  Create two arrays containing the boxes to be distributed,
	    //  one will be sorted according to the x-values, the other
	    //  will be sorted according to the y-values.
	    while(!cachedNodes_.empty()) {
		rstarnode<coord_t, BTECOLL>* cachedNode_ = cachedNodes_.top().first;
		for (counter = 0; counter < cachedNode_->children(); ++counter) {
		    toSort[0]->push_back(cachedNode_->get_child(counter));
		    toSort[1]->push_back(cachedNode_->get_child(counter));
		}
		cachedNode_->persist(PERSIST_DELETE);
		delete cachedNode_;
		cachedNodes_.pop();
	    }

	    std::vector<rectangle<coord_t, bid_t> >* backup = NULL;
	    
	    children_count_t exitLoop = 0;
	    
	    //  Repeat the splitting of the nodes as long as there is no
	    //  proper distribution, i.e., as long as there is no small enough
	    //  node.
	    do {
		
		std::pair<std::vector<rectangle<coord_t, bid_t> >*, children_count_t> seeds;
		children_count_t toDelete         = 0;
		children_count_t firstGroupNumber = 0;
		
		//  Initialize backup vector.
		backup   = NULL;
		exitLoop = 0;
		
		do {
		    
		    //  Determine best split axis and distribution.
		    seeds = choose_split_axis_and_index(toSort[0], toSort[1]);
		    
		    if (seeds.first == toSort[0]) {
			toDelete = 1;
		    } 
		    else {
			toDelete = 0;
		    }
		    
		    delete toSort[toDelete];
		    toSort[toDelete] = new std::vector<rectangle<coord_t, bid_t> >;
		    
		    //  Compute the index for splitting the vector.
		    firstGroupNumber = (children_count_t)(fanout_ / MIN_FANOUT_FACTOR) + seeds.second;
		    
		    //  Check if (a) the first section or (b) the second section
		    //  is small enough to for a node.
		    if (firstGroupNumber < fanout_) {                           //  (a)
			exitLoop += 1;
		    }
		    if ((seeds.first)->size() - firstGroupNumber < fanout_) {   //  (b)
			exitLoop += 2;
		    }
		    
		    if (!exitLoop) {
			
			assert(backup == NULL);
			
			//  Create new backup vector.
			backup = new std::vector<rectangle<coord_t, bid_t> >;
			
			//  Proceed with the smaller part of the distribution.
			//  This is to ensure that we will have a proper node
			//  after the next iteration.
			if (firstGroupNumber < (seeds.first)->size() - firstGroupNumber) {
			    //  Copy the smaller part of the distribution.
			    for(counter = 0; counter < firstGroupNumber; ++counter) {
				toSort[toDelete]->push_back((*(seeds.first))[counter]);
			    }
			    
			    //  Create a backup vector for remaining entries.
			    for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
				backup->push_back((*(seeds.first))[counter]);
			    }
			    
			    //  Create new vector.
			    delete toSort[1-toDelete];
			    toSort[1-toDelete] = new std::vector<rectangle<coord_t, bid_t> >;
			    
			    //  Copy entries from first vector (smaller part).
			    for(vi = toSort[toDelete]->begin(); vi != toSort[toDelete]->end(); ++vi) {
				toSort[1-toDelete]->push_back(*vi);
			    }		    
			}
			else {
			    //  Copy the smaller part of the distribution.
			    for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
				toSort[toDelete]->push_back((*(seeds.first))[counter]);
			    }
			    
			    //  Create a backup vector for remaining entries.
			    for(counter = 0; counter < firstGroupNumber; ++counter) {
				backup->push_back((*(seeds.first))[counter]);
			    }
			    
			    //  Create new vector.
			    delete toSort[1-toDelete];
			    toSort[1-toDelete] = new std::vector<rectangle<coord_t, bid_t> >;
			    
			    //  Copy entries from first vector (smaller part).
			    for(vi = toSort[toDelete]->begin(); vi != toSort[toDelete]->end(); ++vi) {
				toSort[1-toDelete]->push_back(*vi);
			    }		    
			} // else
		    }
		    
		} while (!exitLoop);
		
		//  Check if the first part of the distribution creates a new node.
		if (exitLoop & 1) {
		    
		    //  Create node for the first part of the distribution.
		    newNode = tree_->read_node(NEXT_FREE_BLOCK);
		    newBB   = (*(seeds.first))[0];
		    
		    //  Copy entries to the new node.
		    for(counter = 0; counter < firstGroupNumber; ++counter) {
			newNode->add_child((*(seeds.first))[counter]);
			newBB.extend((*(seeds.first))[counter]);
		    }
		    
		    //  Translate the rectangle by the given offset and
		    //  compute the midpoint in scaled integer coordinates.
		    x = factor_ * 
			(TPIE_OS_LONGLONG)((newBB.get_left() + newBB.get_right()) / 2.0 - xOffset_);
		    y = factor_ * 
			(TPIE_OS_LONGLONG)((newBB.get_lower() + newBB.get_upper()) / 2.0 - yOffset_);
		    
		    //  Compute and set the Hilbert value.
		    hv = compute_hilbert_value(x, y, size_);
		    
		    //  Cache the new node.
		    cachedNodes_.push(std::pair<rstarnode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>(newNode, hv)); // 
		    
		    //  Do not delete this node.
		    newNode = NULL;	    
		    
		    //  If the second part of the distribution is too large,
		    //  copy to 'toSort' and repeat.
		    if (!(exitLoop & 2)) {
			
			//  Initialize temporary vector.
			std::vector<rectangle<coord_t, bid_t> >* tempVector = new std::vector<rectangle<coord_t, bid_t> >;
			
		    //  Copy to the temporary and the other vector.
			for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
			    toSort[toDelete]->push_back((*(seeds.first))[counter]);
			    tempVector->push_back((*(seeds.first))[counter]);
			}
			
			//  Delete the old vector and make the temporary vector
			//  active.
			delete toSort[1-toDelete];
			toSort[1-toDelete] = tempVector;		
		    }
		}
		
		//  Check if the second part of the distribution creates a new node.
		if (exitLoop & 2) {
		    
		    //  Create node for the second part of the distribution.
		    newNode = tree_->read_node(NEXT_FREE_BLOCK);
		    newBB = (*(seeds.first))[firstGroupNumber];
		    
		    //  Copy entries to the new node.
		    for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
			newNode->add_child((*(seeds.first))[counter]);
			newBB.extend((*(seeds.first))[counter]);
		    }
		    
		    //  Translate the rectangle by the given offset and
		    //  compute the midpoint in scaled integer coordinates.
		    TPIE_OS_LONGLONG x = factor_ * 
			(TPIE_OS_LONGLONG)((newBB.get_left() + newBB.get_right()) / 2.0 - xOffset_);
		    TPIE_OS_LONGLONG y = factor_ *
			(TPIE_OS_LONGLONG)((newBB.get_lower() + newBB.get_upper()) / 2.0 - yOffset_);
		    
		    //  Compute and set the Hilbert value.
		    TPIE_OS_LONGLONG hv = compute_hilbert_value(x, y, size_);
		    
		    //  Cache the new node.
		    cachedNodes_.push(std::pair<rstarnode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>(newNode, hv));

		    //  Do not delete this node.
		    newNode = NULL;	    
		    
		    //  If the first part of the distribution is to large,
		    //  copy to 'toSort' and repeat.
		    if (!(exitLoop & 1)) {
			
			//  Initialize temporary vector.
			std::vector<rectangle<coord_t, bid_t> >* tempVector = new std::vector<rectangle<coord_t, bid_t> >;
			
			//  Copy to the temporary and the other vector.
			for(counter = 0; counter < firstGroupNumber; ++counter) {
			    toSort[toDelete]->push_back((*(seeds.first))[counter]);
			    tempVector->push_back((*(seeds.first))[counter]);
			}
			
			//  Delete the old vector and make the temporary vector
			//  active.
			delete toSort[1-toDelete];
			toSort[1-toDelete] = tempVector;		
		    }
		}
		
		if (exitLoop == 3) {
		    
		    delete toSort[1-toDelete];
		    
		    if (backup != NULL) {
			toSort[0] = backup;
			toSort[1] = new std::vector<rectangle<coord_t, bid_t> >;
			
			for(vi = backup->begin(); vi != backup->end(); ++vi) {
			    toSort[1]->push_back(*vi);
			}
		    }
		}
	    } while ((exitLoop != 3) || (backup != NULL));
	}
	
	template<class coord_t, class BTECOLL>
	err bulkloader<coord_t, BTECOLL>::create_hilbert_rtree(rstartree<coord_t, BTECOLL>** tree) {
	    
	    err result = NO_ERROR;
	    	    
	    //  Create a new tree object.
	    tree_ = new rstartree<coord_t, BTECOLL>(get_input_stream() + ".hrtree", fanout_);
	    	    
	    if (tree_->read_tree_information()) {
		
		//  Copy the pointer to the tree.
		*tree = tree_;
		tree_ = NULL;
		
		return NO_ERROR;
	    }
	    
	    
	    stream<rectangle<coord_t, bid_t> > amis(get_input_stream());
	    stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >* boxUnsorted = new stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >("unsorted.boxes");
	    stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >* boxSorted = new stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >("sorted.boxes");
	    
	    amis.persist(PERSIST_PERSISTENT);
	    boxUnsorted->persist(PERSIST_PERSISTENT);
	    boxSorted->persist(PERSIST_PERSISTENT);
	    
	    //  Find the minimum bounding box of all rectangles in the stream.
	    xOffset_ = (coord_t) 0;
	    yOffset_ = (coord_t) 0;
	    
	    coord_t xMax = (coord_t) 0;
	    coord_t yMax = (coord_t) 0;
	    
	    rectangle<coord_t, bid_t>* mbr = NULL;
	    
	    get_minimum_bounding_rectangle(get_input_stream(), &mbr);
	    
	    if (mbr == NULL) {
		scan_computeBoundingBox<coord_t, bid_t> scb(&mbr);
		
		//  Scan data to compute minimum bounding box.
		scan(&amis, &scb, boxUnsorted);
	    }

	    xOffset_ = mbr->get_left();
	    xMax     = mbr->get_right();
	    yOffset_ = mbr->get_lower();
	    yMax     = mbr->get_upper();
	    
	    delete mbr;    
	    
#if (COORD_T==INT)
	    coord_t scaleFactor = 1;
#else
	    coord_t scaleFactor = 1000000.0;
#endif
	    
	    coord_t maxExtent = std::max(xMax-xOffset_, yMax-yOffset_) * scaleFactor;
	    size_ = (TPIE_OS_LONGLONG)(pow((float)2, (int)((log((float)maxExtent)/log((float)2)) + 1.0)));
	    scan_scale_and_compute_hilbert_value<coord_t> ssb(xOffset_, yOffset_, scaleFactor, size_);
	    
	    //  Scan data to scale the midpoint of each MBR such that it fits
	    //  in the grid. The Hilbert value of each scaled midpoint is stored 
	    //  with the MBRs.
	    scan(&amis, &ssb, boxUnsorted);
	    
	    //  Sort MBRs according to their Hilbert values.
	    sort(boxUnsorted, boxSorted);
	    
	    boxUnsorted->persist(PERSIST_DELETE);
	    boxSorted->persist(PERSIST_DELETE);
	    
	    delete boxUnsorted;
	    boxUnsorted = NULL;
	    
	    TPIE_OS_OFFSET               streamLength = 0;
	    TPIE_OS_OFFSET               streamCounter = 0;
	    TPIE_OS_SIZE_T               level = 0;
	    children_count_t             childCounter = 0;
	    TPIE_OS_OFFSET               nodesCreated = 0;
	    std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>* currentObject = NULL;
	    rectangle<coord_t, bid_t>    bb;
	    rstarnode<coord_t, BTECOLL>* currentNode = tree_->read_node(NEXT_FREE_BLOCK);

	    stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >* currentLevel_ = boxSorted;
	    stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >* nextLevel_;
	    
	    children_count_t minimumPacking = (fanout_ * 3) / 4;
	    double increaseRatio = 1.20;
	    
	    //  Bottom-up construction of the Hilbert R-Tree
	    //  While there is more than one node on the current level...
	    do {
		
		nodesCreated     = 1;
		streamLength     = currentLevel_->stream_len();
		
		if (streamLength == 0) {
		    result = END_OF_STREAM;
		    break; // Exit loop.
		}
		
		//  Start at the beginning of the current level.
		currentLevel_->seek(0);
		
		//  Create a repository for the next level.
		nextLevel_   = new stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >;
		childCounter = 0;
		
		//  Scan the current level and group up to 'fanout_' items 
		//  into one new node.
		for (streamCounter = 0; streamCounter < streamLength; ++streamCounter) {
		    currentLevel_->read_item(&currentObject);
		    
		    if ((currentNode->is_full()) ||
			((currentNode->children() > minimumPacking) &&
			 ((coord_t)bb.extended_area((*currentObject).first) / (coord_t)bb.area() > increaseRatio)))
		    {

			//  If the current node is full, label it with the
			//  correct flag (according to the level and
			//  compute the ID of the parent node.
			if (level == 0) {
			    currentNode->set_flag(RNodeTypeLeaf);
			}
			else {
			    currentNode->set_flag(RNodeTypeInternal);
			    currentNode->update_children_parent();
			}
			currentNode->set_parent(currentNode->bid());
			
			//  Update the bounding box of the current node.
			bb.set_id(currentNode->bid());
			currentNode->set_covering_rectangle(bb);
			
			//  Write the bounding box to the next level's stream.
			nextLevel_->write_item(std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>(bb,0));
			
			//  Save the node in the tree.
			delete currentNode;
			
			//  Create the next node.
			currentNode = tree_->read_node(NEXT_FREE_BLOCK);
			++nodesCreated;
			childCounter = 0;
		    } 
		    
		    //  Add the current object to the current node.
		    
		    currentNode->add_child((*currentObject).first);
		    
		    if (!childCounter) {
			bb = (*currentObject).first;
		    }
		    else {
			bb.extend((*currentObject).first);
		    }
		    
		    ++childCounter;
		}

		if (nodesCreated > 1) {
		    //  Compute the flag and the parent ID of the current node.
		    if (level == 0) {
			currentNode->set_flag(RNodeTypeLeaf);
		    }
		    else {
			currentNode->set_flag(RNodeTypeInternal);
			currentNode->update_children_parent();
		    }
		    currentNode->set_parent(currentNode->bid());
		    
		    //  Update the bounding box of the current node.
		    bb.set_id(currentNode->bid());
		    currentNode->set_covering_rectangle(bb);
		    
		    //  Write the bounding box to the next level's stream
		    nextLevel_->write_item(std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>(bb,0));
		    
		    //  Save the node in the tree.
		    delete currentNode;
		    
		    //  Advance one level.
		    ++level;
		    delete currentLevel_;
		    currentLevel_ = nextLevel_;
		    
		    //  Create the next node.
		    currentNode = tree_->read_node(NEXT_FREE_BLOCK);
		    childCounter = 0;
		}
		else {
		    //  Update the root of the tree (i.e. set the correct flag and
		    //  let the parent ID be the ID of the node itself).
		    bb.set_id(currentNode->bid());
		    currentNode->set_parent(currentNode->bid());
		    currentNode->set_flag(RNodeTypeRoot);
		    currentNode->set_covering_rectangle(bb);
		    
		    nextLevel_->persist(PERSIST_DELETE);
		    delete nextLevel_;
		}
	    } while (nodesCreated > 1);
	    
	    //  Delete the stream for the current level.
	    currentLevel_->persist(PERSIST_DELETE);
	    delete currentLevel_;
	    
	    //  Set the tree's root ID, height, and number of objects.
	    tree_->set_tree_information(currentNode->bid(), level, amis.stream_len());
	    
	    delete currentNode;
	    
	    if (statistics_) {
		
		show_stats();
		std::cout << std::endl;
	    }
	    
	    //  Save tree info.
	    tree_->write_tree_information();
	    
	    //  Copy the pointer to the tree.
	    *tree = tree_;
	    tree_ = NULL;
	    
	    return result;
	}
	
	template<class coord_t, class BTECOLL>
	err bulkloader<coord_t, BTECOLL>::create_rstartree(rstartree<coord_t, BTECOLL>** tree) {
	    
	    err result = NO_ERROR;
	    	    
	    //  Create a new tree object.
	    tree_ = new rstartree<coord_t, BTECOLL>(get_input_stream() + ".rstree", fanout_);	
	    
	    if (tree_->read_tree_information()) {
		
		//  Copy the pointer to the tree.
		*tree = tree_;
		tree_ = NULL;
		
		return NO_ERROR;
	    }
	    
	    
	    stream<rectangle<coord_t, bid_t> > amis(get_input_stream());
	    stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >* boxUnsorted = new stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >("unsorted.boxes");
	    stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >* boxSorted = new stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >("sorted.boxes");
	    
	    amis.persist(PERSIST_PERSISTENT);
	    boxUnsorted->persist(PERSIST_PERSISTENT);
	    boxSorted->persist(PERSIST_PERSISTENT);
	    
	    //  Find the minimum bounding box of all rectangles in the stream.
	    xOffset_ = (coord_t) 0;
	    yOffset_ = (coord_t) 0;
	    
	    coord_t xMax = (coord_t) 0;
	    coord_t yMax = (coord_t) 0;
	    
	    rectangle<coord_t, bid_t>* mbr = NULL;
	    
	    get_minimum_bounding_rectangle(get_input_stream(), &mbr);
	    
	    if (mbr == NULL) {
		scan_computeBoundingBox<coord_t, bid_t> scb(&mbr);
		
		//  Scan data to compute minimum bounding box.
		scan(&amis, &scb, boxUnsorted);
	    }

	    xOffset_ = mbr->get_left();
	    xMax     = mbr->get_right();
	    yOffset_ = mbr->get_lower();
	    yMax     = mbr->get_upper();
	    
	    delete mbr;
	    
#if (COORD_T==INT)
	    coord_t scaleFactor = 1;
#else
	    coord_t scaleFactor = 1000000;
#endif

	    coord_t maxExtent = std::max(xMax-xOffset_, yMax-yOffset_) * scaleFactor;
	    size_ = (TPIE_OS_LONGLONG)(pow((float)2, (int)((log((float)maxExtent)/log((float)2)) + 1.0)));
	    
	    scan_scale_and_compute_hilbert_value<coord_t> ssb(xOffset_, yOffset_, scaleFactor, size_);
	    
	    //  Scan data to scale the midpoint of each MBR such that it fits
	    //  in the grid. The Hilbert value of each scaled midpoint is stored 
	    //  with the MBRs.
	    scan(&amis, &ssb, boxUnsorted);
	    
	    //  Sort MBRs according to their Hilbert values.
	    sort(boxUnsorted, boxSorted);
	    
	    boxUnsorted->persist(PERSIST_DELETE);
	    boxSorted->persist(PERSIST_DELETE);
	    
	    delete boxUnsorted;
	    boxUnsorted = NULL;
	    
	    TPIE_OS_OFFSET               streamLength = 0;
	    TPIE_OS_OFFSET               streamCounter = 0;
	    TPIE_OS_SIZE_T               level = 0;
	    unsigned short               counter = 0;
	    children_count_t             childCounter = 0;
	    TPIE_OS_OFFSET               nodesCreated = 0;
	    std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>* currentObject = NULL;
	    rectangle<coord_t, bid_t>    bb;
	    rstarnode<coord_t, BTECOLL>* currentNode = tree_->read_node(NEXT_FREE_BLOCK);
	    
	    stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >* currentLevel_ = boxSorted;
	    stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >* nextLevel_;
	    
	    children_count_t minimumPacking = (fanout_ * 3) / 4;
	    double increaseRatio = 1.20;
	    
	    //  Bottom-up construction of the (Hilbert-)R*-Tree
	    //  While there is more than one node on the current level...
	    do {
		nodesCreated     = 1;
		streamLength     = currentLevel_->stream_len();
		
		if (streamLength == 0) {
		    result = END_OF_STREAM;
		    break; // Exit loop.
		} 
		
		//  Start at the beginning of the current level.
		currentLevel_->seek(0);
		
		//  Create a repository for the next level.
		nextLevel_   = new stream<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG> >;
		childCounter = 0;
		
		//  Scan the current level and group up to 'fanout_' items 
		//  into one new node.
		for (streamCounter = 0; streamCounter < streamLength; ++streamCounter) {
		    currentLevel_->read_item(&currentObject);
		    
		    if ((currentNode->is_full()) ||
			((currentNode->children() > minimumPacking) &&
			 ((coord_t)bb.extended_area((*currentObject).first) / (coord_t)bb.area() > increaseRatio)))
		    {
			
			//  Add current node to the cache and repack nodes cached
			//  there. The object *currentNode will be deleted there.
			repack_cached_nodes(&currentNode);
			
			while (cachedNodes_.size() > 2) {
			    
			    rstarnode<coord_t, BTECOLL>* tempNode = cachedNodes_.top().first;
			    cachedNodes_.pop();
			    
			    bb = tempNode->get_child(0);
			    
			    for(counter = 0; counter < tempNode->children(); ++counter) {
				bb.extend(tempNode->get_child(counter));
			    }
			    
			    
			    //  If the current node is full, label it with the
			    //  correct flag (according to the level and
			    //  compute the ID of the parent node.
			    if (level == 0) {
				tempNode->set_flag(RNodeTypeLeaf);
			    }
			    else {
				tempNode->set_flag(RNodeTypeInternal);
			    }
			    
			    tempNode->update_children_parent();
			    
			    //  Update the bounding box of the current node.
			    bb.set_id(tempNode->bid());
			    tempNode->set_covering_rectangle(bb);
			    
			    //  Write the bounding box to the next level's stream.
			    nextLevel_->write_item(std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>(bb,0));
			    
			    //  Save the node in the tree.
			    tempNode->persist(PERSIST_PERSISTENT);
			    delete tempNode;
			}
			
			//  Create the next node.
			currentNode = tree_->read_node(NEXT_FREE_BLOCK);
			++nodesCreated;
			childCounter = 0;
		    } 
		    
		    //  Add the current object to the current node.
		    
		    currentNode->add_child((*currentObject).first);
		    
		    if (!childCounter) {
			bb = (*currentObject).first;
		    }
		    else {
			bb.extend((*currentObject).first);
		    }
		    
		    ++childCounter;
		}
		
		if (nodesCreated > 1) {
		    
		    //  Add current node to the cache and repack nodes cached
		    //  there. The object *currentNode will be deleted there.
		    repack_cached_nodes(&currentNode);
		    
		    while (!cachedNodes_.empty()) {
			
			rstarnode<coord_t, BTECOLL>* tempNode = cachedNodes_.top().first;
			cachedNodes_.pop();
			
			bb = tempNode->get_child(0);
			
			for(counter = 0; counter < tempNode->children(); ++counter) {
			    bb.extend(tempNode->get_child(counter));
			}
			
			//  If the current node is full, label it with the
			//  correct flag (according to the level and
			//  compute the ID of the parent node.
			if (level == 0) {
			    tempNode->set_flag(RNodeTypeLeaf);
			}
			else {
			tempNode->set_flag(RNodeTypeInternal);
			}
			
			tempNode->update_children_parent();
			
			//  Update the bounding box of the current node.
			bb.set_id(tempNode->bid());
			tempNode->set_covering_rectangle(bb);
			
			//  Write the bounding box to the next level's stream.
			nextLevel_->write_item(std::pair<rectangle<coord_t, bid_t>, TPIE_OS_LONGLONG>(bb,0));
			
			//  Save the node in the tree.
			tempNode->persist(PERSIST_PERSISTENT);
			delete tempNode;
		    }
		    
		    //  Delete the current level.
		    currentLevel_->persist(PERSIST_DELETE);
		    delete currentLevel_;
		    
		    //  Advance one level.
		    currentLevel_ = nextLevel_;
		    ++level;
		    
		    //  Create the next node.
		    currentNode = tree_->read_node(NEXT_FREE_BLOCK);
		    childCounter = 0;
		}
		else {
		    
		    //  Update the root of the tree (i.e. set the correct flag and
		    //  let the parent ID be the ID of the node itself).
		    
		    //  Make the children aware of their parent.
		    currentNode->update_children_parent();
		    
		    //  Make the actual node the root.
		    bb.set_id(currentNode->bid());
		    currentNode->set_parent(currentNode->bid());
		    currentNode->set_flag(RNodeTypeRoot);
		    currentNode->set_covering_rectangle(bb);
		    
		    //  Delete the stream for the next level.
		    nextLevel_->persist(PERSIST_DELETE);
		    delete nextLevel_;
		}
		
		assert(cachedNodes_.empty());
		
	    } while (nodesCreated > 1);
	    
	    
	    //  Delete the stream for the current level.
	    currentLevel_->persist(PERSIST_DELETE);
	    delete currentLevel_;
	    
	    //  Set the tree's root ID, height, and number of objects.
	    tree_->set_tree_information(currentNode->bid(), level, amis.stream_len());
	    
	    delete currentNode;
	    
	    if (statistics_) {
		
		show_stats();
		std::cout << std::endl;
	    }
	    
	    //  Save tree info.
	    tree_->write_tree_information();
	    
	    //  Copy the pointer to the tree.
	    *tree = tree_;
	    tree_ = NULL;
	    
	    return result;
	}

    }  //  ami namespace

}  //  tpie namespace


#endif

//
//  References:
//
//  @inproceedings{BKSS90
//  , author = 	     "Norbert Beckmann and Hans-Peter Kriegel and Ralf
//                    Schneider and Bernhard Seeger" 
//  , title = 	     "The {R$^\ast$}-tree: {A}n Efficient and Robust
//                    Access Method for Points and Rectangles"
//  , pages = 	     "322--331"
//  , booktitle =    "Proceedings of the 1990 {ACM} {SIGMOD} International
//                    Conference on Management of Data"
//  , year = 	     1990
//  , editor = 	     "Hector Garcia-Molina and H. V. Jagadish"
//  , volume = 	     "19.2"
//  , series = 	     "{SIGMOD} Record"
//  , month = 	     "June"
//  , publisher =    "{ACM} Press"
//  }
//
//  @inproceedings{DKL+94
//  , author = 	 "David J. DeWitt and Navin Kabra and Jun Luo and 
//                 Jignesh M. Patel and Jie-Bing Yu"
//  , title = 	 "Client-Server {P}aradise"
//  , pages = 	 "558--569"
//  , booktitle = "Proceedings of the 20th International
//                 Conference on Very Large Data Bases (VLDB'94)"
//  , year =       1994
//  , editor =    "Jorge B. Bocca and Matthias Jarke and Carlo Zaniolo" 
//  , publisher = "Morgan Kaufmann"
//  }
