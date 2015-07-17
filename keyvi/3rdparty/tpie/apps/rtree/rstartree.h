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
//  Description:     declarations for class RStarTree
//  Created:         05.11.1998
//  Author:          Jan Vahrenhold
//  mail:            jan.vahrenhold@math.uni-muenster.de
//  $Id: rstartree.h,v 1.3 2004-08-12 12:37:24 jan Exp $
//  Copyright (C) 1997-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

//  Prevent multiple #includes.
#ifndef _TPIE_AMI_RSTARTREE_H
#define _TPIE_AMI_RSTARTREE_H

#include "app_config.h"
#include <tpie/portability.h>

//  Include <iostream> for output operator.
#include <iostream>

//  Include <algorithm> for STL algorithms (min/max/sorting).
#include <algorithm>

//  Include STL templates pair, list, and vector.
#include <utility>
#include <list>
#include <vector>

//  Include TPIE AMI declarations and template stack.
#include <tpie/stream.h>
#include <tpie/coll.h>
#include <tpie/block.h>
#include <tpie/stack.h>

//  Include class rectangle.
#include "rectangle.h"
#include "rectangle_comparators.h"

//  Include class RStarNode
#include "rstarnode.h"

namespace tpie {

    namespace ami {

//  Define a meaningfull symbolic constant used for indicating
//  that the block to be read is a new one.
	const bid_t NEXT_FREE_BLOCK = 0;
	
////////////////////////////////////////////////////////////////////////////////
/// An RStarTree<coord_t,BTECOLL> realizes an R*-tree for two-dimensional
/// data rectangles with coordinates of type coord_t. Data is stored in a
/// BTE block collection. The implementation follows the original description
/// of Beckmann et al. [BKSS90]. \par
///  [BKSS90] Norbert Beckmann, Hans-Peter Kriegel, Ralf Schneider, Bernhard Seeger:
///  "The R*-tree: An Efficient and Robust Access Method for Points and Rectangles",
///  In: Hector Garcia-Molina and H. V. Jagadish (editors): Proceedings of the 1990 
///  ACM SIGMOD International Conference on Management of Data, SIGMOD Record 19.2 
/// (1990): 322-331.
////////////////////////////////////////////////////////////////////////////////
	template<class coord_t, class BTECOLL = bte::COLLECTION>
	class rstartree {

	public:

	    ////////////////////////////////////////////////////////////////////////////
	    /// The R*-tree is defined by its fan-out, and a name for the file is
	    /// shall reside in. If the storage area does not have an associated
	    /// metadata file or does not contain an R*-tree, a new (persistent) file 
	    /// will be created.
	    /// \param[in] name name of the storage area
	    /// \param[in] fanout maximum fanout
	    ////////////////////////////////////////////////////////////////////////////
	    rstartree(
		const std::string& name,
		children_count_t   fanout);

	    ////////////////////////////////////////////////////////////////////////////
	    /// The R*-tree object is deleted, the tree metadata is written to
	    /// disk, and the storage area is deleted as well (if the storage area
	    /// is not persistent, it will be gone...).
	    ////////////////////////////////////////////////////////////////////////////
	    ~rstartree();
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// Two R*-trees are identical iff they use the same storage area.
	    ////////////////////////////////////////////////////////////////////////////
	    bool operator==(const rstartree<coord_t, BTECOLL>& other);

	    ////////////////////////////////////////////////////////////////////////////
	    /// Two R*-trees are identical iff they use the same storage area.
	    ////////////////////////////////////////////////////////////////////////////
	    bool operator!=(const rstartree<coord_t, BTECOLL>& other);
 
	    ////////////////////////////////////////////////////////////////////////////
	    /// Return the block size of the underlying BTE collection.
	    ////////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T block_size() const;

	    ////////////////////////////////////////////////////////////////////////////
	    /// Return the name of the underlying BTE collection.
	    ////////////////////////////////////////////////////////////////////////////
	    const std::string& name() const;

	    ////////////////////////////////////////////////////////////////////////////
	    /// Return the fanout of the R*-tree.
	    ////////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T fanout() const;

	    ////////////////////////////////////////////////////////////////////////////
	    /// Return the height of the R*-tree (starting at zero).
	    ////////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T tree_height() const;

	    ////////////////////////////////////////////////////////////////////////////
	    /// Return the number of data objects stored in the R*-tree.
	    ////////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET total_objects() const;

	    ////////////////////////////////////////////////////////////////////////////
	    /// Return the ID of the block that stores the root of the tree.
	    ////////////////////////////////////////////////////////////////////////////
	    bid_t root_position() const;

