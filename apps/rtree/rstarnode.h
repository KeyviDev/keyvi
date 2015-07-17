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
//  Description:     declarations for class RStarNode
//  Created:         05.11.1998
//  Author:          Jan Vahrenhold
//  mail:            jan@math.uni-muenster.de
//  $Id: rstarnode.h,v 1.5 2005-11-15 15:33:41 jan Exp $
//  Copyright (C) 1997-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

//  Prevent multiple includes.
#ifndef _TPIE_AMI_RSTARNODE_H
#define _TPIE_AMI_RSTARNODE_H

#include "app_config.h"

#include <tpie/portability.h>

//  Include <iostream.h> for output operator.
#include <iostream>

//  Include STL templates pair, list and vector.
#include <utility>
#include <list>
#include <vector>

#include <cassert>

//  Include TPIE AMI declarations.
#include <tpie/stream.h>
#include <tpie/block.h>
#include <tpie/coll.h>
#include <tpie/stack.h>
#include <tpie/vararray.h>

//  Include declaration of bounding boxes.
#include "rectangle.h"
#include "rectangle_comparators.h"

#include "rstarnode_info.h"

namespace tpie {

    namespace ami {

//  Define constants for describing the kind of node we are looking at.
	const unsigned short RNodeTypeInternal = 1;
	const unsigned short RNodeTypeLeafNode = 2;
	const unsigned short RNodeTypeRoot     = 4;
	const unsigned short RNodeTypeLeaf     = 8;

	const double MIN_FANOUT_FACTOR = 2.5;

//  Forward declaration of R-Tree base class.
	template<class coord_t, class BTECOLL> class rstartree;


////////////////////////////////////////////////////////////////////////////////
/// This class models the nodes of an \ref rstartree.
////////////////////////////////////////////////////////////////////////////////
	template<class coord_t, class BTECOLL = bte::COLLECTION>
	class rstarnode: public block<rectangle<coord_t, bid_t>, _rstarnode_info, BTECOLL> {
	public:

	    using block<rectangle<coord_t, bid_t>, _rstarnode_info, BTECOLL>::info;
	    using block<rectangle<coord_t, bid_t>, _rstarnode_info, BTECOLL>::el;
	    using block<rectangle<coord_t, bid_t>, _rstarnode_info, BTECOLL>::lk;
	    using block<rectangle<coord_t, bid_t>, _rstarnode_info, BTECOLL>::dirty;

            ///////////////////////////////////////////////////////////////////////////
	    /// The constructor expects a pointer to the block collection, the instance
	    /// of the rstartree class this node belongs to (this instance is in
	    /// main memory all the time), the ID of the parent node (i.e. the
	    /// ID of the block that stores the parent node), the ID of the block 
	    /// where the leaf is to be stored, and the maximum number of children 
	    /// for this node. 
	    /// \param[in] pcoll block collection
	    /// \param[in] tree R*-tree
	    /// \param[in] parent ID of the parent node
	    /// \param[in] id ID of the node
	    /// \param[in] max_children maximum number of children
            ///////////////////////////////////////////////////////////////////////////
	    rstarnode(collection_single<BTECOLL>*  pcoll, 
		      rstartree<coord_t, BTECOLL>* tree, 
		      bid_t                        parent, 
		      bid_t                        id, 
		      children_count_t             max_children);

            ///////////////////////////////////////////////////////////////////////////
	    /// Returns the tree the node bekongs to.
            ///////////////////////////////////////////////////////////////////////////
	    rstartree<coord_t, BTECOLL>* tree() const { 
		return tree_; 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Set the ID indicating the block the parent node is stored in.
	    /// \param[in] parent the new ID of the parent node's block
            ///////////////////////////////////////////////////////////////////////////
	    void set_parent(bid_t parent) { 
		info()->parent = parent; 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Get the ID indicating the block the parent node is stored in.
            ///////////////////////////////////////////////////////////////////////////
	    bid_t get_parent() const { 
		return info()->parent; 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Place a child in a given slot. 
	    /// \param[in] index the number of the child to be set
	    /// \param[in] bb the bounding box of the child
            ///////////////////////////////////////////////////////////////////////////
	    void set_child(children_count_t                 index, 
			   const rectangle<coord_t, bid_t>& bb) {
		assert(index < max_children_+1);  
		// The node is allowed to _temporarily_ overflow.
		el[index] = bb;    
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Add a new child in the next free slot.
	    /// \param[in] bb the bounding box of the child
            ///////////////////////////////////////////////////////////////////////////
	    void add_child(const rectangle<coord_t, bid_t>& bb) {
		set_child(children(), bb);
		++info()->children;
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Get the child stored in a given slot. 
	    /// \param[in] index the number of the slot
            ///////////////////////////////////////////////////////////////////////////
	    const rectangle<coord_t, bid_t>& get_child(children_count_t index) const {
		assert(index < children());
		return el[index]; 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Check whether this node is the parent of another node
	    /// \param[in] other the potential child node
            ///////////////////////////////////////////////////////////////////////////
	    bool is_parent(const rstarnode<coord_t, BTECOLL>* other) const;

            ///////////////////////////////////////////////////////////////////////////
	    /// Check whether there is a child with a given ID. If so, the index of the 
	    /// child, otherwise the branching factor of the tree is returned.
	    /// \param[in] id the ID to be looked for 
            ///////////////////////////////////////////////////////////////////////////
	    children_count_t find_child(bid_t id) const;
	    
            ///////////////////////////////////////////////////////////////////////////
	    /// Remove a given child (replace it by the child in the last slot) and
	    /// return the new number of children.
	    /// \param[in] r the child to delete 
            ///////////////////////////////////////////////////////////////////////////
	    children_count_t remove_child(const rectangle<coord_t, bid_t>& r);

            ///////////////////////////////////////////////////////////////////////////
	    /// Remove a child from a given slot (replace it by the child in the
	    /// last slot) and return the new number of children.
	    /// \param[in] index the slot from which to delete 
            ///////////////////////////////////////////////////////////////////////////
	    children_count_t remove_child(children_count_t index);

            ///////////////////////////////////////////////////////////////////////////
	    /// Set the flag used to determine the kind of node represented.
	    /// \param[in] flag the new flag
            ///////////////////////////////////////////////////////////////////////////
	    void set_flag(unsigned short flag) { 
		info()->flag = flag; 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Get the flag used to determine the kind of node represented.
            ///////////////////////////////////////////////////////////////////////////
	    unsigned short get_flag() const { 
		return info()->flag; 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Find the index of the child whose bounding box needs least enlargement
	    /// to include the given bounding box and adjust that child to include
	    /// the given box as well.
	    /// \param[in] bb the object to be routed
            ///////////////////////////////////////////////////////////////////////////
	    children_count_t route(const rectangle<coord_t, bid_t>& bb);

            ///////////////////////////////////////////////////////////////////////////
	    /// Given the ID (not the index!) of a child node and a bounding box 
	    /// adjust the bounding box of the child to include the given object.
	    /// \param[in] id id of the child node to be updated
	    /// \param[in] bb new bounding box
            ///////////////////////////////////////////////////////////////////////////
	    void adjust_bounding_rectangle(
		bid_t                            id, 
		const rectangle<coord_t, bid_t>& bb);

            ///////////////////////////////////////////////////////////////////////////
	    /// Returns the number of children of this node.
            ///////////////////////////////////////////////////////////////////////////
	    children_count_t children() const { 
		return info()->children; 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Print a debugging output of the children of this node to std::cerr.
            ///////////////////////////////////////////////////////////////////////////
	    void show_children() const;

            ///////////////////////////////////////////////////////////////////////////
	    /// Update the parent-pointer of all children to point to this node.
            ///////////////////////////////////////////////////////////////////////////
	    void update_children_parent() const;

            ///////////////////////////////////////////////////////////////////////////
	    /// Set the covering rectangle of the node. There is no sanity check!
	    /// \param[in] bb the new covering rectangle
            ///////////////////////////////////////////////////////////////////////////
	    void set_covering_rectangle(const rectangle<coord_t, bid_t>& bb) {
		covering_rectangle_ = bb;
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Returns the covering rectangle of the node. If no rectangle has been 
	    /// set/computed before, compute the rectangle from scratch.
            ///////////////////////////////////////////////////////////////////////////
	    rectangle<coord_t, bid_t> get_covering_rectangle() {
		if (covering_rectangle_.get_id() != bid_t()) {
		    covering_rectangle_ = get_child(0);
		    for(children_count_t c = 1; c < children(); ++c) {
			covering_rectangle_.extend(get_child(c));
		    }
		    covering_rectangle_.set_id(bid_t());
		}
		return covering_rectangle_;
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Do we have to split the node if we want to add another child?
            ///////////////////////////////////////////////////////////////////////////
	    bool is_full() const {
		// Note to self: check why there is a "+2" instead of "+1"
		return (children() + 2 >= max_children_); 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Is this node the tree's root?
            ///////////////////////////////////////////////////////////////////////////
	    bool is_root() const { 
		return ((get_flag() & RNodeTypeRoot) == RNodeTypeRoot); 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Is this node an internal node?
            ///////////////////////////////////////////////////////////////////////////
	    bool is_internal_node() const { 
		return ((get_flag() & RNodeTypeInternal) == RNodeTypeInternal); 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Is this node a leaf node?
            ///////////////////////////////////////////////////////////////////////////
	    bool is_leaf_node() const { 
		return ((get_flag() & RNodeTypeLeafNode) == RNodeTypeLeafNode); 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// Is this node a leaf?
            ///////////////////////////////////////////////////////////////////////////
	    bool is_leaf() const {
		return ((get_flag() & RNodeTypeLeaf) == RNodeTypeLeaf); 
	    }

            ///////////////////////////////////////////////////////////////////////////
	    /// This method realizes the "single shot" query algorithm for R-trees.
	    /// Given a query rectangle we look for all subtrees whose bounding
	    /// rectangle overlaps the query rectangle and proceed recursively
	    /// for each such subtree. If the current node is a leaf, all overlapping
	    /// rectangles are written to the output stream, otherwise all
	    /// overlapping bounding rectangles of subtrees are pushed on
	    /// the stack. 
	    /// \param[in] bb object to be found
	    /// \param     candidates stack of nodes to be visited
	    /// \param     matches matches found so far
	    /// \param     candidatesCounter number of candidated to be visited
	    /// \param     leafCounter (for statistical puposes)
	    /// \param[in] bruteForce whether or not to query the whole tree
            ///////////////////////////////////////////////////////////////////////////
	    void query(
		const rectangle<coord_t, bid_t>&    bb, 
		ami::stack<bid_t>*                  candidates, 
		stream<rectangle<coord_t, bid_t> >* matches,
		TPIE_OS_OFFSET&                     candidatesCounter,
		TPIE_OS_OFFSET&                     leafCounter,
		bool                                bruteForce = false) const;

            ///////////////////////////////////////////////////////////////////////////
	    /// This method realizes a depth-first search looking for a node
	    /// with a given ID and prints all its contents. \par 
	    /// This method is used for debugging purposes.
	    /// \param[in] nodeID ID of the object to be found
	    /// \param     candidates stack of nodes to be visited
	    /// \param     candidatesCounter number of candidates to be visited
            ///////////////////////////////////////////////////////////////////////////
	    void find_node(
		bid_t              nodeID,
		ami::stack<bid_t>* candidates, 
		TPIE_OS_OFFSET&    candidatesCounter) const;

            ///////////////////////////////////////////////////////////////////////////
	    /// This method realizes a depth-first search looking for a leaf
	    /// containing a given object and returns this leaf's ID (or 0 if
	    /// the search was unsuccessful. Note that this 
	    /// algorithm keeps track of all node to be visited in INTERNAL
	    /// memory.\par
	    /// This method is used for debugging purposes.
	    /// \param[in] r object to be found
	    /// \param     candidates list of nodes to be visited
            ///////////////////////////////////////////////////////////////////////////
	    bid_t find_leaf(const rectangle<coord_t, bid_t>& r, 
			   std::list<bid_t>*                 candidates) const;

            ///////////////////////////////////////////////////////////////////////////
	    /// This method is called as a subroutine from the tree-checking
	    /// procedure and appends the ID and the bounding box of the
	    /// current node to the given list (in main-memory). \par
	    /// This method is used for debugging purposes.
	    /// \param[in] bb the node to be checked
	    /// \param     l the list of objects to be checked
	    /// \param     objectCounter number of objects checked so far
            ///////////////////////////////////////////////////////////////////////////
	    void check_children(
		const rectangle<coord_t, bid_t>&                          bb, 
		std::list<std::pair<bid_t, rectangle<coord_t, bid_t> > >& l, 
		TPIE_OS_OFFSET&                                           objectCounter);

            ///////////////////////////////////////////////////////////////////////////
	    /// This method returns the axis perpendicular to which the split
	    /// will be performed and the index of the distribution (according to 
	    /// [BKSS90] (p.326).)
            ///////////////////////////////////////////////////////////////////////////
	    std::pair<std::vector<rectangle<coord_t, bid_t> >*, children_count_t> choose_split_axis_and_index() const;

	protected:
	    //  No protected members.

	private:

	    rstartree<coord_t, BTECOLL>* tree_;               
	    //  Pointer to the tree.

	    children_count_t             max_children_;        
	    //  Maximum number of children allowed for this node.

	    rectangle<coord_t, bid_t>    covering_rectangle_;  
	    //  Can be set by user.

            ///////////////////////////////////////////////////////////////////////////
	    /// The default constructor is forbidden since it makes no sense to 
	    /// create a node without e.g. a tree.
	    /// The assigment operator/copy constructor is not defined to avoid 
	    /// several nodes refering to the same block.
            ///////////////////////////////////////////////////////////////////////////
	    rstarnode();
	    rstarnode(const rstarnode<coord_t, BTECOLL>& other);
	    rstarnode<coord_t, BTECOLL>& operator=(const rstarnode<coord_t, BTECOLL>& other);
	};

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	std::ostream& operator<<(std::ostream& os, const rstarnode<coord_t, BTECOLL>& r) {
	    os << "ID: "       << r.bid() 
	       << ", parent: " << r.get_parent() 
	       << ", flag: "   << r.get_flag();
	    if (r.is_root()) {
		os << " (root)";
	    }
	    if (r.is_internal_node()) {
		os << " (internal node)";
	    }
	    if (r.is_leaf_node()) {
		os << " (leaf node)";
	    }
	    if (r.is_leaf()) {
		os << " (leaf)";
	    }
	    return os;
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	rstarnode<coord_t, BTECOLL>::rstarnode(collection_single<BTECOLL>*  pcoll, 
					       rstartree<coord_t, BTECOLL>* tree, 
					       bid_t                        parent, 
					       bid_t                        id, 
					       children_count_t             max_children): 
	    block<rectangle<coord_t, bid_t>, _rstarnode_info, BTECOLL>(pcoll, 0, id), tree_(tree) {
    
	    //  Initialize the four info fields.
	    if (id == 0) {
		info()->parent = parent;
		info()->children = 0;
		info()->flag = 0;
	    }

	    //  If a user-defined maximum size of children is given, check whether
	    //  this number fits into the block and is not equal to zero.
	    //  el.capacity is small, so it is safe to cast.
	    max_children_ = static_cast<children_count_t>(el.capacity());
	    if (max_children <= max_children_) {
		max_children_ = std::max((children_count_t) 2, max_children);
	    }
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	children_count_t rstarnode<coord_t, BTECOLL>::find_child(bid_t id) const {
	    children_count_t counter;
	    children_count_t returnValue_ = max_children_;
  
	    //  Check all children to find the index of the child with 
	    //  given ID.
	    for(counter = 0; counter < children(); ++counter) {
		if (el[counter].get_id() == id)
		    returnValue_ = counter;
	    }

	    // [tavi] this assert should not be here.
	    assert(returnValue_ < max_children_);
	    return returnValue_;
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	bool rstarnode<coord_t, BTECOLL>::is_parent(const rstarnode<coord_t, BTECOLL>* other) const {
	    children_count_t counter = 0;
	    children_count_t returnValue_ = max_children_;
	    bid_t            id = other->bid();
  
	    //  Check all children to find the index of the child with 
	    //  the given ID. If there is no such child, the predicate 
	    //  returns false.
	    for(counter = 0; counter < children(); ++counter) {
		if (el[counter].get_id() == id)
		    returnValue_ = counter;
	    }

	    return (returnValue_ < max_children_);
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstarnode<coord_t, BTECOLL>::adjust_bounding_rectangle(
	    bid_t id, 
	    const rectangle<coord_t, bid_t>& bb) {  

	    //  Search the child with the matching ID and adjust its bounding
	    //  rectangle to include the given bounding box.
	    for(children_count_t c = 0; c < children(); ++c) {
		if (el[c].get_id() == id) {
		    el[c].extend(bb);
		}
	    }
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstarnode<coord_t, BTECOLL>::show_children() const {

	    //  Print all children.
	    for(children_count_t c = 0; c < children(); ++c)
		std::cout << "  " << el[c] << std::endl;
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	children_count_t rstarnode<coord_t, BTECOLL>::remove_child(const rectangle<coord_t, bid_t>& r) {
	    children_count_t childFound = children();

	    //  Find the child to be deleted. 
	    for(children_count_t c = 0; c < children(); ++c) {
		if (el[c] == r) {
		    childFound = c;
		    break;
		}
	    }

	    // If the child has been found replace it by the last child.
	    if (childFound < children()) {
		if (childFound < children() - 1)
		    el[childFound] = el[children()-1];
		el[children()-1] = rectangle<coord_t, bid_t>();
		--info()->children;
	    }

	    return children();
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	children_count_t rstarnode<coord_t, BTECOLL>::remove_child(children_count_t index) {

	    if (index < children()) {
		if (index < children() -1)
		    el[index] = el[children()-1];
		el[children()-1] = rectangle<coord_t, bid_t>();
		--info()->children;
	    }

	    return children();
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstarnode<coord_t, BTECOLL>::check_children(
	    const rectangle<coord_t, bid_t>&                          bb,
	    std::list<std::pair<bid_t, rectangle<coord_t, bid_t> > >& l,
	    TPIE_OS_OFFSET&                                           objectCounter) {
	    
	    rectangle<coord_t, bid_t> toCompare = el[0];
	    toCompare.set_id(bid_t());
	    
	    //  Compute the bounding box of all children. If the child is the root
	    //  of a non-trivial subtree, push its ID on the stack.
	    for( children_count_t c = 0; c < children(); ++c) {
		toCompare.extend(el[c]);
		if (!is_leaf()) {
		    l.push_back(std::pair<bid_t, rectangle<coord_t, bid_t> >(el[c].get_id(), el[c]));
		} else {
		    ++objectCounter;
		}
	    }

	    //  Compare the computed bounding box against the bounding box 'bb'
	    //  that has been stored this node's bounding box its father.
	    if (!(toCompare == bb)) {
		if (!is_root()) {
		    std::cout << "Test failed for " << bid_t() 
			      << ", parent: " << get_parent() 
			      << ", flag: " << get_flag() << std::endl
			      << toCompare << " should be " << std::endl 
			      << bb << std::endl;
		    show_children();
		    abort();
		}
	    }
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstarnode<coord_t, BTECOLL>::update_children_parent() const {

	    bid_t                        child_id = 0;
	    rstarnode<coord_t, BTECOLL>* n = NULL;
  
	    //  Updating is not possible on leaf-node level.
	    if (is_leaf() == false) {
		for(children_count_t c = 0; c < children(); ++c) {
		    child_id = el[c].get_id();
      
		    //  Read the child 'counter'.
		    n = tree()->read_node(child_id);
      
		    //  Check whether the parent pointer needs to be updated.
		    if (n->get_parent() != bid_t()) {
			//  If so, set the parent pointer the actual ID...
			n->set_parent(bid_t());		
		    }

		    //  Write the node to disk.
		    delete n;
		    n = NULL;
		}
	    }
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstarnode<coord_t, BTECOLL>::query(
	    const rectangle<coord_t, bid_t>&    bb, 
	    ami::stack<bid_t>*                  candidates, 
	    stream<rectangle<coord_t, bid_t> >* matches,
	    TPIE_OS_OFFSET&                     candidatesCounter, 
	    TPIE_OS_OFFSET&                     leafCounter, 
	    bool                                bruteForce) const {

	    rectangle<coord_t, bid_t> rb;
  
	    if (is_leaf()) {
		//  If the current node is a leaf, write all children that overlap
		//  the given rectangle 'bb' to the output stream 'matches'.
		for(children_count_t c = 0; c < children(); ++c) {
		    rb = el[c];
		    ++leafCounter;
		    if (bb.intersects(rb)) {
			matches->write_item(rb);
		    }
		}
	    } else {
		//  If the current node is not a leaf, push all children that 
		//  overlap the given rectangle 'bb' onto the stack 'candidates'.
		//  Increment the size counter of the stack accordingly.
		//  If the flag 'bruteForce' is true, push all children onto the
		//  stack, i.e. perform a depth-first traversal of the tree.
		for(children_count_t c = 0; c < children(); ++c) {
		    rb = el[c];
		    if ((bruteForce) || (bb.intersects(rb))) {
			candidates->push(rb.get_id());
			++candidatesCounter;
		    }
		}
	    }
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	bid_t rstarnode<coord_t, BTECOLL>::find_leaf(
	    const rectangle<coord_t, bid_t>& r, 
	    std::list<bid_t>*                candidates) const {

	    rectangle<coord_t, bid_t> rb;

	    if (is_leaf()) {
		//  If the current node is a leaf, write all children that overlap
		//  the given rectangle 'bb' to the output stream 'matches'.
		for(children_count_t c = 0; c < children(); ++c) {
		    if (el[c] == r) {
			return bid_t();
		    }
		}
	    } else {
		//  If the current node is not a leaf, push all children that 
		//  overlap the given rectangle 'bb' onto the stack 'candidates'.
		//  Increment the size counter of the stack accordingly.
		//  If the flag 'bruteForce' is true, push all children onto the
		//  stack, i.e. perform a depth-first traversal of the tree.
		for(children_count_t c = 0; c < children(); ++c) {
		    rb = el[c];
		    if (r.intersects(rb)) {
			candidates->push_front(rb.get_id());
		    }
		}  
	    }

	    return (bid_t) 0;
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	void rstarnode<coord_t, BTECOLL>::find_node(
	    bid_t              nodeID, 
	    ami::stack<bid_t>* candidates, 
	    TPIE_OS_OFFSET&    candidatesCounter) const {

	    if (is_leaf()) {
		//  If the current node is a leaf, check whether its ID matches
		//  the ID in question. If so, print the leaf and its children.
		if (bid_t() == nodeID) {
		    std::cout << bid_t() 
			      << ", parent: " << get_parent() 
			      << ", flag: "   << get_flag() << std::endl;
		    show_children();
		}
	    } else {
		//  If the current node is not a leaf, check whether the ID of
		//  one of its children matches the ID in question. Push all 
		//  children onto the stack and adjust the size counter of the
		//  stack accordingly (perform a depth-first traversal of the tree).
		for(children_count_t c = 0; c < children(); ++c) {
		    rectangle<coord_t, bid_t> rb = el[c];
		    if (rb.get_id() == nodeID) {
			std::cout << rb << std::endl;
		    }
		    candidates->push(rb.get_id());
		    ++candidatesCounter;
		}
	    }
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	children_count_t rstarnode<coord_t, BTECOLL>::route(const rectangle<coord_t, bid_t>& bb) {
	    children_count_t returnValue_ = 0;
	    coord_t          area;
	    coord_t          perimeter;
	    coord_t          otherArea;
	    coord_t          otherPerimeter;
 
	    assert (children() > 0);

	    //  For each child compute the area of its bounding rectangle
	    //  extended so that it includes the given bounding box.
	    area      = el[0].extended_area(bb) - el[0].area();
	    perimeter = el[0].width() + el[0].height();
    
	    for(children_count_t c = 1; c < children(); ++c) {
		otherArea      = el[c].extended_area(bb) - el[c].area();
		otherPerimeter = el[c].width() + el[c].height();
      
		//  Check for which child the difference between the enlarged
		//  area and the original area is minimal. If the difference
		//  is the same for two children, select the child whose 
		//  bounding rectangle has smaller perimeter.	
		if ((otherArea < area) ||
		    ((otherArea == area) && (otherPerimeter < perimeter))){
		    area         = otherArea;
		    perimeter    = otherPerimeter;
		    returnValue_ = c;
		}
	    }

	    el[returnValue_].extend(bb);

	    return returnValue_;
	}

	///////////////////////////////////////////////////////////////////////////

	template<class coord_t, class BTECOLL>
	std::pair<std::vector<rectangle<coord_t, bid_t> >*, children_count_t> 
	rstarnode<coord_t, BTECOLL>::choose_split_axis_and_index() const {

	    const unsigned short dim = 2;
	    std::vector<rectangle<coord_t, bid_t> >* toSort[dim];
	    children_count_t                         c = 0;
	    children_count_t                         c2 = 0;
	    unsigned short                           dimC = 0;

	    coord_t S[dim];
    
	    for (dimC = 0; dimC < dim; ++dimC) {

		toSort[dimC] = new std::vector<rectangle<coord_t, bid_t> >;

		for (c = 0; c < children(); ++c) {
		    toSort[dimC]->push_back(el[c]);
		}
	    }


	    children_count_t firstGroupMinSize = 
		(children_count_t)(max_children_ / MIN_FANOUT_FACTOR);

	    children_count_t distributions = max_children_ - 2*firstGroupMinSize + 1;

	    //  area-value:    area[bb(first group)] +
	    //                 area[bb(second group)]
	    //  margin-value:  margin[bb(first group)] +
	    //                 margin[bb(second group)]
	    //  overlap-value: area[bb(first group) $\cap$ bb(second group)]

	    VarArray2D<coord_t> areaValue(dim,distributions);
	    VarArray2D<coord_t> marginValue(dim,distributions);
	    VarArray2D<coord_t> overlapValue(dim,distributions);

	    //  "For each axis 
	    //     Sort the entries by their lower then by their upper 
	    //     value of their rectangles and determine all 
	    //     distributions as described above. Compute S, the
	    //     sum of all margin-values of the different 
	    //     distributions.
	    //   end."

	    rectangle<coord_t, bid_t> group[dim];
	    rectangle<coord_t, bid_t> firstGroup;
	    rectangle<coord_t, bid_t> secondGroup;

	    for (dimC = 0; dimC < dim; ++ dimC) {
	
		if (dimC == 0) {
		    //  Process x-axis.
		    sort(toSort[0]->begin(), toSort[0]->end(),
			 sort_boxes_along_x_axis<coord_t>());
		} 
		else {
		    //  Process y-axis.
		    sort(toSort[1]->begin(), toSort[1]->end(), 
			 sort_boxes_along_y_axis<coord_t>());
		}

		S[dimC] = 0.0;

		firstGroup  = (*toSort[dimC])[0];
		secondGroup = (*toSort[dimC])[children()];
    
		//  The first group contains at least the first "firstGroupMinSize" 
		//  boxes while the second group contains at least the last
		//  "firstGroupMinSize" boxes. This is true for all distributions.
		for (c = 1; c < firstGroupMinSize; ++c) {
		    firstGroup.extend((*toSort[dimC])[c]);
		    secondGroup.extend((*toSort[dimC])[children()-c]);
		}
    
		//  Iterate over all possible distributions.
		for (c = 0; c < distributions; ++c) {
	    
		    //  Initialize groups.
		    group[0] = firstGroup;
		    group[1] = secondGroup;
	    
		    //  Update first group.
		    for (c2 = firstGroupMinSize; c2 < firstGroupMinSize+c; ++c2) {
			group[0].extend((*toSort[dimC])[c2]);
		    } 
	    
		    //  Update second group.
		    for (c2 = (firstGroupMinSize + c); c2 < children(); ++c2) {
			group[1].extend((*toSort[dimC])[c2]);
		    }
	    
		    //  Compute area-value, margin-value and overlap-value.
		    areaValue(dimC,c)    = group[0].area() + group[1].area();
		    marginValue(dimC,c)  = group[0].width() + 
			group[0].height() + group[1].width() + group[1].height();
		    overlapValue(dimC,c) = group[0].overlap_area(group[1]);

		    //  Update S.
		    S[dimC] += marginValue(dimC,c);
		}
	    }
    
    
	    //  "Choose the axis with the minimum S as split axis."
	    unsigned short   splitAxis = 0;
	    children_count_t bestSoFar = 0;
	    coord_t minS = S[0];

	    for (dimC = 1; dimC < dim; ++dimC) {
		if (S[dimC] < minS) {
		    minS      = S[dimC];
		    splitAxis = dimC;
		}
	    }

	    for (dimC = 0; dimC < dim; ++dimC) {
		if (dimC != splitAxis) {
		    delete toSort[dimC];
		    toSort[dimC] = NULL;
		}
	    }

	    //  "Along the chosen split axis, choose the
	    //   distribution with the minimum overlap-value.
	    //   resolve ties by choosing the distribution with
	    //   minimum area-value."

	    for(c = 1; c < distributions; ++c) {
		if ((overlapValue(splitAxis,c) < overlapValue(splitAxis,bestSoFar)) ||
		    ((overlapValue(splitAxis,c) == overlapValue(splitAxis,bestSoFar)) && 
		     (areaValue(splitAxis,c) < areaValue(splitAxis,bestSoFar)))) {
		    bestSoFar = c;
		}
	    }

	    return std::pair<std::vector<rectangle<coord_t, bid_t> >*, children_count_t>(toSort[splitAxis], bestSoFar);
	}

	///////////////////////////////////////////////////////////////////////////


    }  //  ami namespace

}  //  tpie namespace


#endif