	    ////////////////////////////////////////////////////////////////////////////
	    /// Loads the given block and create a new node object from the block's 
	    /// contents. Note that it is the responsibility of the programmer to 
	    /// delete this object.
	    /// There is NO corresponding 'writeNode' method, as writing a node
	    /// is be done automatically upon calling the node's destructor.
	    /// \param[in] position indicates the ID of the block to be read
	    ////////////////////////////////////////////////////////////////////////////
	    rstarnode<coord_t, BTECOLL>* read_node(bid_t position);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method provides the standard way to insert an object into the
	    /// tree [BKSS90].
	    /// \param[in] r bounding rectangle plus ID of the object to be inserted
	    ////////////////////////////////////////////////////////////////////////////
	    err insert(const rectangle<coord_t, bid_t>& r);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method provides the standard way to delete an object from the
	    /// tree (as an exact match delete) [BKSS90].
	    /// \param[in] r bounding rectangle plus ID of the object to be deleted
	    ////////////////////////////////////////////////////////////////////////////
	    err remove(const rectangle<coord_t, bid_t>& r);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method realizes the "single shot" query algorithm for R-trees.
	    /// Given a query rectangle "r" we look for all subtrees whose bounding
	    /// rectangle overlaps the query rectangle and proceed recursively
	    /// for each such subtree. If the current node is a leaf, all overlapping
	    /// rectangles are written to the stream "matches", otherwise all
	    /// overlapping bounding rectangles of subtrees are pushed on
	    /// the stack "candidates". If the "bruteForce" flag is set, the complete
	    /// tree is traversed.
	    /// \param[in] r rectangle representing the query range
	    /// \param[in] matches stream to store all results
	    /// \param[in] candidates (external) stack to store intermediate data
	    /// \param[in] bruteForce flag indicating whether or not to traverse
	    /// the whole tree.
	    ////////////////////////////////////////////////////////////////////////////
	    err query(
		const rectangle<coord_t, bid_t>&    r, 
		stream<rectangle<coord_t, bid_t> >* matches,
		ami::stack<bid_t>*                  candidates,
		bool                                bruteForce = false);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method realized a depth-first search looking for a node
	    /// with a given ID and prints all its contents.
	    ////////////////////////////////////////////////////////////////////////////
	    err find_node(
		bid_t             nodeID,  
		ami::stack<bid_t>* candidates);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method traverses the whole tree and checks for each node
	    /// whether the bounding box stored with a subtree is the same as the
	    /// exact bounding box obtained when computing it "manually".\par
	    /// This method is for debugging purposes only.
	    ////////////////////////////////////////////////////////////////////////////
	    void check_tree();
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method shows several statistical information about the
	    /// including the BTE statistics (if available).
	    ////////////////////////////////////////////////////////////////////////////
	    void show_stats();
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// After bulk loading by bottom-up construction, this method needs
	    /// to be called to set the information about the root's block ID,
	    /// the height of the tree, and the number of objects. There is
	    /// no validation of these values, however, so be sure you know
	    /// what you are doing.
	    ////////////////////////////////////////////////////////////////////////////
	    void set_tree_information(
		bid_t          root, 
		TPIE_OS_SIZE_T height,
		TPIE_OS_OFFSET objects);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// Write the tree's metadata (height, fanout, number of objects, and block
	    /// ID of the root) so that the tree can be reused. The name of the metadata 
	    /// file is the tree's name suffixed by ".info". 
	    ////////////////////////////////////////////////////////////////////////////
	    void write_tree_information();

	    ////////////////////////////////////////////////////////////////////////////
	    /// Read the tree's metadata from a file whose name is the tree's name
	    /// suffixed by ".info" and initialize the height, fanout, number of
	    /// objects, and root position. There is no validation of these values, 
	    /// however, so be sure you know what you are doing.
	    ////////////////////////////////////////////////////////////////////////////
	    bool read_tree_information();
	    
	protected:

	    ////////////////////////////////////////////////////////////////////////////
            /// Pointer to the storage area
	    ////////////////////////////////////////////////////////////////////////////
	    collection_single<BTECOLL>* storage_area_;      

	    ////////////////////////////////////////////////////////////////////////////
            /// Maximum fanout for a node.
	    ////////////////////////////////////////////////////////////////////////////
	    children_count_t            fanout_;          

	    ////////////////////////////////////////////////////////////////////////////
            /// Minimum fanout for a node.
	    ////////////////////////////////////////////////////////////////////////////
	    children_count_t            minFanOut_;       

	    ////////////////////////////////////////////////////////////////////////////
            /// Block size of the underlying storage area in bytes.
	    ////////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T              block_size_;        

	    ////////////////////////////////////////////////////////////////////////////
            /// Height of the tree.
	    ////////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T              tree_height_;       

	    ////////////////////////////////////////////////////////////////////////////
            /// ID of the block storing the root.
	    ////////////////////////////////////////////////////////////////////////////
	    bid_t                       root_position_;     

	    ////////////////////////////////////////////////////////////////////////////
            /// Number of objects in the tree.
	    ////////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET              total_objects_;     

	    ////////////////////////////////////////////////////////////////////////////
            /// Name of the storage area's file.
	    ////////////////////////////////////////////////////////////////////////////
	    std::string                  name_;

	    ////////////////////////////////////////////////////////////////////////////
	    /// This method splits a given node into two new nodes.
	    /// If necessary, a new root is created.
	    /// The first node returned has the same block ID as the original node.
	    /// \param[in] toSplit the node to be split.
	    ////////////////////////////////////////////////////////////////////////////
	    std::pair<rstarnode<coord_t, BTECOLL>*, 
		 rstarnode<coord_t, BTECOLL>*> split_node(rstarnode<coord_t, BTECOLL>* toSplit);

	    ////////////////////////////////////////////////////////////////////////////
	    /// This method inserts a given rectangle using the default routing
	    /// algorithm making sure that the level of the node where the rectangle 
	    /// is placed is the given level. N.B. Levels are counted bottom-up with
	    /// leaves being on level 0.
	    /// \param[in] r bounding rectangle to be inserted
	    /// \param[in] level level on which to insert the rectangle 
	    ////////////////////////////////////////////////////////////////////////////
	    err insert_on_level(
		const rectangle<coord_t, bid_t>& r, 
		TPIE_OS_SIZE_T                   level);

	    ////////////////////////////////////////////////////////////////////////////
	    /// This method selects a node on a given level suitable to insert
	    /// the given rectangle into. See "Algorithm ChooseSubtree" [BKSS90].
	    /// N.B. Levels are counted bottom-up with leaves being on level 0.
	    /// \param[in] r bounding rectangle to be inserted
	    /// \param[in] level level on which to insert the rectangle 
	    ////////////////////////////////////////////////////////////////////////////
	    rstarnode<coord_t, BTECOLL>* choose_node_on_level(
		const rectangle<coord_t, bid_t>& r, 
		TPIE_OS_SIZE_T                   level);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method selects a leaf node suitable to insert
	    /// the given rectangle into. This corrensponds to calling
	    /// \ref choose_node_on_level with arguments r and 0.
	    /// \param[in] r bounding rectangle to be inserted
	    ////////////////////////////////////////////////////////////////////////////
	    rstarnode<coord_t, BTECOLL>* choose_leaf(const rectangle<coord_t, bid_t>& r);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method handles underflow treatment after deletions.
	    /// See "Algorithm CondenseTree" [Gutt84].
	    /// \param[in] n node at which to start the condense process
	    /// \param[in] level level on which the node n resides
	    ////////////////////////////////////////////////////////////////////////////
	    err condense_tree(rstarnode<coord_t, BTECOLL>* n, 
			      TPIE_OS_SIZE_T               level);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// Given a (pointer to a) node, its bounding rectangle, this method
	    /// reinserts the node on the given level making sure that any
	    /// overflow is treated properly.
	    /// \param[in] n node to be reinserted
	    /// \param[in] r bounding rectangle of the node to be reinserted
	    /// \param[in] level level on which the node n resides
	    ////////////////////////////////////////////////////////////////////////////
	    err reinsert(
		rstarnode<coord_t, BTECOLL>*     n, 
		const rectangle<coord_t, bid_t>& r, 
		TPIE_OS_SIZE_T                   level);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// This method realized a depth-first search looking for a leaf
	    /// containing a given object and returns this leaf's ID (or 0 if
	    /// the search was unsuccessful. Note that this algorithm keeps track 
	    /// of all node to be visited in internal memory.
	    /// \param[in] r bounding rectangle plus ID of the object
	    ////////////////////////////////////////////////////////////////////////////
	    bid_t find_leaf(const rectangle<coord_t, bid_t>& r);
	    
	    ////////////////////////////////////////////////////////////////////////////
	    /// Install an old node 'node1' and a new node 'node2' to 'node1's 
	    /// parent node. If necessary, split the parent node and propagate the
	    /// split upwards. The ID of the child to be replaced in the
	    /// parent node has to be passed to this method.
	    /// \param[in] node1 node to replace an already present node
	    /// \param[in] node2 node to be installed as node1's sibling
	    /// \param[in] childToBeReplaced ID of the node to be replaced.
	    /// \param[in] level level on which the node node1 resides
	    ////////////////////////////////////////////////////////////////////////////
	    void adjust_tree_on_level(
		rstarnode<coord_t, BTECOLL>*  node1, 
		rstarnode<coord_t, BTECOLL>*  node2,
		bid_t                         childToBeReplaced,
		TPIE_OS_SIZE_T                level);
	    
	private:
	    ////////////////////////////////////////////////////////////////////////////
	    /// No default constructor is provided for R*-trees.
	    ////////////////////////////////////////////////////////////////////////////
	    rstartree();

	    ////////////////////////////////////////////////////////////////////////////
	    /// Neither a copy constructor nor an assignment operator are
	    /// provided for R*-trees.
	    ////////////////////////////////////////////////////////////////////////////
	    rstartree(const rstartree<coord_t, BTECOLL>& other);

	    ////////////////////////////////////////////////////////////////////////////
	    /// Neither a copy constructor nor an assignment operator are
	    /// provided for R*-trees.
	    ////////////////////////////////////////////////////////////////////////////
	    rstartree<coord_t, BTECOLL>& operator=(const rstartree<coord_t, BTECOLL>& other);

	    std::list<std::pair<rectangle<coord_t, bid_t>, TPIE_OS_SIZE_T> > reinsertObjects_; 
	    std::vector<bool>           overflowOnLevel_;

	    //  This method is called upon returning from a deletion
	    //  if there are objects to be reinserted.
	    err handle_reinsertions();
	    
	};
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	TPIE_OS_SIZE_T rstartree<coord_t, BTECOLL>::block_size() const {
	    return block_size_;
	}
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	TPIE_OS_SIZE_T rstartree<coord_t, BTECOLL>::tree_height() const {
	    return tree_height_;
	}
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	TPIE_OS_OFFSET rstartree<coord_t, BTECOLL>::total_objects() const {
	    return total_objects_;
	}
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	bid_t rstartree<coord_t, BTECOLL>::root_position() const {
	    return root_position_;
	}
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	TPIE_OS_SIZE_T rstartree<coord_t, BTECOLL>::fanout() const {
	    return fanout_;
	}
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	const std::string& rstartree<coord_t, BTECOLL>::name() const {
	    return name_;
	}
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	rstarnode<coord_t, BTECOLL>* rstartree<coord_t, BTECOLL>::read_node(bid_t position) {
	    
	    //  Try to fetch the node from the buffer.
	    rstarnode<coord_t, BTECOLL>* node = 
		new rstarnode<coord_t, BTECOLL>(storage_area_,
						this, 
						position, 
						position, 
						fanout_);
	    
	    assert((position==NEXT_FREE_BLOCK) || (node->bid() == position));
	    
	    return node;
	}
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstartree<coord_t, BTECOLL>::set_tree_information(bid_t          root, 
							       TPIE_OS_SIZE_T height, 
							       TPIE_OS_OFFSET objects) {
	    root_position_ = root;
	    tree_height_   = height;
	    total_objects_ = objects;
	}
	
	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstartree<coord_t, BTECOLL>::show_stats() {
	    TPIE_OS_OFFSET nodes = storage_area_->size();
	    std::cout << std::endl;
	    std::cout << "R*-tree statistics for file: " << name() << std::endl;
	    std::cout << "- root position     : " << root_position_ << std::endl;
	    std::cout << "- fan-out           : " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(fanout_) << std::endl;
	    std::cout << "- height            : " << tree_height_ << std::endl;
	    std::cout << "- nodes             : " << nodes << std::endl;
	    std::cout << "- objects           : " << total_objects_ << std::endl;
	    std::cout << "- space utilization : " << (double) (total_objects_ + nodes -1) / (double)(nodes*fanout_) << std::endl;
	    std::cout << "BTE statistics: " << std::endl;
	    std::cout << "- block reads       : " << storage_area_->stats().get(BLOCK_GET) <<std::endl;	    
	    std::cout << "- block writes      : " << storage_area_->stats().get(BLOCK_PUT) <<std::endl;	    
	    std::cout << std::endl;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	rstartree<coord_t, BTECOLL>::rstartree(const std::string& name, 
					       children_count_t   fanout) {

	    name_ = name;

	    storage_area_ = new collection_single<BTECOLL>(name_);
	    storage_area_->persist(PERSIST_PERSISTENT);

	    //  Set all attributes to the initial values.
	    block_size_     = storage_area_->block_size();
	    tree_height_    = 0;
	    total_objects_  = 0;

	    //  Compute the maximal number of children that can be packed
	    //  into a disk block.
	    const TPIE_OS_SIZE_T nodeInfoSize = 
		sizeof(fanout) +
		sizeof(block_size_) +
		sizeof(root_position_);
	    const TPIE_OS_SIZE_T childInfoSize = sizeof(rectangle<coord_t, bid_t>);

	    fanout_   = static_cast<children_count_t>((block_size_ - nodeInfoSize) / childInfoSize);

	    //  If a user-defined branching factor is given, check whether
	    //  this number fits into the block and is not equal to zero.
	    if ((fanout <= fanout_) &&
		(fanout != 0)) {
		fanout_ = fanout;
	    }

	    minFanOut_ = (TPIE_OS_SIZE_T) ((double)fanout_ / MIN_FANOUT_FACTOR);
    
	    overflowOnLevel_.push_back(false);

	    //  Try to read tree information from meta file.
	    read_tree_information();

	    if (!total_objects_) {

		//  Create a "fresh" block collection.
		storage_area_->persist(PERSIST_DELETE);
		delete storage_area_;
		storage_area_  = new collection_single<BTECOLL>(name_);
	
		//  Create an empty root.
		rstarnode<coord_t, BTECOLL>* root = new rstarnode<coord_t, BTECOLL>(
		    storage_area_,
		    this, 
		    NEXT_FREE_BLOCK, NEXT_FREE_BLOCK,
		    fanout_);
		root->set_flag(RNodeTypeLeaf | RNodeTypeRoot );
		root->set_parent(root->bid());
		root_position_ = root->bid();

		//  Write the root to disk.
		delete root;
		root = NULL;
	    }
	    else {
		std::cerr << "Existing block collection.\n";
	    }

	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	rstartree<coord_t, BTECOLL>::~rstartree() {

	    write_tree_information();

	    //  Free the storage area.
	    delete storage_area_;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	bool rstartree<coord_t, BTECOLL>::operator==(const rstartree<coord_t, BTECOLL>& other) {
	    return (storage_area_ == other.storage_area_);
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	bool rstartree<coord_t, BTECOLL>::operator!=(const rstartree<coord_t, BTECOLL>& other) {
	    return !(*this == other);
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	err rstartree<coord_t, BTECOLL>::query(
	    const rectangle<coord_t, bid_t>&    bb, 
	    stream<rectangle<coord_t, bid_t> >* matches, 
	    ami::stack<bid_t>*            candidates, 
	    bool                                bruteForce) {

	    rstarnode<coord_t, BTECOLL>* n = NULL;
	    const bid_t*                 current = NULL;
	    err                          result = NO_ERROR;
	    TPIE_OS_OFFSET               candidatesCounter = 0;
	    TPIE_OS_OFFSET               leafCounter = 0;

	    //  Initialize the process by pushing the root's ID onto the stack.
	    candidates->push(root_position_);
	    ++candidatesCounter;

	    //  Explore the tree by (restricted) depth-first traversal.
	    while (candidatesCounter > 0) {

		//  Pop the topmost node ID from the stack.
		result = candidates->pop(&current);
		--candidatesCounter;
	 
		if (result != NO_ERROR) {
		    break;
		}
	 
		//  Read the current node and find eventually overlapping children.
		n = read_node(*current);    
		n->query(bb, candidates, matches, candidatesCounter, leafCounter, bruteForce);
		delete n;
		n = NULL;
	    }
     
	    if (bruteForce) {
		std::cout << leafCounter << " objects." << std::endl;
	    }

	    return result;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	err rstartree<coord_t, BTECOLL>::find_node(bid_t              nodeID,  
						  ami::stack<bid_t>* candidates) {
	    rstarnode<coord_t, BTECOLL>* n = NULL;
	    const bid_t*                       current = NULL;
	    err                          result = NO_ERROR;
	    TPIE_OS_OFFSET               candidatesCounter = 0;

	    std::cout << "Looking for : " << nodeID << std::endl;

	    //  Initialize the search by pushing the root's ID onto the stack.
	    candidates->push(root_position_);
	    ++candidatesCounter;

	    //  Explore the tree by depth-first traversal.
	    while (candidatesCounter > 0) {

		//  Pop the topmost node ID from the stack.
		result = candidates->pop(&current);
		--candidatesCounter;
	 
		if (result != NO_ERROR) {
		    break;
		}
	 
		//  Read the current node and find the node in question.
		n = read_node(*current);    
		n->find_node(nodeID, candidates, candidatesCounter);
		delete n;
		n = NULL;
	    }

	    return result;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstartree<coord_t, BTECOLL>::check_tree() {

	    std::list<std::pair<bid_t, rectangle<coord_t, bid_t> > > l;
	    rectangle<coord_t, bid_t>                                checkNode;
	    rstarnode<coord_t, BTECOLL>*                             n = NULL;
	    TPIE_OS_OFFSET                                           objectCounter = 0;

	    std::cerr << "Checking tree " << name() << std::endl;

	    //  Initialize the process by pushing the root's ID into the queue.
	    l.push_back(std::pair<bid_t, rectangle<coord_t, bid_t> >(root_position_, checkNode));

	    //  Explore the tree by breadth-first traversal.
	    while (!l.empty()) {

		//  Remove the frontmost element from the queue.
		n         = read_node(l.front().first);
		checkNode = l.front().second;
		l.pop_front();

		//  Check all children of the current node.
		n->check_children(checkNode, l, objectCounter);
		delete n;
		n = NULL;
	    }

	    std::cerr << objectCounter << " objects stored in this tree." << std::endl;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	rstarnode<coord_t, BTECOLL>* rstartree<coord_t, BTECOLL>::choose_leaf(const rectangle<coord_t, bid_t>& r) {
	    return choose_node_on_level(r, 0);
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	rstarnode<coord_t, BTECOLL>* rstartree<coord_t, BTECOLL>::choose_node_on_level(
	    const rectangle<coord_t, bid_t>& r, 
	    TPIE_OS_SIZE_T                   level) {

	    rstarnode<coord_t, BTECOLL>*  N  = read_node(root_position_);
	    bid_t                         ID = 0;
	    TPIE_OS_SIZE_T                lookingAtLevel = tree_height();
    
	    //  Proceed on a root-to-leaf path.
	    while(lookingAtLevel > level) {

		--lookingAtLevel;

		//  Select the child whose bounding rectangle need least
		//  enlargement to include "r".
		//  Adjust the bounding rectangle to include "r".
		ID = N->get_child(N->route(r)).get_id();

		//  Write the node to disk.
		delete N;
		N = NULL;
		//  Load the child node selected above.
		N = read_node(ID);
	    }

	    return N;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	bid_t rstartree<coord_t, BTECOLL>::find_leaf(const rectangle<coord_t, bid_t>& r) {
	    rstarnode<coord_t, BTECOLL>*     n = NULL;
	    bid_t       current = (bid_t) 0;     
	    std::list<bid_t> candidateList;

	    //  Initialize the process by pushing the root's ID onto the stack.
	    candidateList.push_back(root_position_);

	    //  Explore the tree by (restricted) depth-first traversal.
	    while ((candidateList.size() > 0) && (current == 0)) {

		//  Pop the topmost node ID from the stack.
		current = candidateList.front();
		candidateList.pop_front();
	 
		//  Read the current node and find eventually overlapping children.
		n = read_node(current);    
		current = n->find_leaf(r, &candidateList);
		delete n;
		n = NULL;
	    }
     
	    return current;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	err rstartree<coord_t, BTECOLL>::condense_tree(rstarnode<coord_t, BTECOLL>* n, 
						      TPIE_OS_SIZE_T level) {

	    //  The return value is always NO_ERROR.
	    //  You might want to check for I/O errors every time
	    //  an I/O has been performed.
	    err result = NO_ERROR;
    
	    while ((!n->is_root()) && (result == NO_ERROR)) {
		rstarnode<coord_t, BTECOLL>*     parent = read_node(n->get_parent());
		children_count_t sonID = parent->find_child(n->bid());
		children_count_t counter = 0;	

		//  Check whether there is an underflow in node n.
		if (n->children() < minFanOut_) {
	    
		    //  Move all children of node n to the list of objects to
		    //  be reinserted.
		    for (counter = 0; counter < n->children(); ++counter) {
			reinsertObjects_.push_back(std::pair<rectangle<coord_t, bid_t>, TPIE_OS_SIZE_T>(n->get_child(counter), level));
		    }
	    
		    //  Remove n from its parent.
		    parent->remove_child(sonID);
	    
		    //  Delete the (block containing the) old node.
		    n->persist(PERSIST_DELETE);
		    delete n;
		    n = NULL;

		}
		else {
	    
		    //  Compute the exact boundingbox of n.
		    rectangle<coord_t, bid_t> cover = n->get_child(0);
		    for (counter = 1; counter < n->children(); ++counter) {
			cover.extend(n->get_child(counter));
		    }
		    cover.set_id(n->bid());
	    
		    //  Update n's bounding box stored in n's parent.
		    parent->set_child(sonID, cover);

		    //  Write the node to disk.
		    delete n;
		    n = NULL;

		}
	
		n = parent;

		++level;
	    }

	    //  Uncomment the following line, if you don't trust
	    //  the algorithm and want to check the tree every time
	    //  the height decreases.
//    TPIE_OS_SIZE_T th = tree_height();

	    if (result == NO_ERROR) {

		//  Node n is the root of the tree.
		if (!n->is_leaf()) {
		    if (n->children() == 1) {
		
			//  Check whether there is an underflow in node n.
			bid_t newRootPosition = n->get_child(0).get_id();
		
			//  If the only child of the root is a node make it
			//  the new root.
			if (newRootPosition > 0) {
		    
			    root_position_ = newRootPosition;
		    
			    //  Delete the old node.
			    n->persist(PERSIST_DELETE);
			    delete n;
			    n = read_node(root_position_);
			    n->set_flag(n->get_flag() | RNodeTypeRoot);
			    n->set_parent(n->bid());
		    
			    //  Decrease the height of the tree by one.
			    --tree_height_;
			}
			//  Otherwise nothing is to be done.
		    }
		}
	    }

	    //  Write the node to disk.
	    delete n;
	    n = NULL;

	    //  Uncomment the following three lines, if you don't trust
	    //  the algorithm and want to check the tree every time
	    //  the height decreases.
//    if (tree_height() != th) {
//	check_tree();
//    }

	    return result;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	err rstartree<coord_t, BTECOLL>::remove(const rectangle<coord_t, bid_t>& r) {
	    err   result = NO_ERROR;
	    bid_t nodeID = find_leaf(r);

	    if (nodeID > 0) {
		//  If the node contained the objects proceed as
		//  described in Guttman's delete-algorithm.
		rstarnode<coord_t, BTECOLL>* n = read_node(nodeID);

		//  Remove the object to be deleted.
		n->remove_child(r);

		--total_objects_;

		//  Node n will be deleted within this method.
		result = condense_tree(n, 0);  
	
		if (result == NO_ERROR) {

		    //  Reinsert orphaned entries.
		    result = handle_reinsertions();
		}

		if (result != NO_ERROR) {
		    std::cerr << "After deletion of " << r.get_id() << std::endl;
		    std::cerr << "ERROR " << result << " occurred." << std::endl;
		}

	    }
	    else {
		std::cerr << "Object to be deleted (ID=" << r.get_id() << ") not found." << std::endl;
		check_tree();
		result = END_OF_STREAM;
	    }

	    return result;    
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	err rstartree<coord_t, BTECOLL>::handle_reinsertions() {

	    err                       result = NO_ERROR;
	    rectangle<coord_t, bid_t> r;
	    TPIE_OS_SIZE_T            level = 0;

	    while ((result == NO_ERROR) && (!reinsertObjects_.empty())) { 

		r     = (*reinsertObjects_.begin()).first;
		level = (*reinsertObjects_.begin()).second;

		reinsertObjects_.pop_front();

		result = insert_on_level(r, level);
	
	    }

	    return result;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	err rstartree<coord_t, BTECOLL>::insert(const rectangle<coord_t, bid_t>& r) {

	    err result = NO_ERROR;

	    //  Initialize the overflow array
	    TPIE_OS_SIZE_T counter;
	    for (counter = 0; counter < overflowOnLevel_.size(); ++counter) {
		overflowOnLevel_[counter] = false;
	    }

	    ++total_objects_;

	    result = insert_on_level(r, 0);

	    if (result == NO_ERROR) {
		handle_reinsertions();
	    }

	    return result;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	err rstartree<coord_t, BTECOLL>::insert_on_level(const rectangle<coord_t, bid_t>& r,
							 TPIE_OS_SIZE_T level) {

	    //  The return value is always NO_ERROR.
	    //  You might want to check for I/O errors every time
	    //  an I/O has been performed.
	    err    result        = NO_ERROR;

	    rstarnode<coord_t, BTECOLL>* insertionNode = choose_node_on_level(r, level);

	    if (insertionNode->is_full()) {

		//  "If the level is not the root level and this is the first
		//   call of OverflowTreatment in the given level during the
		//   insertion of one data rectangle then invoke reinsert
		//   else invoke split." [BKS93], 327.
	
		if ((overflowOnLevel_[level]) || (insertionNode->is_root())) {

		    insertionNode->add_child(r);
	    
		    //  If we are inserting on other than leaf-level, the 
		    //  (re-)inserted node needs to know its new parent.
		    if (!insertionNode->is_leaf()) {
			rstarnode<coord_t, BTECOLL>* tempNode = read_node(r.get_id());
			tempNode->set_parent(insertionNode->bid());

			//  Write the node to disk.
			delete tempNode;
			tempNode = NULL;
		    }

		    //  If the insertion node is full, split it to obtain two new 
		    //  node. (The special case of splitting the root is handled by 
		    //  the split-method.)

		    std::pair<rstarnode<coord_t, BTECOLL>*, rstarnode<coord_t, BTECOLL>*> nodeTupel = split_node(insertionNode);

		    bid_t childToBeReplaced = insertionNode->bid();

		    //  Delete the node that has been split.
		    insertionNode->persist(PERSIST_DELETE);
		    delete insertionNode;
		    insertionNode = NULL;
	    
		    //  Propagate the split upwards passing the new nodes.
		    if (nodeTupel.first) {
			adjust_tree_on_level(nodeTupel.first, nodeTupel.second, 
					  childToBeReplaced, level+1);
		    }
		}
		else {
	    

		    //  Mark the level.
		    overflowOnLevel_[level] = true;

		    //  Force reinsert.
		    reinsert(insertionNode, r, level);

		    //  Delete the old node (it has been substituted in 'reinsert').
		    insertionNode->persist(PERSIST_DELETE);
		    delete insertionNode;
		    insertionNode = NULL;

		}
	    }
	    else {

		//  If the insertion node still has enough room, insert the
		//  given bounding box...
		insertionNode->add_child(r);

		if (!insertionNode->is_leaf()) {
		    //  Notify the node of its new parent, i.e., "insertionNode".
		    rstarnode<coord_t, BTECOLL>* tempNode = read_node(r.get_id());
		    tempNode->set_parent(insertionNode->bid());

		    //  Write the node to disk.
		    delete tempNode;
		    tempNode = NULL;
		    insertionNode->update_children_parent();
		}

		//  ...and save the touched node.
		delete insertionNode;
		insertionNode = NULL;
	    }

	    return result;
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	err rstartree<coord_t, BTECOLL>::reinsert(rstarnode<coord_t, BTECOLL>*     n, 
						  const rectangle<coord_t, bid_t>& r, 
						  TPIE_OS_SIZE_T                   level) {

	    rectangle<coord_t, bid_t>                        cover = n->get_child(0);
	    std::vector<std::pair<TPIE_OS_SIZE_T, coord_t> > sortVector;
	    TPIE_OS_SIZE_T                                   counter = 0;

	    err result = NO_ERROR;

	    for (counter=1; counter < n->children(); ++counter) {
		cover.extend(n->get_child(counter));
	    }

	    coord_t midX = (cover.get_left()+cover.get_right()) / 2.0;
	    coord_t midY = (cover.get_lower()+cover.get_upper()) / 2.0;

	    //  Copy all children (pointers to them) along with their MBR's bounding
	    //  rectangle's distance to the centerpoint into an array.
	    for(counter=0; counter < n->children(); ++counter) {
		cover = n->get_child(counter);
		sortVector.push_back(std::pair<TPIE_OS_SIZE_T, coord_t>(
					 counter, 
					 (((cover.get_left()+cover.get_right()) / 2.0) - midX)*
					 (((cover.get_left()+cover.get_right()) / 2.0) - midX) +
					 (((cover.get_lower()+cover.get_upper()) / 2.0) - midY)*
					 (((cover.get_lower()+cover.get_upper()) / 2.0) - midY)));
	    }
	    sortVector.push_back(std::pair<TPIE_OS_SIZE_T, coord_t>(
				     counter, 
				     (((r.get_left()+r.get_right()) / 2.0) - midX)*
				     (((r.get_left()+r.get_right()) / 2.0) - midX) +
				     (((r.get_lower()+r.get_upper()) / 2.0) - midY)*
				     (((r.get_lower()+r.get_upper()) / 2.0) - midY)));    

	    //  Sort by increasing distance to n's centerpoint.
	    sort(sortVector.begin(), sortVector.end(), 
		 sort_by_center_distance<coord_t>());

	    //  Create a new node that recieves the first 70% of the (sorted)
	    //  sequence of children.
	    //  "The experiments have shown that p=30% of M for leaf nodes as
	    //   well as for non-leaf nodes yields the best performance" [BKS93], 327.

	    rstarnode<coord_t, BTECOLL>* newNode = 
		new rstarnode<coord_t, BTECOLL>(storage_area_,
						this, 
						n->get_parent(), 
						NEXT_FREE_BLOCK,
						fanout_);
	    newNode->set_flag(n->get_flag());

	    typename std::vector<std::pair<TPIE_OS_SIZE_T, coord_t> >::iterator vi = sortVector.begin();

	    //  Copy the entries into the new node.
	    for(counter = 0; counter < (n->children() * 70) / 100; ++counter, ++vi) {
		//  Check to see whether we are moving around entries
		//  from the old node or are inserting rectangle r.
		if ((*vi).first == n->children()) {
		    newNode->add_child(r);
		    if (!newNode->is_leaf()) {
			//  Notify each child of its new parent.
			rstarnode<coord_t, BTECOLL>* tempNode = read_node(r.get_id());
			tempNode->set_parent(newNode->bid());

			//  Write the node to disk.
			delete tempNode;
			tempNode = NULL;
		    }
		}
		else {
		    newNode->add_child(n->get_child((*vi).first));	
		    if (!newNode->is_leaf()) {
			//  Notify each child of its new parent.
			rstarnode<coord_t, BTECOLL>* tempNode = read_node(n->get_child((*vi).first).get_id());
			tempNode->set_parent(newNode->bid());

			//  Write the node to disk.
			delete tempNode;
			tempNode = NULL;
		    }
		}
	    }    

	    //  Delete the old node.
	    n->persist(PERSIST_DELETE);

	    rstarnode<coord_t, BTECOLL>* currentNode = newNode;
	    bid_t   lastID      = newNode->bid();
	    bid_t   new_parent   = newNode->get_parent();
	    //  Adjust bounding rectangles along the path to the root.
	    //  Note: At this point the first node in question cannot be the root.
	    do {
	
		//  Compute the parent's new bounding rectangle.
		cover = currentNode->get_child(0);
		cover.set_id(currentNode->bid());
		for (counter = 1; counter < currentNode->children(); ++counter) {
		    cover.extend(currentNode->get_child(counter));
		}	    

		new_parent = currentNode->get_parent();

		//  Save the bid_t necessary for the next iteration.
		if (currentNode == newNode) {
		    lastID    = n->bid();  // We are replacing "n" by "newNode".
		    delete newNode;
		    newNode     = NULL;
		    currentNode = NULL;
		}
		else {
		    lastID    = currentNode->bid();
		}

		//  Delete the old parent node and load the parent's parent.
		delete currentNode;
		currentNode = NULL;
       
		currentNode = read_node(new_parent);

		//  Update the bounding rectangle of the last node in its
		//  parents child array.
		currentNode->set_child(currentNode->find_child(lastID), cover);

	    } while (!currentNode->is_root()); 

	    delete currentNode;
	    currentNode = NULL;

	    //  Reinsert the remaining 30% of the entries.
	    while ((result == NO_ERROR) && (vi != sortVector.end())) {

		if ((*vi).first == n->children()) {
		    reinsertObjects_.push_back(std::pair<rectangle<coord_t, bid_t>, TPIE_OS_SIZE_T>(r, level));
		}
		else {
		    reinsertObjects_.push_back(std::pair<rectangle<coord_t, bid_t>, TPIE_OS_SIZE_T>(n->get_child((*vi).first), level));
		}
		++vi;
	    }

	    return result;

	    //  Don't forget to delete n after returning from this method!
	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstartree<coord_t, BTECOLL>::adjust_tree_on_level(
	    rstarnode<coord_t, BTECOLL>* node1, 
	    rstarnode<coord_t, BTECOLL>* node2,
	    bid_t                        childToBeReplaced,
	    TPIE_OS_SIZE_T               level) {

	    rstarnode<coord_t, BTECOLL>* parent = NULL;
	    children_count_t             index  = 0;

	    parent = read_node(node1->get_parent());
    
	    //  Update the covering rectangle of the actual node in
	    //  its parent node.
	    index  = parent->find_child(childToBeReplaced);     
	    parent->set_child(index, node1->get_covering_rectangle());
    
	    if (parent->is_full()) {
	
		if ((overflowOnLevel_[level]) || (parent->is_root())) {

		    parent->add_child(node2->get_covering_rectangle());
		    node2->set_parent(parent->bid());
	    
		    assert(parent->is_parent(node2));

		    //  Split the parent node.
		    std::pair<rstarnode<coord_t, BTECOLL>*, rstarnode<coord_t, BTECOLL>*> nodeTupel = split_node(parent);

		    bid_t childToBeReplaced = parent->bid();

		    //  Delete the node that has been split.
		    parent->persist(PERSIST_DELETE);
		    delete parent;
		    parent = NULL;
	    
		    rstarnode<coord_t, BTECOLL>* newNode1 = nodeTupel.first;
		    rstarnode<coord_t, BTECOLL>* newNode2 = nodeTupel.second;
	    
		    if (newNode1) {
			//  Check which node is the actual parent of node1.
			if (newNode1->is_parent(node1)) {
			    node1->set_parent(newNode1->bid());
			}
			else {
			    node1->set_parent(newNode2->bid());
			}
			//  Check which node is the actual parent of node2.
			if (newNode1->is_parent(node2)) {
			    node2->set_parent(newNode1->bid());
			}
			else {
			    node2->set_parent(newNode2->bid());
			}		
		    }
	    
		    //  Write the nodes to disk.
		    delete node1;
		    delete node2;
		    node1 = NULL;
		    node2 = NULL;
	    
		    //  The touched new nodes do not have to be saved at this point.
		    //  This is done after the next modifications in 'adjustTree'.
	    
		    //  Propagate the split upwards passing the new nodes.
		    if (newNode1) {
			adjust_tree_on_level(newNode1, newNode2, 
					  childToBeReplaced, level + 1);
		    }
		} 
		else {
		    //  Mark the level.
		    overflowOnLevel_[level] = true;

		    rectangle<coord_t, bid_t> r = node2->get_covering_rectangle();
		    r.set_id(node2->bid());

		    //  Write the nodes to disk.
		    delete node1;
		    delete node2;
		    node1 = NULL;
		    node2 = NULL;

		    //  Force reinsert.
		    reinsert(parent, r, level);

		    //  Delete the old node (it has been substituted in 'reinsert').
		    parent->persist(PERSIST_DELETE);
		    delete parent;
		    parent = NULL;

		}
	    }
	    else {
	
		node2->set_parent(parent->bid());
		parent->add_child(node2->get_covering_rectangle());
	
		assert(parent->is_parent(node1));
		assert(parent->is_parent(node2));

		//  Write all nodes to disk.
		delete parent;
		delete node1;
		delete node2;
		parent = NULL;
		node1 = NULL;
		node2 = NULL;
	    }	

	    handle_reinsertions();
	}

	////////////////////////////////////////////////////////////////////////////

//  R*-tree split heuristic from [BKSS90].
	template<class coord_t, class BTECOLL>
	std::pair<rstarnode<coord_t, BTECOLL>*, rstarnode<coord_t, BTECOLL>*>
	rstartree<coord_t, BTECOLL>::split_node(rstarnode<coord_t, BTECOLL>* toSplit) {

	    rstarnode<coord_t, BTECOLL>* newNode1 =
		new rstarnode<coord_t, BTECOLL>(storage_area_,
						this, 
						toSplit->get_parent(), 
						NEXT_FREE_BLOCK, 
						fanout_);

	    rstarnode<coord_t, BTECOLL>* newNode2 = 
		new rstarnode<coord_t, BTECOLL>(storage_area_,
						this, 
						toSplit->get_parent(), 
						NEXT_FREE_BLOCK,
						fanout_);

	    rstarnode<coord_t, BTECOLL>* newRoot         = NULL;

	    bid_t          newRootPosition = 0;
	    TPIE_OS_SIZE_T counter         = 0;

	    //  Determine split axis and distribution.
	    std::pair<std::vector<rectangle<coord_t, bid_t> >*, TPIE_OS_SIZE_T> seeds = 
		toSplit->choose_split_axis_and_index();

	    TPIE_OS_SIZE_T firstGroupNumber = (TPIE_OS_SIZE_T)(fanout_/ MIN_FANOUT_FACTOR)
 + seeds.second;

	    rectangle<coord_t, bid_t> b1 = (*(seeds.first))[0];
	    rectangle<coord_t, bid_t> b2 = (*(seeds.first))[firstGroupNumber];

	    for(counter = 0; counter < firstGroupNumber; ++counter) {
		newNode1->add_child((*(seeds.first))[counter]);
		b1.extend((*(seeds.first))[counter]);
	    }

	    for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
		newNode2->add_child((*(seeds.first))[counter]);
		b2.extend((*(seeds.first))[counter]);
	    }

	    delete seeds.first;
	    seeds.first = NULL;

	    b1.set_id(newNode1->bid());
	    b2.set_id(newNode2->bid());
	
	    //  Check whether the old node was the root. If so, create a new root.
	    if (toSplit->is_root()) {


		//  Create a new root whose child is the first node.
		newRoot       = new rstarnode<coord_t, BTECOLL>(storage_area_,
								this, 
								NEXT_FREE_BLOCK, 
								NEXT_FREE_BLOCK, 
								fanout_);
		newRoot->set_flag(RNodeTypeRoot);
		newRoot->set_parent(newRoot->bid());
		newRootPosition = newRoot->bid();
		root_position_   = newRoot->bid();

		++tree_height_;

		//  Extend overflow array
		overflowOnLevel_.push_back(false);

		newRoot->add_child(b1);
		newRoot->add_child(b2);
		newNode1->set_parent(newRootPosition);
		newNode2->set_parent(newRootPosition);

		assert(newRoot->is_parent(newNode1));
		assert(newRoot->is_parent(newNode2));

		//  Write the new root to disk.
		delete newRoot;
		newRoot = NULL;

		//  If the old node was a leaf node, the new nodes are leaf nodes, too.
		//  Otherwise the new ones are internal nodes.
		if (toSplit->is_leaf()) {
		    newNode1->set_flag(RNodeTypeLeaf);
		    newNode2->set_flag(RNodeTypeLeaf);
		}
		else {
		    newNode1->set_flag(RNodeTypeInternal);
		    newNode2->set_flag(RNodeTypeInternal);
		}
	    } 
	    else {

		//  The new nodes are of the same kind as the old node.
		newNode1->set_flag(toSplit->get_flag());
		newNode2->set_flag(toSplit->get_flag());
	    }

	    //  Set the correct covering rectangles.
	    newNode1->set_covering_rectangle(b1);
	    newNode2->set_covering_rectangle(b2);

	    //  The children of the new nodes refer to the ID of the old node.
	    //  Therefore these pointers have to be updated.
	    newNode1->update_children_parent();
	    newNode2->update_children_parent();

	    if (newRootPosition) {
		delete newNode1;
		delete newNode2;
		newNode1 = NULL;
		newNode2 = NULL;
	    }

	    //  Return pointers to the new nodes.
	    return std::pair<rstarnode<coord_t, BTECOLL>*, 
		rstarnode<coord_t, BTECOLL>*>(newNode1,newNode2);

	}

	////////////////////////////////////////////////////////////////////////////

// Try to get tree information using the ".info" meta file.
	template<class coord_t, class BTECOLL>
	bool rstartree<coord_t, BTECOLL>::read_tree_information() {

	    bool returnValue;

	    //  Add suffix ".info" to the input file name.
	    std::string treeinfo_filename = name_ + ".info";
    
	    std::cerr << "Looking for info file " << treeinfo_filename << std::endl;
    
	    //  Try to read some info.
	    std::ifstream *treeinfo_file_stream =
		new std::ifstream(treeinfo_filename.c_str());

	    if (!(*treeinfo_file_stream)) {
		root_position_ = 0;
		tree_height_   = 0;
		total_objects_ = 0;
		std::cerr << "No tree information found." << std::endl;
		returnValue = false;
	    }
	    else {
		treeinfo_file_stream->read((char *) &root_position_, 
					   sizeof(bid_t));
		treeinfo_file_stream->read((char *) &tree_height_, 
					   sizeof(TPIE_OS_SIZE_T));
		treeinfo_file_stream->read((char *) &total_objects_, 
					   sizeof(TPIE_OS_OFFSET));
		treeinfo_file_stream->read((char *) &fanout_, 
					   sizeof(TPIE_OS_OFFSET));
		returnValue = true;
	    }
    
	    delete treeinfo_file_stream;

	    return returnValue;

	}

	////////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstartree<coord_t, BTECOLL>::write_tree_information() {

	    //  Add suffix ".info" to the input file name.
	    std::string treeinfo_filename = name_ + ".info";

	    //  Write some info.
	    std::ofstream *treeinfo_file_stream = 
		new std::ofstream(treeinfo_filename.c_str());

	    treeinfo_file_stream->write((char *) &root_position_, 
					sizeof(bid_t));
	    treeinfo_file_stream->write((char *) &tree_height_, 
					sizeof(TPIE_OS_SIZE_T));
	    treeinfo_file_stream->write((char *) &total_objects_, 
					sizeof(TPIE_OS_OFFSET));  
	    treeinfo_file_stream->write((char *) &fanout_, 
					sizeof(TPIE_OS_SIZE_T));  
    
	    delete treeinfo_file_stream;
	}

	////////////////////////////////////////////////////////////////////////////


    }  //  ami namespace

}  // tpie namespace

#endif

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
//  @inproceedings{Gutt84
//  , author = 	     "Antonin Guttman"
//  , title = 	     "{R}-trees: {A} Dynamic Index Structure for Spatial 
//                    Searching"
//  , pages = 	     "47--57" 
//  , booktitle =    "{SIGMOD} '84, Proceedings of Annual Meeting"
//  , year = 	     1984
//  , editor = 	     "Beatrice Yormark"
//  , volume = 	     "14.2"
//  , series = 	     "{SIGMOD} Record"
//  , month = 	     "June"
//  , publisher =    "{ACM} Press"
//  }


//
//   End of File.
//

