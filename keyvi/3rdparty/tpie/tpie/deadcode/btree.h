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

#ifndef _TPIE_AMI_BTREE_H
#define _TPIE_AMI_BTREE_H
///////////////////////////////////////////////////////////////////////////
/// \file tpie/btree.h
/// Definition and implementation of the TPIE B-tree data type.
///////////////////////////////////////////////////////////////////////////

// STL files.
#include <algorithm>
#include <map>
#include <stack>
#include <vector>
#include <functional>
#include <fstream>
#include <string>

// Get a stream implementation.
#include <tpie/stream.h>
// Get templates for sort.
#include <tpie/sort.h>
// Get a collection implementation.
#include <tpie/coll.h>
// Get a block implementation.
#include <tpie/block.h>
// The cache manager.
#include <tpie/cache.h>
// The stats_tree class for tree statistics.
#include <tpie/stats_tree.h>
// The tpie_tempnam() function
#include <tpie/tempname.h>

namespace tpie {

    namespace ami {
	
/// Determines how elements are stored in a leaf. If set to 0, elements are
/// stored in the order in which they are inserted, which may results in
/// slower queries. If set to 1, elements are stored in a sorted list, which
/// may result in slower insertions when allowing duplicate keys (see
/// below).
#ifndef BTREE_LEAF_ELEMENTS_SORTED
#  define BTREE_LEAF_ELEMENTS_SORTED 1
#endif

/// Determines whether to allow duplicate keys when inserting and bulk
/// loading. Support for duplicate keys is incomplete, so you might
/// experience errors when setting this to 0.
#ifndef BTREE_UNIQUE_KEYS
# define BTREE_UNIQUE_KEYS 1
#endif

/// Determines whether "previous" pointers are maintained for leaves.
/// Don't set to 0! There is good reason to maintain prev pointers:
/// computing predecessor queries. Unless maintaining previous pointers
/// proves costly, we keep them.
#ifndef BTREE_LEAF_PREV_POINTER
#  define BTREE_LEAF_PREV_POINTER 1
#endif

    }  //  ami namespace

}  // tpie namespace


namespace tpie {

    namespace ami {
    /** TPIE btree validity status */
    enum btree_status {
    /** Signaling validity of the btree. */ 
	    BTREE_STATUS_VALID,
      /** Signaling invalidity of the btree. */ 
	    BTREE_STATUS_INVALID
	};

    }  //  ami namespace

}  // tpie namespace



namespace tpie {
    
    namespace ami {
	
	
    ///////////////////////////////////////////////////////////////////////////
    /// The btree_params class encapsulates all user-definable B-tree parameters.
    /// These parameters dictate the layout of the tree and its behavior under
    /// insertions and deletions. An instance of the class created using the 
    /// default constructor gives default values to all parameters. Each
    /// parameter can then be changed independently.  
    ///////////////////////////////////////////////////////////////////////////
    class btree_params {

	public:

	    /** Min number of Value's in a leaf. 0 means use default B-tree behavior. */
	    TPIE_OS_SIZE_T leaf_size_min;
	    /** Min number of Key's in a node. 0 means use default B-tree behavior. */
	    TPIE_OS_SIZE_T node_size_min;
	    /** Max number of Value's in a leaf. 0 means use all available capacity. */
	    TPIE_OS_SIZE_T leaf_size_max;
	    /**  Max number of Key's in a node. 0 means use all available capacity. */
	    TPIE_OS_SIZE_T node_size_max;
	    /**  How much bigger is the leaf logical block than the system block. */
	    TPIE_OS_SIZE_T leaf_block_factor;
	    /**  How much bigger is the node logical block than the system block. */
	    TPIE_OS_SIZE_T node_block_factor;
	    /** The max number of leaves cached. */
	    TPIE_OS_SIZE_T leaf_cache_size;
	    /** The max number of nodes cached. */
	    TPIE_OS_SIZE_T node_cache_size;

	    
	    ///////////////////////////////////////////////////////////////////////////
      /// Construtor setting default parameter values.
	    /// The default parameter values are as follows:
	    /// \par leaf_size_min0
	    /// \par node_size_min 0 
	    /// \par leaf_size_max 0 
	    /// \par node_size_max 0 
	    /// \par leaf_block_factor 1 
	    /// \par node_block_factor 1 
	    /// \par leaf_cache_size 5 
	    /// \par node_cache_size 10 
	    ///////////////////////////////////////////////////////////////////////////
	    btree_params(): 
		leaf_size_min(0), node_size_min(0), 
		leaf_size_max(0), node_size_max(0),
		leaf_block_factor(1), node_block_factor(1), 
		leaf_cache_size(32), node_cache_size(64) {
		//  No code in this constructor.
	    }
	};
	

  /** A global object storing the default parameter values. */
	const btree_params btree_params_default = btree_params();
	
	// Forward references.
	template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL> 
	class btree_leaf;
	template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL> 
	class btree_node;
	
	///////////////////////////////////////////////////////////////////////////
  /// An implementation of the B<sup>+</sup>-tree. The btree class implements the
  /// behavior of a dynamic B<sup>+</sup>-tree or (a,b)-tree storing fixed-size data
  /// items. All data elements (of type <em>Value</em>) are stored in the leaves of
  /// the tree, with internal nodes containing keys (of type <em>Key</em>) and links
  /// to other nodes. The keys are ordered using the Compare function
  /// object, which should define a strict weak ordering (as in the STL sorting
  /// algorithms). Keys are extracted from the <em>Value</em> data elements using
  /// the <em>KeyOfValue</em> function object.
  /// 
  /// @param Key The key type.
  /// @param Value The type of the data elements.
  /// @param Compare A function object which defines a strict weak ordering for the keys.
  /// @param KeyOfValue A function object for extracting a <em>Key</em> from a <em>Value</em>.
  /// @param BTECOLL The underlying BTE collection type. It defaults to  bte::COLLECTION.
  ///////////////////////////////////////////////////////////////////////////
	template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL = bte::COLLECTION>
	class btree {

	public:
	    
	    typedef btree_node<Key, Value, Compare, KeyOfValue, BTECOLL> node_t;
	    typedef btree_leaf<Key, Value, Compare, KeyOfValue, BTECOLL> leaf_t;
	    typedef collection_single<BTECOLL> collection_t;
	    typedef Key key_t;
	    typedef Value record_t;
	    typedef btree_params params_t;

	    //////////////////////////////////////////////////////////////////////////
      /// Default filter for range queries.
      /// This is a function object that always returns true 
      /// (i.e., it lets every result of the query pass through).
      //////////////////////////////////////////////////////////////////////////
	    class dummy_filter_t {
	    public:
		bool operator()(const Value& /* v */) const { return true; }
	    };
	    
	    
      //////////////////////////////////////////////////////////////////////////
      /// Construct an empty B-tree using temporary files. The persistency flag is 
      /// set to \ref PERSIST_DELETE. The <em>params</em> object contains the
      /// user-definable parameters.
	  /// \sa btree_params.
	  //////////////////////////////////////////////////////////////////////////
	    btree(const btree_params &params = btree_params_default);
	    
      //////////////////////////////////////////////////////////////////////////
      /// Construct a B-tree from the given leaves and nodes. The files 
	    /// created/used by a Btree instance are outlined below:
	    /// \par .l.blk 
	    /// Contains the leaves block collection.
      /// \par .l.stk 
      /// Contains the free blocks stack for the leaves block collection.
      /// \par .n.blk 
      /// Contains the nodes block collection.
      /// \par .n.stk 
      /// Contains the free blocks stack for the leaves block collection.
	    /// The persistency flag is 
      /// set to \ref PERSIST_PERSISTENT. The <em>params</em> object contains the
      /// user-definable parameters.
      /// \sa btree_params, persist(), tpie::ami::collection_single< BTECOLL > 
      //////////////////////////////////////////////////////////////////////////

	  btree(const std::string& base_file_name,
		  collection_type type = WRITE_COLLECTION,
		  const btree_params &params = btree_params_default);

      //////////////////////////////////////////////////////////////////////////
	    /// As a convenience, this function sorts the stream <em>in_stream</em>
	    /// and stores the result in <em>out_stream</em>. If the value
	    /// of <em>out_stream</em> passed to the function is NULL, a new stream 
	    /// is created and os points to it. 
      /// \sa load().
      //////////////////////////////////////////////////////////////////////////
	    err sort(stream<Value>* in_stream, stream<Value>* &out_stream);


      //////////////////////////////////////////////////////////////////////////
	    /// Bulk load the tree from a sorted stream. Same as \ref load() but assumes
	    /// the input stream to be srted.
	    /// Leaves are filled to <em>leaf_fill</em> times capacity, and nodes are 
	    /// filled to <em>node_fill</em> times capacity.
      /// \sa load().
      //////////////////////////////////////////////////////////////////////////
	    err load_sorted(stream<Value>* stream_s, 
			    float leaf_fill = .75, float node_fill = .60);

      //////////////////////////////////////////////////////////////////////////
	    ///  Bulk load from given stream. 
	    /// Calls sort() and then load_sorted(). 
	    /// Leaves are filled to <em>leaf_fill</em> times capacity, and nodes 
	    /// are filled to	<em>node_fill</em> times capacity.
      //////////////////////////////////////////////////////////////////////////
	    err load(stream<Value>* s, 
		     float leaf_fill = .75, float node_fill = .60);


      //////////////////////////////////////////////////////////////////////////
	    /// Write all elements stored in the tree to the given stream, in sorted
	    /// order. No changes are performed on the tree.
      //////////////////////////////////////////////////////////////////////////
	    err unload(stream<Value>* s);


      //////////////////////////////////////////////////////////////////////////
	    /// Bulk load from another B-tree. This is a means of reoganizing a
	    /// B-tree after a lot of updates. A newly loaded structure may use less
	    /// space and may answer range queries faster. 
	    /// Leaves are filled to <em>leaf_fill</em> times capacity, and nodes are
	    /// filled to <em>node_fill</em> times capacity.
      //////////////////////////////////////////////////////////////////////////
	    err load(btree<Key, Value, Compare, KeyOfValue, BTECOLL>* bt,
		     float leaf_fill = .75, float node_fill = .60);


      //////////////////////////////////////////////////////////////////////////
	    /// Traverse the tree in depth-first-search preorder.
	    /// Returns a std::pair containing the next node to be visited and its 
	    /// level (root is on level 0). To initiate the process, the function 
	    /// should be called with -1 for <em>level</em>.
      //////////////////////////////////////////////////////////////////////////
	    std::pair<bid_t, Key> dfs_preorder(int& level);


      //////////////////////////////////////////////////////////////////////////
	    /// Insert the given element into the tree.
	    /// Returns <em>true</em> if the insertion succeeded, <em>false</em> 
	    /// otherwise (duplicate key).
	    //////////////////////////////////////////////////////////////////////////
	    bool insert(const Value& v);


      //////////////////////////////////////////////////////////////////////////
	    /// Modify a given element.
	    /// If the given element is not found in the tree, it is inserted.
	    /// Equivalent to erase() followed by insert(), but using a single search operation.
      //////////////////////////////////////////////////////////////////////////
	    bool modify(const Value& v);


      //////////////////////////////////////////////////////////////////////////
	    /// Delete the element with the given key from the tree.
	    /// If an element was found and deleted, the function returns <em>true</em>.
	    //////////////////////////////////////////////////////////////////////////
	    bool erase(const Key& k);


      //////////////////////////////////////////////////////////////////////////
	    /// Find an element based on the given key. 
	    /// If found, return <em>true</em> and store the element in <em>v</em>.
      //////////////////////////////////////////////////////////////////////////
	    bool find(const Key& k, Value& v);


      //////////////////////////////////////////////////////////////////////////
	    /// Find the highest element stored in the tree whose key is lower than 
	    /// the given key. If such an element exists, the function returns
	    /// <em>true</em> and stores the result in <em>v</em>. 
      /// @see succ()
      //////////////////////////////////////////////////////////////////////////
	    bool pred(const Key& k, Value& v);


      //////////////////////////////////////////////////////////////////////////
	    /// Find the lowest element stored in the tree whose key is higher than
	    /// the given key. If such an element exists, the function returns 
	    /// <em>true</em> and stores the result in <em>v</em>. 
	    /// @see pred()
	    //////////////////////////////////////////////////////////////////////////
	    bool succ(const Key& k, Value& v);


      //////////////////////////////////////////////////////////////////////////
	    /// Find all elements within the given range.
	    /// If <em>s</em> is not <em>NULL</em>, the elements found are stored
	    /// in the stream and the number of elements found is returned.
	    /// Otherwise, the results are not stored, only the count is returned.
      //////////////////////////////////////////////////////////////////////////
	    template<class Filter>
	    size_t range_query(const Key& k1, const Key& k2, 
			       stream<Value>* s, const Filter& filter_through)
		{
	      // This method is inlined such as to comply with MSVC++ "requirements".
  
		    Key kmin = comp_(k1, k2) ? k1: k2;
		    Key kmax = comp_(k1, k2) ? k2: k1;

		    // Find the leaf that might contain kmin.
		    bid_t bid = find_leaf(kmin);
		    btree_leaf<Key, Value, Compare, KeyOfValue, BTECOLL> *p = fetch_leaf(bid);
		    bool done = false;
		    size_t result = 0;

#if  BTREE_LEAF_ELEMENTS_SORTED

		    size_t j;
		    j = p->find(kmin);
		    while (bid != 0 && !done) {
			while (j < p->size() && !done) {
			    if (comp_(kov_(p->el[j]), kmax) || 
				(!comp_(kov_(p->el[j]), kmax) && !comp_(kmax, kov_(p->el[j])))) {
				if (filter_through(p->el[j])) {
				    if (s != NULL)
					s->write_item(p->el[j]);
				    result++;
				}
			    } else
				done = true;
			    j++;
			}
			bid = p->next();
			release_leaf(p);
			if (bid != 0 && !done)
			    p = fetch_leaf(bid);
			j = 0;
		    }

#else

		    size_t i;
		    // Check elements of p.
		    for (i = 0; i < p->size(); i++) {
			if (comp_(kov_(p->el[i]), kmax) && comp_(kmin, kov_(p->el[i])) ||
			    !comp_(kov_(p->el[i]), kmax) && !comp_(kmax, kov_(p->el[i])) ||
			    !comp_(kov_(p->el[i]), kmin) && !comp_(kmin, kov_(p->el[i]))) {
			    if (filter_through(p->el[i])) {
				if (s != NULL)
				    s->write_item(p->el[i]);
				result++;
			    }
			}
		    }
		    bid = p->next();
		    release_leaf(p);

		    if (bid != 0) {
			p = fetch_leaf(bid);
			bid_t pnbid = p->next();
			btree_leaf<Key, Value, Compare, KeyOfValue, BTECOLL>* pn;

			while (pnbid != 0 && !done) {
			    pn = fetch_leaf(pnbid);
			    if (comp_(kov_(pn->el[0]), kmax)) {
				// Write all elements from p to stream s.
				for (i = 0; i < p->size(); i++) {
				    if (filter_through(p->el[i])) {
					if (s!= NULL)
					    s->write_item(p->el[i]);
					result++;
				    }
				}
			    } else 
				done = true;

			    release_leaf(p);
			    p = pn;
			    pnbid = p->next();
			}

			// Check elements of p.
			for (i = 0; i < p->size(); i++) {
			    if (comp_(kov_(p->el[i]), kmax) ||
				(!comp_(kov_(p->el[i]), kmax) && !comp_(kmax, kov_(p->el[i])))) {
				if (filter_through(p->el[i])) {
				    if (s!= NULL)
					s->write_item(p->el[i]);
				    result++;
				}
			    }
			}
			release_leaf(p);
		    }
#endif

		    empty_stack();
		    return result;
		}


      //////////////////////////////////////////////////////////////////////////
	    /// Find all elements within the given range.
	    /// If <em>s</em> is not <em>NULL</em>, the elements found are stored
	    /// in the stream and the number of elements found is returned. 
	    /// Otherwise, the results are not stored, only the count is returned.
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET range_query(const Key& k1, const Key& k2, stream<Value>* s)
		{ return range_query(k1, k2, s, dummy_filter_t()); }


      //////////////////////////////////////////////////////////////////////////
	    /// Same as \ref range_query().
	    //////////////////////////////////////////////////////////////////////////
	    template<class Filter>
	    size_t window_query(const Key& k1, const Key& k2, 
				stream<Value>* s, const Filter& f) 
		{ return range_query(k1, k2, s, f); }


      //////////////////////////////////////////////////////////////////////////
      /// Same as \ref range_query().
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET window_query(const Key& k1, const Key& k2, stream<Value>* s)
		{ return range_query(k1, k2, s, dummy_filter_t()); }


      //////////////////////////////////////////////////////////////////////////
      /// Inquire the number of elements stored in the leaves of this tree.
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET size() const { return header_.size; }


      //////////////////////////////////////////////////////////////////////////
      /// Inquire the number of leaf nodes of this tree.
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET leaf_count() const { return pcoll_leaves_->size(); }


      //////////////////////////////////////////////////////////////////////////
      /// Inquire the number of internal (non-leaf) nodes of this tree.
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET node_count() const { return pcoll_nodes_->size(); }


      //////////////////////////////////////////////////////////////////////////
      /// Inquire the number of os-blocks used by the btree leaves. The 
	    /// value is calculated by the node count of the btree and the 
	    /// \ref leaf_block_factor and \ref node_block_factor, resp.
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET os_block_count() const { 
		return pcoll_leaves_->size() * params_.leaf_block_factor + 
		    pcoll_nodes_->size() * params_.node_block_factor; 
	    }


      //TODO Make this method protected or remove it!
      //////////////////////////////////////////////////////////////////////////
      /// Returns the block id of type \ref bid_t  of the root. 
      //////////////////////////////////////////////////////////////////////////
	    bid_t root_bid() const { return header_.root_bid; }

      //////////////////////////////////////////////////////////////////////////
	    /// Returns the height of the tree, including the leaf level.
	    /// A value of 0 represents an empty tree. A value of 1 represents a tree
	    /// with only one leaf node (which is also the root node).
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T height() const { return header_.height; }


      //////////////////////////////////////////////////////////////////////////
	    /// Set the \ref persistency flag of the B-tree. 
	    /// The persistency flag dictates the behavior of the destructor of
	    /// this btree instance. If <em>per</em> is <em>PERSIST_DELETE</em>, all 
	    /// files associated with the tree will be removed, and all the elements
	    /// stored in the tree will be lost after the destruction of this btree 
	    /// instance. If <em>per</em> is \ref PERSIST_PERSISTENT, all files 
	    /// associated with the tree will be closed during the destruction, and
	    /// all the information needed to reopen this tree will be saved.
      //////////////////////////////////////////////////////////////////////////
	    void persist(persistence per);


      //////////////////////////////////////////////////////////////////////////
	    /// Returns a const reference to the \ref btree_params object used by the
	    /// B-tree. This object contains the true values of all parameters
	    /// (unlike the object passed to the constructor, which may contain 
	    /// 0-valued parameters to indicate default behavior).
	    /// @see btree_params
	    //////////////////////////////////////////////////////////////////////////
	    const btree_params& params() const { return params_; }


      //////////////////////////////////////////////////////////////////////////
	    /// Returns the status of the collection. The result is either
	    /// \ref BTREE_STATUS_VALID or \ref BTREE_STATUS_INVALID. The only
	    /// operation that can leave the tree invalid is the constructor
	    /// (if that happens, the log file contains more information).
	    /// @see is_valid()
      //////////////////////////////////////////////////////////////////////////
	    btree_status status() const { return status_; }


      //////////////////////////////////////////////////////////////////////////
	    /// Returns <em>true</em> if the status of the tree is 
	    /// \ref BTREE_STATUS_VALID</em>, <em>false</em> otherwise.
	    /// @see status()
      //////////////////////////////////////////////////////////////////////////
	    bool is_valid() const { return status_ == BTREE_STATUS_VALID; }


      //////////////////////////////////////////////////////////////////////////
	    /// Returns an object containing the statistics of this B-tree.
	    ///The statistics refer to this B-tree instance only.
      /// @see gstats()
      //////////////////////////////////////////////////////////////////////////
	    const stats_tree &stats();

      //////////////////////////////////////////////////////////////////////////
	    /// Inquire the base path name.
	    /// This is the name of the B-tree, determined during construction.
	    /// @see btree()
      //////////////////////////////////////////////////////////////////////////
	    const std::string& name() const { return name_; }


      //////////////////////////////////////////////////////////////////////////
	    /// Close (and potentially destroy) this B-tree.
	    /// If the persistency flag is \ref PERSIST_DELETE, all files
	    /// associated with the tree will be removed.
	    /// @see persist()
      //////////////////////////////////////////////////////////////////////////
	    ~btree();

	protected:


      //////////////////////////////////////////////////////////////////////////
	    /// Function object for the node cache write out.
	    //////////////////////////////////////////////////////////////////////////
	    class remove_node {
	    public:
			void operator()(node_t* p) { tpie_delete(p); }
	    };

	    //////////////////////////////////////////////////////////////////////////
      /// Function object for the leaf cache write out.
      //////////////////////////////////////////////////////////////////////////
	    class remove_leaf { 
	    public:
			void operator()(leaf_t* p) { tpie_delete(p); }
	    };

	    typedef CACHE_MANAGER<node_t*, remove_node> node_cache_t;
	    typedef CACHE_MANAGER<leaf_t*, remove_leaf> leaf_cache_t;

      //////////////////////////////////////////////////////////////////////////
      /// Holds meta information about a btree.
      //////////////////////////////////////////////////////////////////////////
	    class header_t {
	    public:
		bid_t root_bid;
		TPIE_OS_SIZE_T height;
		TPIE_OS_OFFSET size;

		header_t(): root_bid(0), height(0), size(0) {}
	    };

	    /** Critical information: root bid, height, size (will be stored into
	    / the header of the nodes collection). */
	    header_t header_;

	    /** The node cache. */
	    node_cache_t* node_cache_;
	    /** The leaf cache. */
	    leaf_cache_t* leaf_cache_;

	    /** Run-time parameters. */
	    btree_params params_;

	    /** The collection storing the leaves. */
	    collection_t* pcoll_leaves_;

	    /** The collection storing the internal nodes (could be the same). */
	    collection_t* pcoll_nodes_;

	    /** Comparison object. */
	    Compare comp_;

      //////////////////////////////////////////////////////////////////////////
      /// Comparator class.
      //////////////////////////////////////////////////////////////////////////
	    class comp_for_sort {
		Compare comp_;
		KeyOfValue kov_;
	    public:
		int compare(const Value& v1, const Value& v2) {
		    return (comp_(kov_(v1), kov_(v2)) ? -1: 
			    (comp_(kov_(v2), kov_(v1)) ? 1: 0));
		}
	    }; 

	    /** The status. Set during construction. */
	    btree_status status_;

	    /** Stack to store the path to a leaf. */
	    std::stack<std::pair<bid_t,TPIE_OS_SIZE_T> >path_stack_;

	    /** Stack to store path during dfspreorder traversal. Each element is
	     * a std::pair: block id and link index. */
	    std::stack<std::pair<bid_t,TPIE_OS_SIZE_T> >dfs_stack_;

	    /** Statistics. */
	    stats_tree stats_;

	    /** Use this to obtain keys from Value elements. */
	    KeyOfValue kov_;

	    /** Base path name. */
	    std::string name_;

	    /** Helper function for inserting into a btree. */
	    bool insert_split(const Value& v, 
			      leaf_t* p, 
			      bid_t& leaf_id, bool loading = false);

	    /** Helper function for inserting into a btree. */
	    bool insert_empty(const Value& v);

	    /** Helper function for inserting into a btree. */
	    bool insert_load(const Value& v,   
			     leaf_t* &lcl);

	    /** Intialization routine shared by all constructors. */
	    void shared_init(const std::string& base_file_name, collection_type type);

	    /** Empty the path stack. */
	    void empty_stack() { while (!path_stack_.empty()) path_stack_.pop(); }

	    ///////////////////////////////////////////////////////////////////////////
      /// Find the leaf where an element with key k might be.  Return the
      /// bid of that leaf. The stack contains the path to that leaf (but
      /// not the leaf itself). Each item in the stack is a std::pair of a bid
      /// of a node and the position (in this node) of the link to the son
      /// that is next on the path to the leaf.
	    ///////////////////////////////////////////////////////////////////////////
	    bid_t find_leaf(const Key& k);

      ///////////////////////////////////////////////////////////////////////////
	    /// Returns the leaf with the minimum key element. Nothing is pushed
	    /// on the stack.
      ///////////////////////////////////////////////////////////////////////////
	    bid_t find_min_leaf();

      ///////////////////////////////////////////////////////////////////////////
	    /// Returns true if leaf p is underflow.
      ///////////////////////////////////////////////////////////////////////////
	    bool underflow_leaf(leaf_t *p) const;

      ///////////////////////////////////////////////////////////////////////////
	    /// Returns true if node p is underflow.
      ///////////////////////////////////////////////////////////////////////////
	    bool underflow_node(node_t *p) const;

      ///////////////////////////////////////////////////////////////////////////
	    /// Returns the underflow size of a leaf. This function does reside here and 
	    /// not in the btree_lead class for saving the space of the minimum fanout, a.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T cutoff_leaf(leaf_t *p) const;

      ///////////////////////////////////////////////////////////////////////////
	    /// Returns the underflow size of a node. his function does reside here and 
      /// not in the btree_lead class for saving the space of the minimum fanout, a.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T cutoff_node(node_t *p) const;

      ///////////////////////////////////////////////////////////////////////////
	    // Returns true if leaf p is full.
      ///////////////////////////////////////////////////////////////////////////
	    bool full_leaf(const leaf_t *p) const;

      ///////////////////////////////////////////////////////////////////////////
      // Returns true if node p is full.
      ///////////////////////////////////////////////////////////////////////////
	    bool full_node(const node_t *p) const;

      ///////////////////////////////////////////////////////////////////////////
	    /// Tries to balance p (when underflow) by borrowing one element from 
	    /// a sibling. f is the father of p and pos is the 
	    /// position of the link to p in f.
	    /// Returns false if unsuccessful.
      ///////////////////////////////////////////////////////////////////////////
	    bool balance_leaf(node_t *f, 
			      leaf_t *p, size_t pos);

      ///////////////////////////////////////////////////////////////////////////
	    // Same as balance_leaf(), but p is a node.
      ///////////////////////////////////////////////////////////////////////////
	    bool balance_node(node_t *f, 
			      node_t *p, size_t pos);

      ///////////////////////////////////////////////////////////////////////////
	    /// Merges p with a sibling; to call when balancing fails.
	    /// f is the father of p and pos is the position of the link to p in f.
      ///////////////////////////////////////////////////////////////////////////
	    void merge_leaf(node_t *f, 
			    leaf_t* &p, size_t pos);

      ///////////////////////////////////////////////////////////////////////////
      /// Same as merge_leaf(), but p is a node.
	    /// Merges p with a sibling; called when balancing fails.
      /// f is the father of p and pos is the position of the link to p in f.
      ///////////////////////////////////////////////////////////////////////////
	    void merge_node(node_t *f, 
			    node_t* &p, size_t pos);

	public:
	    ///////////////////////////////////////////////////////////////////////////
	    /// Fetch a node by its block id.
	    ///////////////////////////////////////////////////////////////////////////
	    node_t* fetch_node(bid_t bid = 0);

	    ///////////////////////////////////////////////////////////////////////////
      /// Fetch a leaf by its block id.
      ///////////////////////////////////////////////////////////////////////////
	    leaf_t* fetch_leaf(bid_t bid = 0);

      ///////////////////////////////////////////////////////////////////////////
      /// Writes leaf p back to disk if p's persistence  is
	    /// \ref PERSIST_PERSISTENT, and just deletes p otherwise
      ///////////////////////////////////////////////////////////////////////////
	    void release_leaf(leaf_t* p);

	    ///////////////////////////////////////////////////////////////////////////
      /// Writes node p back to disk if p' persistence is 
	    /// \ref PERSIST_PERSISTENT, and just 
      /// deletes p otherwise
      ///////////////////////////////////////////////////////////////////////////
	    void release_node(node_t* p);
	};




// Define shortcuts. They are undefined at the end of the file.
#define BTREE_NODE btree_node<Key, Value, Compare, KeyOfValue, BTECOLL>
#define BTREE_LEAF btree_leaf<Key, Value, Compare, KeyOfValue, BTECOLL>
#define BTREE      btree<Key, Value, Compare, KeyOfValue, BTECOLL>



	///////////////////////////////////////////////////////////////////////////
	/// Holding metainformation about a btree_leaf.
  ///////////////////////////////////////////////////////////////////////////
	struct _btree_leaf_info {
	    TPIE_OS_SIZE_T size;
	    bid_t prev;
	    bid_t next;
	};

  //////////////////////////////////////////////////////////////////////////
	/// The btree_leaf class for storing \ref size() elements of type Value 
	/// in a \ref btree.
  //////////////////////////////////////////////////////////////////////////
	template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL = tpie::bte::COLLECTION >
	class btree_leaf: public block<Value, _btree_leaf_info, BTECOLL> {

	    Compare comp_;

      ///////////////////////////////////////////////////////////////////////////
	    /// This allows comparison between 
	    /// Values and Keys for STL's lower_bound().
      ///////////////////////////////////////////////////////////////////////////
	    struct Compare_value_key { 
		bool operator()(const Value& v, const Key& k) const { 
		    return Compare()(KeyOfValue()(v), k); 
		}
		bool operator()(const Value& v1, const Value& v2) const { 
		    return Compare()(KeyOfValue()(v1), KeyOfValue()(v2)); 
		}
	    };

	    ///////////////////////////////////////////////////////////////////////////
      /// This allows comparison between 
      /// Values and Keys for STL's lower_bound().
      ///////////////////////////////////////////////////////////////////////////
	    struct Compare_value_value { 
		bool operator()(const Value& v1, const Value& v2) const { 
		    return Compare()(KeyOfValue()(v1), KeyOfValue()(v2)); 
		}
	    };
	    Compare_value_key comp_value_key_;
	    Compare_value_value comp_value_value_;
  
	public:
	    using block<Value, _btree_leaf_info, BTECOLL>::info;
	    using block<Value, _btree_leaf_info, BTECOLL>::el;
	    using block<Value, _btree_leaf_info, BTECOLL>::dirty;
  
      ///////////////////////////////////////////////////////////////////////////
	    /// Compute the capacity of the el vector STATICALLY (but you have to
	    /// give it the correct logical block size!).
      ///////////////////////////////////////////////////////////////////////////
	    static TPIE_OS_SIZE_T el_capacity(size_t block_size);

      ///////////////////////////////////////////////////////////////////////////
	    /// Find and return the position of key k 
	    /// (ie, the lowest position where it would be inserted).
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T find(const Key& k);

      ///////////////////////////////////////////////////////////////////////////
	    /// Retrieves the predecessor of k.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T pred(const Key& k);
    
      ///////////////////////////////////////////////////////////////////////////
      /// Retrieves the Successor of k.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T succ(const Key& k);

      ///////////////////////////////////////////////////////////////////////////
      /// Construcor.
      ///////////////////////////////////////////////////////////////////////////
	    btree_leaf(collection_single<BTECOLL>* pcoll, bid_t bid = 0);

      ///////////////////////////////////////////////////////////////////////////
      /// Returns the number of elements stored in this leaf.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T & size() { return info()->size; }

      ///////////////////////////////////////////////////////////////////////////
      /// Returns the number of elements stored in this leaf as a const reference.
      ///////////////////////////////////////////////////////////////////////////
	    const TPIE_OS_SIZE_T & size() const { return info()->size; }

      ///////////////////////////////////////////////////////////////////////////
      /// Returns maximum number of elements that can be stored in this leaf.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T capacity() const { return el.capacity(); }

      ///////////////////////////////////////////////////////////////////////////
      /// Retrieves the block_id of the previous leaf in the btree. Calls btree_leaf_info#prev()
      ///////////////////////////////////////////////////////////////////////////
	    bid_t& prev() { return info()->prev; }

      ///////////////////////////////////////////////////////////////////////////
      /// Retrieves the block_id of the previous leaf in the btree. Calls btree_leaf_info#prev()
      ///////////////////////////////////////////////////////////////////////////
	    const bid_t& prev() const { return info()->prev; }

      ///////////////////////////////////////////////////////////////////////////
      /// Retrieves the block_id of the next leaf in the btree. Calls btree_leaf_info#next()
      ///////////////////////////////////////////////////////////////////////////
	    bid_t& next() { return info()->next; }

      ///////////////////////////////////////////////////////////////////////////
      /// Retrieves the block_id of the next leaf in the btree. Calls btree_leaf_info#next()
      ///////////////////////////////////////////////////////////////////////////
	    const bid_t& next() const { return info()->next; }

      ///////////////////////////////////////////////////////////////////////////
      /// Returns true if the leaf's capacity is reached.
      ///////////////////////////////////////////////////////////////////////////
	    bool full() const { return size() == capacity(); }

      ///////////////////////////////////////////////////////////////////////////
      /// Returns true if the leaf is empty.
      ///////////////////////////////////////////////////////////////////////////
	    bool empty() const { return size() == 0; }

      ///////////////////////////////////////////////////////////////////////////
	    /// Split into two leaves containing the same number of elements.
	    /// Return the median key (i.e., the key of the last element stored 
	    /// in this leaf, after split).
      ///////////////////////////////////////////////////////////////////////////
	    Key split(BTREE_LEAF &right);

      ///////////////////////////////////////////////////////////////////////////
	    /// Merge this leaf with a leaf given.
      ///////////////////////////////////////////////////////////////////////////
	    void merge(const BTREE_LEAF &right);

      ///////////////////////////////////////////////////////////////////////////
	    /// Insert a data element. The leaf should NOT be full.
	    /// Return false if the key is already in the tree.
      ///////////////////////////////////////////////////////////////////////////
	    bool insert(const Value& v);

      ///////////////////////////////////////////////////////////////////////////
	    /// Insert element into position pos.
      ///////////////////////////////////////////////////////////////////////////
	    void insert_pos(const Value& v, TPIE_OS_SIZE_T pos);

      ///////////////////////////////////////////////////////////////////////////
	    /// Delete an element given by its key. The leaf should NOT be empty.
	    /// Return false if the key is not found in the tree.
      ///////////////////////////////////////////////////////////////////////////
	    bool erase(const Key& k);

      ///////////////////////////////////////////////////////////////////////////
	    /// Erase element from position pos.
      ///////////////////////////////////////////////////////////////////////////
	    void erase_pos(size_t pos);

      ///////////////////////////////////////////////////////////////////////////
	    /// Sort elements.
      ///////////////////////////////////////////////////////////////////////////
	    void sort();

      ///////////////////////////////////////////////////////////////////////////
	    /// Destructor.
      ///////////////////////////////////////////////////////////////////////////
	    ~btree_leaf();
	};

  //////////////////////////////////////////////////////////////////////////
	/// The btree_node class for representing an internal node of a \ref  btree.
	/// It stores size() keys and size()+1 links representing 
	/// the following pattern: Link0 Key0 Link1 Key1 ... LinkS KeyS Link(S+1)
  //////////////////////////////////////////////////////////////////////////
	template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL = tpie::bte::COLLECTION >
	class btree_node: public block<Key, size_t, BTECOLL> {

	    Compare comp_;
  
	public:
	    using block<Key, size_t, BTECOLL>::info;
	    using block<Key, size_t, BTECOLL>::el;
	    using block<Key, size_t, BTECOLL>::lk;
	    using block<Key, size_t, BTECOLL>::dirty;
  
      ///////////////////////////////////////////////////////////////////////////
	    /// Compute the capacity of the lk vector STATICALLY (but you have to
	    /// give it the correct logical block size!).
      ///////////////////////////////////////////////////////////////////////////
	    static size_t lk_capacity(size_t block_size);

      ///////////////////////////////////////////////////////////////////////////
	    /// Compute the capacity of the el vector STATICALLY.
      ///////////////////////////////////////////////////////////////////////////
	    static TPIE_OS_SIZE_T el_capacity(size_t block_size);

      ///////////////////////////////////////////////////////////////////////////
	    /// Find and return the position of key k 
	    /// (ie, the lowest position in the array of keys where it would be inserted).
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T find(const Key& k);

      ///////////////////////////////////////////////////////////////////////////
	    /// Constructor. Calls the block constructor with the 
	    /// appropriate number of links.
      ///////////////////////////////////////////////////////////////////////////
	    btree_node(collection_single<BTECOLL>* pcoll, bid_t bid = 0);

      ///////////////////////////////////////////////////////////////////////////
	    /// Number of keys stored in this node.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T& size() { return (TPIE_OS_SIZE_T&) (*info()); }
	    const TPIE_OS_SIZE_T& size() const { return (TPIE_OS_SIZE_T&) (*info()); }

      ///////////////////////////////////////////////////////////////////////////
	    /// Maximum number of keys that can be stored in this node.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T capacity() const { return el.capacity(); }

      ///////////////////////////////////////////////////////////////////////////
      /// Returns true if the node's capacity is reached.
      ///////////////////////////////////////////////////////////////////////////
	    bool full() const { return size() == capacity(); }

      ///////////////////////////////////////////////////////////////////////////
      /// Returns true if the leaf is empty.
      ///////////////////////////////////////////////////////////////////////////
	    bool empty() const { return size() == 0; }

      ///////////////////////////////////////////////////////////////////////////
      /// Split into two leafes containing the same number of elements.
      /// Return the median key (i.e., the key of the last element stored 
      /// in this leaf, after split).
      ///////////////////////////////////////////////////////////////////////////
	    Key split(BTREE_NODE &right);

      ///////////////////////////////////////////////////////////////////////////
      /// Merge this node with a node given.
      ///////////////////////////////////////////////////////////////////////////
	    void merge(const BTREE_NODE &right, const Key& k);

      ///////////////////////////////////////////////////////////////////////////
	    /// Insert a key and link into a non-full node in a given position.
	    /// No validity checks.
      ///////////////////////////////////////////////////////////////////////////
	    void insert_pos(const Key& k, bid_t l, TPIE_OS_SIZE_T k_pos, TPIE_OS_SIZE_T l_pos);

      ///////////////////////////////////////////////////////////////////////////
	    /// Insert a key and link into a non-full node 
	    /// (uses the key k to find the right position).
      ///////////////////////////////////////////////////////////////////////////
	    void insert(const Key& k, bid_t l);

      ///////////////////////////////////////////////////////////////////////////
	    /// Delete an element given by its key.
      ///////////////////////////////////////////////////////////////////////////
	    void erase_pos(size_t k_pos, size_t l_pos);

      ///////////////////////////////////////////////////////////////////////////
	    /// Desctructor.
	    ///////////////////////////////////////////////////////////////////////////
	    ~btree_node();
	};


// --------------------***Implementation*** ---------------------------------


// --------------------***btree_leaf*** ---------------------------------

			  template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
			  TPIE_OS_SIZE_T BTREE_LEAF::el_capacity(TPIE_OS_SIZE_T block_size) {
			      return block<Value, _btree_leaf_info, BTECOLL>::el_capacity(block_size, 0);
			  }

/// *btree_leaf::btree_leaf* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
BTREE_LEAF::btree_leaf(collection_single<BTECOLL>* pcoll, bid_t lbid)
    : block<Value, _btree_leaf_info, BTECOLL>(pcoll, 0, lbid) {
    if (lbid == 0) {
	size() = 0;
	next() = 0;
#if BTREE_LEAF_PREV_POINTER
	prev() = 0;
#endif
    }
}

// --------------------***btree_leaf::split*** ---------------------------------
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
Key  BTREE_LEAF::split(BTREE_LEAF &right) {

#if (!BTREE_LEAF_ELEMENTS_SORTED)
    sort();
#endif

    // save the original size of this leaf.
    TPIE_OS_SIZE_T original_size = size();

    // The new leaf will have half of this leaf's elements.
    // If the original size is odd, the new leaf will have fewer elements.
    right.size() = original_size / 2;

    // Update this leaf's size.
    size() = original_size - right.size();

    // Copy the elements of the new leaf from the end 
    // of this leaf's array of elements.
    right.el.copy(0, right.size(), el, size());

    dirty() = 1;
    right.dirty() = 1;
  
    // Return the key of the last element from this leaf.
    return KeyOfValue()(el[size() - 1]);
}

/// *btree_leaf::merge* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void BTREE_LEAF::merge(const BTREE_LEAF &right) {

    // Make sure there's enough place.
    assert(size() + right.size() <= capacity());

#if BTREE_LEAF_ELEMENTS_SORTED
    assert(comp_(KeyOfValue()(el[size() - 1]), KeyOfValue()(right.el[0])));
#endif

    // save the original size of this leaf.
    TPIE_OS_SIZE_T original_size = size();
   
    // Update this leaf's size.
    size() = original_size + right.size();

    // Copy the elements of the right leaf to the end 
    // of this leaf's array of elements.
    el.copy(original_size, right.size(), right.el, 0);

    next() = right.next();
    dirty() = 1;  
}

/// *btree_leaf::insert_pos* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
inline void BTREE_LEAF::insert_pos(const Value& v, size_t pos) {

    // Insert mechanics.
    if (pos == size())
	el[pos] = v;
    else
	el.insert(v, pos);

    // Increase size by one and update the dirty bit.
    size()++;
    dirty() = 1;
}

/// *btree_leaf::insert* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
inline bool BTREE_LEAF::insert(const Value& v) {

#if (BTREE_LEAF_ELEMENTS_SORTED || BTREE_UNIQUE_KEYS)
    // Find the position where v should be.
    size_t pos;
    if (size() == 0)
	pos = 0;
    else if (comp_(KeyOfValue()(el[size()-1]), KeyOfValue()(v)))
	pos = size();
    else
	pos = find(KeyOfValue()(v));
#endif

#if BTREE_UNIQUE_KEYS
    // Check for duplicate key.
    if (pos < size())
	if (!comp_(KeyOfValue()(v), KeyOfValue()(el[pos])) && 
	    !comp_(KeyOfValue()(el[pos]), KeyOfValue()(v))) {
	    TP_LOG_WARNING_ID("Attempting to insert duplicate key. Ignoring insert.");
	    return false;
	}
#endif

#if BTREE_LEAF_ELEMENTS_SORTED
    insert_pos(v, pos);
#else
    insert_pos(v, size());
#endif

    return true;
}

/// *btree_leaf::find* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
size_t BTREE_LEAF::find(const Key& k) {
#if BTREE_LEAF_ELEMENTS_SORTED
    // Sanity check.
    assert(size() < 2 || comp_(KeyOfValue()(el[0]), KeyOfValue()(el[size()-1])));
    return std::lower_bound(&el[0], &el[size()-1] + 1, k, comp_value_key_) - &el[0];
#else
    size_t i;
    for (i = 0u; i < size(); i++)
	if (!comp_(KeyOfValue()(el[i]), k) && !comp_(k, KeyOfValue()(el[i])))
	    return i;
    return size();
#endif
}

/// *btree_leaf::pred* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
size_t BTREE_LEAF::pred(const Key& k) {

    size_t pred_idx;
#if BTREE_LEAF_ELEMENTS_SORTED
    // Sanity check.
    assert(size() < 2 || comp_(KeyOfValue()(el[0]), KeyOfValue()(el[size()-1])));
    pred_idx = std::lower_bound(&el[0], &el[size()-1] + 1, k,comp_value_key_) - &el[0];
    // lower_bound pos is off by one for pred
    // Final pred_idx cannot be matching key
    if (pred_idx != 0)
	pred_idx--;
    return pred_idx;
#else
    size_t i=0;
    size_t j;
    // Find candidate
    while (i < size() && !comp_(KeyOfValue()(el[i]), k) )
	i++;
    pred_idx = i;
    // Check for closer candidates
    for (j = i+1; j < size(); j++)
	if (comp_(KeyOfValue()(el[j]), k) && comp_(KeyOfValue()(el[i]), KeyOfValue()(el[j])))
	    pred_idx = j;

    return ((i != size()) ? pred_idx: 0);
#endif
}

/// *btree_leaf::succ* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
size_t BTREE_LEAF::succ(const Key& k) {

    size_t succ_idx;
#if BTREE_LEAF_ELEMENTS_SORTED
    // Sanity check.
    assert(size() < 2 || comp_(KeyOfValue()(el[0]), KeyOfValue()(el[size()-1])));
    succ_idx = lower_bound(&el[0], &el[size()-1] + 1, k, comp_value_key_) - &el[0];
    // Bump up one spot if keys match
    if (succ_idx != size() &&
	!comp_(k,KeyOfValue()(el[succ_idx])) && !comp_(KeyOfValue()(el[succ_idx]),k) )
	succ_idx++;
    return succ_idx;
#else
    size_t i=0;
    size_t j;

    // Find candidate
    while (i < size() && !comp_(k, KeyOfValue()(el[i])) )
	i++;
    succ_idx = i;
    // Check for closer candidates
    for (j = i+1; j < size(); j++)
	if (comp_(k, KeyOfValue()(el[j])) && comp_(KeyOfValue()(el[j]), KeyOfValue()(el[i])))
	    succ_idx = j;

    return ((i != size()) ? succ_idx: 0);
#endif
}

/// *btree_leaf::erase* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool BTREE_LEAF::erase(const Key& k) {

    // Sanity check.
    assert(!empty());

    // Find the position where k should be.
    size_t pos = find(k);

    // Make sure we found an exact match.
    if (pos == size())
	return false;
    // TODO: make sure this is right.
    if (comp_(KeyOfValue()(el[pos]), k) || comp_(k, KeyOfValue()(el[pos])))
	return false;
  
    erase_pos(pos);

    return true;
}

/// *btree_leaf::erase_pos* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void BTREE_LEAF::erase_pos(TPIE_OS_SIZE_T pos) {
 
    // Erase mechanics.
    el.erase(pos);
  
    // Decrease size by one and update dirty bit.
    size()--;
    dirty() = 1;
}

/// *btree_leaf::sort* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void BTREE_LEAF::sort() {
    sort(&el[0], &el[size()-1] + 1, comp_value_value_);
}

/// *btree_leaf::~btree_leaf* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
BTREE_LEAF::~btree_leaf() {
    // TODO: is there anything to do here?
}


//////////////////////
////// **btree_node** //////
//////////////////////

	template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	size_t BTREE_NODE::lk_capacity(size_t block_size) {
	    return (size_t) ((block_size - sizeof(size_t) - sizeof(bid_t)) /
			     (sizeof(Key) + sizeof(bid_t)) + 1);
	}

	template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	TPIE_OS_SIZE_T BTREE_NODE::el_capacity(TPIE_OS_SIZE_T block_size) {
	    // Sanity check. Two different methods of computing the el capacity.
	    // [tavi 01/26/02]: Changed == into >= since I could fit one more
	    // element, but not one more link.
	    assert((block<Key, TPIE_OS_SIZE_T>::el_capacity(block_size, lk_capacity(block_size))) >= (TPIE_OS_SIZE_T) (lk_capacity(block_size) - 1));
	    return (TPIE_OS_SIZE_T) (lk_capacity(block_size) - 1);
	}

/// *btree_node::btree_node* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
BTREE_NODE::btree_node(collection_single<BTECOLL>* pcoll, bid_t nbid): 
    block<Key, size_t, BTECOLL>(pcoll, lk_capacity(pcoll->block_size()), nbid) {
    if (nbid == 0)
	size() = 0;
}


/// *btree_node::split* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
Key BTREE_NODE::split(BTREE_NODE &right) {

    // TODO: Is this needed? I want to be left with at least one key in each
    // node.
    assert(size() >= 3);

    // save the original size of this node.
    size_t original_size = size();

    // The new node will have half of this node's keys and half of its links.
    right.size() = original_size / 2;

    // Update this node's size (subtract one to account for the key 
    // that is going up the tree).
    size() = original_size - right.size() - 1;

    // Copy the keys of the new node from the end of this node's array of keys.
    //memcpy(right.elem(0), elem(size()+1), right.size() * sizeof(Key));
    right.el.copy(0, right.size(), el, size() + 1);

    // Copy the links of the new node from the end of this node's array of links.
    //memcpy(right.link(0), link(size()+1), (right.size()+1) * sizeof(bid_t));
    right.lk.copy(0, right.size() + 1, lk, size() + 1);

    dirty() = 1;
    right.dirty() = 1;

    // Return a copy of the key past the last key (no longer stored here).
    return el[size()];
}

/// *btree_node::insert_pos* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void BTREE_NODE::insert_pos(const Key& k, bid_t l, size_t k_pos, size_t l_pos) {

    assert(!full());

    // Insert mechanics.
    if (k_pos == size())
	el[k_pos] = k;
    else
	el.insert(k, k_pos);

    if (l_pos == size() + 1)
	lk[l_pos] = l;
    else
	lk.insert(l, l_pos);

    // Update size and dirty bit.
    size()++;
    dirty() = 1;
}


/// *btree_node::insert* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void BTREE_NODE::insert(const Key& k, bid_t l) {

    // Find the position using STL's binary search.
    size_t pos = std::lower_bound(&el[0], &el[size()-1] + 1, k, comp_) - &el[0];

    // Insert.
    insert_pos(k, l, pos, pos + 1);
}

/// *btree_node::erase_pos* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void BTREE_NODE::erase_pos(TPIE_OS_SIZE_T k_pos, TPIE_OS_SIZE_T l_pos) {

    assert(!empty());

    // Erase mechanics.
    el.erase(k_pos);
    lk.erase(l_pos);

    // Update the size and dirty bit.
    size()--;
    dirty() = 1;
}


/// *btree_node::merge* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void BTREE_NODE::merge(const BTREE_NODE &right, const Key& k) {

    // Make sure there's enough place.
    assert(size() + right.size() + 1 <= capacity());

    // save the original size of this leaf.
    size_t original_size = size();
   
    // Update this leaf's size. We add one to account for the key 
    // that's added in-between.
    size() = original_size + right.size() + 1;

    // Copy the elements of the right leaf to the end of this leaf's
    // array of elements.
    el[original_size] = k;
    el.copy(original_size + 1, right.size(), right.el, 0);

    // Copy the links also.
    lk.copy(original_size + 1, right.size() + 1, right.lk, 0);

    dirty() = 1;
}

/// *btree_node::find* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
size_t BTREE_NODE::find(const Key& k) {
    return (size() == 0) ? 0: (std::lower_bound(&el[0], &el[size()-1] + 1, k, comp_) - &el[0]);
}

/// *btree_node::~btree_node* ///
template<class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
BTREE_NODE::~btree_node() {
    // TODO: is there anything to do here?
}

/////////////////////
////// **btree** //////
/////////////////////


/// *btree::btree* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
btree<Key, Value, Compare, KeyOfValue, BTECOLL>::btree(const btree_params &params): header_(), params_(params), 
										    status_(BTREE_STATUS_VALID) {


    name_ = std::string(tempname::tpie_name("btree"));
    shared_init(name_, WRITE_COLLECTION);
    if (status_ == BTREE_STATUS_VALID) {
	persist(PERSIST_DELETE);
    }
}

/// *btree::btree* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
btree<Key, Value, Compare, KeyOfValue, BTECOLL>::btree(const std::string& base_file_name, collection_type type, 
						       const btree_params &params):
    header_(), params_(params), status_(BTREE_STATUS_VALID), stats_(), kov_(), name_(base_file_name) {

    shared_init(base_file_name, type);

    if (status_ == BTREE_STATUS_VALID) {
	if (pcoll_leaves_->size() > 0) {
	    // Read root bid, height and size from header.
	    header_ = *((header_t *) pcoll_nodes_->user_data());
	    // TODO: sanity checks.
	}
	persist(PERSIST_PERSISTENT);
    }
}

/// *btree::shared_init* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void btree<Key, Value, Compare, KeyOfValue, BTECOLL>::shared_init(const std::string& base_file_name, collection_type type) 
{
    if (base_file_name.empty()) {
	status_ = BTREE_STATUS_INVALID;
	TP_LOG_WARNING_ID("btree::btree: NULL file name.");
	return;
    }

    std::string lcollname = base_file_name + ".l";
    std::string ncollname = base_file_name + ".n";

    // Initialize these pointers to NULL to avoid errors in the
    // destructor in case of premature return from this function.
    node_cache_ = NULL;
    leaf_cache_ = NULL;
    pcoll_leaves_ = NULL;
    pcoll_nodes_ = NULL;

    pcoll_leaves_ = tpie_new<collection_t>(lcollname, type, params_.leaf_block_factor);
    if (!pcoll_leaves_->is_valid()) {
	status_ = BTREE_STATUS_INVALID;
	TP_LOG_WARNING_ID("btree::btree: Could not open leaves collection.");
	return;
    }

    pcoll_nodes_ = tpie_new<collection_t>(ncollname, type, params_.node_block_factor);
    if (!pcoll_nodes_->is_valid()) {
	status_ = BTREE_STATUS_INVALID;
	TP_LOG_WARNING_ID("btree::btree: Could not open nodes collection.");
	return;
    }    

    // Initialize the caches (associativity = 8).
    node_cache_ = tpie_new<node_cache_t>(params_.node_cache_size, 8);
    leaf_cache_ = tpie_new<leaf_cache_t>(params_.leaf_cache_size, 8);

    // Give meaningful values to parameters, if necessary.
    size_t leaf_capacity = BTREE_LEAF::el_capacity(pcoll_leaves_->block_size());
    if (params_.leaf_size_max == 0 || params_.leaf_size_max > leaf_capacity)
	params_.leaf_size_max = leaf_capacity;
    if (params_.leaf_size_max == 1)
	params_.leaf_size_max = 2;

    if (params_.leaf_size_min == 0)
	params_.leaf_size_min = params_.leaf_size_max / 2;

    size_t node_capacity = BTREE_NODE::el_capacity(pcoll_nodes_->block_size());
    if (params_.node_size_max == 0 || params_.node_size_max > node_capacity)
	params_.node_size_max = node_capacity;
    if (params_.node_size_max == 1 || params_.node_size_max == 2)
	params_.node_size_max = 3;

    if (params_.node_size_min == 0)
	params_.node_size_min = params_.node_size_max / 2;

    // Set the right block factor parameters for the case of an existing tree.
    params_.leaf_block_factor = pcoll_leaves_->block_factor();
    params_.node_block_factor = pcoll_nodes_->block_factor();
}


/// *btree::sort* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
err btree<Key, Value, Compare, KeyOfValue, BTECOLL>::sort(stream<Value>* in_stream, stream<Value>* &out_stream) {

    if (status_ != BTREE_STATUS_VALID) {
	TP_LOG_FATAL_ID("sort: tree is invalid.");
	return GENERIC_ERROR;
    }
    if (in_stream == NULL) {
	TP_LOG_FATAL_ID("sort: attempting to sort a NULL stream pointer.");
	return GENERIC_ERROR;
    }  
    if (in_stream->stream_len() == 0) {
	TP_LOG_FATAL_ID("sort: attempting to sort an empty stream.");
	return GENERIC_ERROR;
    }

    err retval;
    comp_for_sort cmp;
  
    if (out_stream == NULL) {
	out_stream = new stream<Value>;
	if (!out_stream->is_valid()) {
	    TP_LOG_FATAL_ID("sort: error initializing temporary stream.");
	    tpie_delete(out_stream);
	    return OBJECT_INITIALIZATION;
	}
	out_stream->persist(PERSIST_DELETE);
    }

    retval = tpie::ami::sort<Value,comp_for_sort>(in_stream, out_stream, &cmp);

    if (retval != NO_ERROR)
	TP_LOG_WARNING_ID("sort: sorting returned error.");
    else  if (in_stream->stream_len() != out_stream->stream_len()) {
	TP_LOG_WARNING_ID("sort: sorted stream has different length than unsorted stream.");
	retval = GENERIC_ERROR;
    }

    return retval;
}

/// *btree::load_sorted* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
err btree<Key, Value, Compare, KeyOfValue, BTECOLL>::load_sorted(stream<Value>* s, float leaf_fill, float node_fill) {

    if (status_ != BTREE_STATUS_VALID) {
	TP_LOG_FATAL_ID("load: tree is invalid.");
	return GENERIC_ERROR;
    }
    if (s == NULL) {
	TP_LOG_FATAL_ID("load: attempting to load with NULL stream pointer.");
	return GENERIC_ERROR;
    }
    if (!s->is_valid()) {
	TP_LOG_FATAL_ID("load: attempting to load with invalid input stream.");
	return GENERIC_ERROR;
    }

    Value* pv;
    err retval = NO_ERROR;
    btree_params params_saved = params_;
    params_.leaf_size_max = std::min(params_.leaf_size_max, size_t(leaf_fill*params_.leaf_size_max));
    params_.node_size_max = std::min(params_.node_size_max, size_t(node_fill*params_.node_size_max));

    BTREE_LEAF* lcl = NULL; // locally cached leaf.

    retval = s->seek(0);
    assert(retval == NO_ERROR);

    // Repeatedly insert items in sorted order.
    while ((retval = s->read_item(&pv)) == NO_ERROR) {
	insert_load(*pv, lcl);
    }

    if (retval != END_OF_STREAM)
	TP_LOG_FATAL_ID("load: error occured while reading the input stream.");
    else
	retval = NO_ERROR;

    if (lcl != NULL)
	release_leaf(lcl);
    params_ = params_saved;

    return retval;
}

/// *btree::load* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
err btree<Key, Value, Compare, KeyOfValue, BTECOLL>::load(stream<Value>* s, float leaf_fill, float node_fill) {

    err retval;
    stream<Value>* stream_s = tpie_new<stream<Value> >();

    retval = sort(s, stream_s);

    if (retval != NO_ERROR)
	return retval;

    retval = load_sorted(stream_s, leaf_fill, node_fill);

    tpie_delete(stream_s);
    return retval;
}

/// *btree::unload* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
err btree<Key, Value, Compare, KeyOfValue, BTECOLL>::unload(stream<Value>* s) {

    if (status_ != BTREE_STATUS_VALID) {
	TP_LOG_WARNING_ID("unload: tree is invalid. unload aborted.");
	return GENERIC_ERROR;
    }
    if (s == NULL) {
	TP_LOG_WARNING_ID("unload: NULL stream pointer. unload aborted.");
	return GENERIC_ERROR;
    }

    bid_t lbid = find_min_leaf();
    BTREE_LEAF* l;
    err retval = NO_ERROR;
    size_t i;

    tp_assert(lbid != 0, "");
  
    while (lbid != 0) {
	l = fetch_leaf(lbid);
	for (i = 0; i < l->size(); i++)
	    s->write_item(l->el[i]);
	lbid = l->next();
	release_leaf(l);
    }
    return retval;
}

/// *btree::load* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
err btree<Key, Value, Compare, KeyOfValue, BTECOLL>::load(BTREE* bt, float leaf_fill, float node_fill) {

    if (!is_valid()) {
	TP_LOG_WARNING_ID("load: tree is invalid.");
	return GENERIC_ERROR;
    }
    if (bt == NULL) {
	TP_LOG_WARNING_ID("load: NULL btree pointer.");
	return GENERIC_ERROR;
    }
    if (!bt->is_valid()) {
	TP_LOG_WARNING_ID("load: input tree is invalid.");
	return GENERIC_ERROR;
    }

    btree_params params_saved = params_;
    err retval = NO_ERROR;
    params_.leaf_size_max = std::min(params_.leaf_size_max, size_t(leaf_fill*params_.leaf_size_max));
    params_.node_size_max = std::min(params_.leaf_size_max, size_t(node_fill*params_.node_size_max));
    BTREE_LEAF* lcl = NULL; // locally cached leaf.

    // Get the bid of the min leaf in bt.
    bid_t lbid = bt->find_min_leaf();
    // Pointer to a leaf in bt.
    BTREE_LEAF* btl;
    size_t i;

    tp_assert(lbid != 0, "");

    // Iterate over all leaves of bt.
    while (lbid != 0) {
	btl = bt->fetch_leaf(lbid);

	for (i = 0; i < btl->size(); i++) {
	    insert_load(btl->el[i], lcl);
	}

	// Get next leaf in bt.
	lbid = btl->next();

	bt->release_leaf(btl);
    }

    if (lcl != NULL)
	release_leaf(lcl);
    params_ = params_saved;
    return retval;
}


/// *btree::dfs_preorder* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
std::pair<bid_t, Key> btree<Key, Value, Compare, KeyOfValue, BTECOLL>::dfs_preorder(int& level) {

    Key k;
    if (level == -1) {
	// Empty the stack. This allows restarts in the middle of a
	// traversal. All previous state information is lost.
	while (!dfs_stack_.empty())
	    dfs_stack_.pop();
	// Push the root on the stack.
	dfs_stack_.push(std::pair<bid_t, size_t>(header_.root_bid, 0));

	level = (int)dfs_stack_.size() - 1;
	return std::pair<bid_t, Key>(header_.root_bid, k);
    } else {
	BTREE_NODE* bn;
	bid_t id = 0;
	// If the top of the stack is a node
	if (dfs_stack_.size() < header_.height) {
	    // Fetch the node ...
	    bn = fetch_node(dfs_stack_.top().first);
	    // ... and get the appropriate child.
	    id = bn->lk[dfs_stack_.top().second];
	    dfs_stack_.push(std::pair<bid_t, size_t>(id, 0));
	    release_node(bn);
	} else { // top of the stack is leaf
	    dfs_stack_.pop();
	    bool done = false;
	    while (!dfs_stack_.empty() && !done) {
		// Fetch the node ...
		bn = fetch_node(dfs_stack_.top().first);
		// Increment the link index.
		(dfs_stack_.top().second)++;
		// Check the link index for validity.
		if (dfs_stack_.top().second < bn->size() + 1) {
		    id = bn->lk[dfs_stack_.top().second];
		    k = bn->el[dfs_stack_.top().second-1];
		    dfs_stack_.push(std::pair<bid_t, size_t>(id, 0));
		    done = true;
		} else
		    dfs_stack_.pop();

		release_node(bn);
	    }
	}
	level = (int)dfs_stack_.size() - 1;
	return std::pair<bid_t, Key>(id, k);
    }
}

/// *btree::find* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::find(const Key& k, Value& v) {

    bool ans;
    size_t idx;

    if (header_.height == 0)
	return false;

    // Find the leaf that might contain the key and fetch it.
    bid_t bid = find_leaf(k);
    BTREE_LEAF *p = fetch_leaf(bid);

    // Check whether we have a match.
    idx = p->find(k);

    if (idx < p->size() && 
	!comp_(kov_(p->el[idx]), k) && 
	!comp_(k, kov_(p->el[idx]))) {
	v = p->el[idx]; // using Value's assignment operator.
	ans = true;
    } else
	ans = false;

    // Write back the leaf and empty the stack.
    release_leaf(p);
    empty_stack();

    return ans;
}

/// *btree::pred* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::pred(const Key& k, Value& v) {

    bool ans = false;
    BTREE_LEAF * pl;
    bid_t bid;
    size_t idx;

    assert(header_.height >= 1);
    assert(path_stack_.empty());

    // Get a close candidate and path_stack
    bid = find_leaf(k);
    pl = fetch_leaf(bid);
    idx = pl->pred(k);
  
    // Check whether we have a match.
    if (comp_(kov_(pl->el[idx]),k)){
	v = pl->el[idx]; 
	ans = true;
    } else {
#if BTREE_LEAF_PREV_POINTER
	bid = pl->prev();
#else
	assert(0);
#endif
	if (bid != 0) {
	    release_leaf(pl);
	    pl = fetch_leaf(bid);
	    v = pl->el[pl->pred(k)];
	    ans=true;
	}
    }

    // Write back the leaf and empty the stack.
    release_leaf(pl);
    empty_stack();

    return ans;
}

/// *btree::succ* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::succ(const Key& k, Value& v) {

    bool ans = false;
    BTREE_LEAF * pl;
    bid_t bid;
    size_t idx;

    assert(header_.height >= 1);
    assert(path_stack_.empty());

    // Get a close candidate and path_stack
    bid = find_leaf(k);
    pl = fetch_leaf(bid);
    idx = pl->succ(k);
  
    // Check whether we have a match.
    if (comp_(k,kov_(pl->el[idx]))){
	v = pl->el[idx]; 
	ans = true;
    } else {
	bid = pl->next();
	if (bid !=0) {
	    release_leaf(pl);
	    pl = fetch_leaf(bid);
	    v = pl->el[pl->succ(k)];
	    ans=true;
	}
    }
  
    // Write back the leaf and empty the stack.
    release_leaf(pl);
    empty_stack();

    return ans;
}

/// *btree::insert* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::insert(const Value& v) {

    bool ans = true;

    // Check for empty tree.
    if (header_.height == 0) {
	return insert_empty(v);
    }

    // Find the leaf where v should be inserted and fetch it.
    bid_t bid = find_leaf(kov_(v));
    BTREE_LEAF *p = fetch_leaf(bid);

    // If the leaf is not full, insert v into it.
    if (!full_leaf(p)) {
	ans = p->insert(v);
	release_leaf(p);
    } else {
#if BTREE_UNIQUE_KEYS
	size_t pos = p->find(kov_(v));
	// Check for duplicate key. 
	if (pos < p->size() && 
	    !comp_(kov_(v), kov_(p->el[pos])) &&  
	    !comp_(kov_(p->el[pos]), kov_(v))) { 
	    TP_LOG_WARNING_ID("Attempting to insert duplicate key. Ignoring insert.");
	    ans = false; 
	} else {
	    ans = insert_split(v, p, bid);
	}
#else
	ans = insert_split(v, p, bid);
#endif
    }

    empty_stack();

    // Update the size and return.
    header_.size += ans ? 1: 0;
    return ans;
}

/// *btree::modify* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::modify(const Value& v) {
                                                                                
    bool ans = true;
                                                                                
    // Check for empty tree.
    if (header_.height == 0) {
	return insert_empty(v);
    }
                                                                                
    // Find the leaf where v should be inserted and fetch it.
    bid_t bid = find_leaf(kov_(v));
    BTREE_LEAF *p = fetch_leaf(bid);
                                                                                
    if(p->erase(kov_(v))){
	// Item was present, can insert without overflow
	ans = p->insert(v);
	release_leaf(p);
    }
    else{
	// Item was not present, do a standard insert
	// If the leaf is not full, insert v into it.
	if (!full_leaf(p)) {
	    ans = p->insert(v);
	    release_leaf(p);
	}
	else {
#if BTREE_UNIQUE_KEYS
	    size_t pos = p->find(kov_(v));
	    // Check for duplicate key.
	    if (pos < p->size() &&
                !comp_(kov_(v), kov_(p->el[pos])) &&
                !comp_(kov_(p->el[pos]), kov_(v))) {
		TP_LOG_WARNING_ID("Attempting to insert duplicate key. Ignoring insert.");
		ans = false;
	    }
	    else {
		ans = insert_split(v, p, bid);
	    }
#else
	    ans = insert_split(v, p, bid);
#endif
	}
    }
                                                                               
    empty_stack();
                                                                               
    // Return answer.
    return ans;
}

/// *btree::insert_load* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
inline bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::insert_load(const Value& v, BTREE_LEAF* &lcl) {

    BTREE_LEAF *p;
    bool ans = false;
    bid_t bid;

    // Check for empty tree.
    if (header_.height == 0) {
	ans = insert_empty(v);
	lcl = fetch_leaf(header_.root_bid);
	return ans;
    }

    p = lcl;
    // Verify sorting.
    ///  assert(!comp_(kov_(v), kov_(lcl->el[lcl->size()-1])));
  
	if (!comp_(kov_(p->el[p->size()-1]), kov_(v)))
	    ans = false;
	else {
	    // If the leaf is not full, insert v into it.
	    if (!full_leaf(p)) {
		ans = p->insert(v);
	    } else {
		//bid_t pbid = p->bid();///
		release_leaf(p);
      
		// Do the whole routine.
		bid = find_leaf(kov_(v));
		//assert(bid == pbid);///
		// Should be in cache.
		p = fetch_leaf(bid);
		// bid will store the id of the leaf containing v after insert.
		ans = insert_split(v, p, bid, true);
		lcl = fetch_leaf(bid);
	    }
	}

	empty_stack();

	// Update the size and return.
	header_.size += ans ? 1: 0;
	return ans;
}


/// *btree::insert_empty* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::insert_empty(const Value& v) {
    bool ans;
    assert(header_.size == 0);

    // Create new root (as leaf).
    BTREE_LEAF* lroot = fetch_leaf();
  
    // Store its bid.
    header_.root_bid = lroot->bid();
  
    lroot->next() = 0;
#if BTREE_LEAF_PREV_POINTER
    lroot->prev() = 0;
#endif

    // Insert v into the root.
    ans = lroot->insert(v);
    assert(ans);
  
    // Don't want the root object around.
    release_leaf(lroot);
  
    // Height and size are now 1.
    header_.height = 1;
    header_.size = 1;
  
    status_ = BTREE_STATUS_VALID;
    return ans;
}

/// *btree::insert_split* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::insert_split(const Value& v, BTREE_LEAF* p, bid_t& leaf_id, bool loading) {

    BTREE_LEAF *q, *r;
    std::pair<bid_t, size_t> top;
    bid_t bid;
    bool ans;

    // Split the leaf.
    q = fetch_leaf();
    Key mid_key;
    if (loading) {
	mid_key = kov_(p->el[p->size()-1]);
    } else
	mid_key = p->split(*q);

    // Update the next pointers.
    q->next() = p->next();
    p->next() = q->bid();

#if BTREE_LEAF_PREV_POINTER
    // Update the prev pointers.
    q->prev() = p->bid();  
    if (q->next() != 0) {
	r = fetch_leaf(q->next());
	r->prev() = q->bid();
	release_leaf(r);
    }
#endif

    bid = q->bid();
  
    // Insert in the appropriate leaf.
    if (!comp_(mid_key, kov_(v)) && !comp_(kov_(v), mid_key)) {
	ans = false;
	TP_LOG_WARNING_ID("Attempting to insert duplicate key");
	// TODO: during loading, this is not enough. q may remain empty!
    } else {
	ans = (comp_(mid_key, kov_(v)) ? q: p)->insert(v);
	leaf_id = (comp_(mid_key, kov_(v)) ? q: p)->bid();
	assert(!loading || q->size() == 1);
    }

    release_leaf(p);
    release_leaf(q);
  
    Key fmid_key;  
    BTREE_NODE *qq, *fq;
  
    // Go up the tree.
    while (bid != 0 && !path_stack_.empty()) {
    
	// Pop the stack to find q's father.
	top = path_stack_.top();
	path_stack_.pop();
    
	// Read the father of q.
	fq = fetch_node(top.first);
    
	// Check whether we need to go further up the tree.
	if (!full_node(fq)) {
      
	    // Insert the key and link into position.
	    fq->insert_pos(mid_key, bid, top.second, top.second + 1);

	    // Exit the loop.
	    bid = 0;
      
	} else { // Need to split further.
      
	    // Split fq.
	    qq = fetch_node();
	    fmid_key = loading ? mid_key: fq->split(*qq);
      
	    // Insert in the appropriate node.
	    if (loading)
		qq->lk[0] = bid; // TODO: this is ugly. qq has no keys now. 
	    else
		(comp_(fmid_key, mid_key) ? qq: fq)->insert(mid_key, bid);
      
	    // Prepare for next iteration.
	    mid_key = fmid_key;
	    bid = qq->bid();
	    release_node(qq);
	}

	release_node(fq);
    
    } // End of while.
  
    // Check whether the root was split.
    if (bid != 0) {
    
	assert(path_stack_.empty());
    
	// Create a new root node with the 2 links.
	BTREE_NODE* nroot = fetch_node();
	// Not very nice...
	nroot->lk[0] = header_.root_bid;
	nroot->insert_pos(mid_key, bid, 0, 1);
    
	// Update the root id.
	header_.root_bid = nroot->bid();
    
	release_node(nroot);
    
	// Update the height.
	header_.height++;
    
    } 
    return ans;
}


/// *btree::find_leaf* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bid_t btree<Key, Value, Compare, KeyOfValue, BTECOLL>::find_leaf(const Key& k) {

    BTREE_NODE * p;
    bid_t bid = header_.root_bid;
    size_t pos;
    TPIE_OS_SIZE_T level;

    assert(header_.height >= 1);
    assert(path_stack_.empty());

    // Go down the tree.
    for (level = header_.height - 1; level > 0; level--) {
	// Fetch the node.
	p = fetch_node(bid);
	// Find the position of the link to the child node.
	pos = p->find(k);
	// Push the current node and position on the path stack.
	path_stack_.push(std::pair<bid_t, size_t>(bid, pos));
	// Find the actual block id of the child node.
	bid = p->lk[pos];
	// Release the node.
	release_node(p);
    }

    // This should be the id of a leaf.
    return bid;
}

/// *btree::find_min_leaf* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bid_t btree<Key, Value, Compare, KeyOfValue, BTECOLL>::find_min_leaf() {
    BTREE_NODE* p;
    bid_t bid = header_.root_bid;
    int level;

    assert(header_.height >= 1);
  
    for (level = (int)header_.height - 1; level > 0; level--) {    
	p = fetch_node(bid);
	bid = p->lk[0];
	release_node(p);
    }

    return bid;
}

/// *btree::underflow_leaf* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::underflow_leaf(BTREE_LEAF *p) const {
    return p->size() <= cutoff_leaf(p);
}

/// *btree::underflow_node* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::underflow_node(BTREE_NODE *p) const {
    return p->size() <= cutoff_node(p);
}

/// *btree::cutoff_leaf* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
size_t btree<Key, Value, Compare, KeyOfValue, BTECOLL>::cutoff_leaf(BTREE_LEAF *p) const {
    // Be careful how you test for the root (thanks, Andrew).
    return (p->bid() == header_.root_bid && header_.height == 1) ? 0 
	: params_.leaf_size_min - 1;
}

/// *btree::cutoff_node* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
size_t btree<Key, Value, Compare, KeyOfValue, BTECOLL>::cutoff_node(BTREE_NODE *p) const {
    return (p->bid() == header_.root_bid) ? 0 : params_.node_size_min - 1;
}

/// *btree::full_leaf* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::full_leaf(const BTREE_LEAF *p) const {
    return (p->size() == params_.leaf_size_max); 
}

/// *btree::full_node* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::full_node(const BTREE_NODE *p) const {
    return (p->size() == params_.node_size_max);
}

/// *btree::balance_node* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::balance_node(BTREE_NODE *f, BTREE_NODE *p, size_t pos) {

    bool ans = false;
    BTREE_NODE *sib;

    assert(p->bid() == f->lk[pos]);

    // First try to borrow from the right sibling.
    if (pos < f->size()) {

	sib = fetch_node(f->lk[pos + 1]);
	if (sib->size() >= cutoff_node(sib) + 2) {

	    // Rotate left. Insert the key from the father (f) and the link
	    // from the sibling (sib) to the end of p.
	    p->insert_pos(f->el[pos], sib->lk[0], p->size(), p->size() + 1);
	    // Move the key from sib up to the father (f).
	    f->el[pos] = sib->el[0];
	    // Remove the first key and link of sib.
	    sib->erase_pos(0, 0);
	    ans = true;

	} 
	release_node(sib);
    }

    if (pos > 0 && !ans) {

	sib = fetch_node(f->lk[pos - 1]);
	if (sib->size() >= cutoff_node(sib) + 2) {
	    // Rotate right.
	    p->insert_pos(f->el[pos - 1], sib->lk[sib->size()], 0, 0);
	    f->el[pos - 1] = sib->el[sib->size() - 1];
	    sib->erase_pos(sib->size() - 1, sib->size());
	    ans = true;

	} 
	release_node(sib);
    }

    // Return.
    return ans;
}

/// *btree::balance_leaf* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::balance_leaf(BTREE_NODE *f, BTREE_LEAF *p, size_t pos) {

    bool ans = false;
    BTREE_LEAF *sib;

    // First try to borrow from the right sibling.
    if (pos < f->size()) {
	sib = fetch_leaf(f->lk[pos + 1]);
	if (sib->size() >= cutoff_leaf(sib) + 2) {

#if (!BTREE_LEAF_ELEMENTS_SORTED)
	    sib->sort();
#endif
	    // Rotate left.
	    // Insert the first element from sib to the end of p.
	    p->insert(sib->el[0]);
	    // Update the key in the father (f).
	    f->el[pos] = kov_(sib->el[0]);
	    // Delete the first element from sib.
	    sib->erase_pos(0);
	    ans = true;

	}
	release_leaf(sib);
    }

    if (pos > 0 && !ans) {

	sib = fetch_leaf(f->lk[pos - 1]);
	if (sib->size() >= cutoff_leaf(sib) + 2) {

#if (!BTREE_LEAF_ELEMENTS_SORTED)
	    sib->sort();
#endif
	    // Rotate right.
	    // Insert the last element of sib to the beginning of p.
	    p->insert(sib->el[sib->size() - 1]);
	    // Update the key in the father.
	    f->el[pos - 1] = kov_(sib->el[sib->size() - 2]);
	    // Delete the last element from sib.
	    sib->erase_pos(sib->size() - 1); 
	    ans = true;

	}
	release_leaf(sib);
    }

    return ans;
}

/// *btree::merge_leaf* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void btree<Key, Value, Compare, KeyOfValue, BTECOLL>::merge_leaf(BTREE_NODE* f, BTREE_LEAF* &p, size_t pos) {

    BTREE_LEAF * sib, *r;

    // f will be the father of both p and sib.

    if (pos < f->size()) {

	// Merge with right sibling.
	// Fetch the sibling.
	sib = fetch_leaf(f->lk[pos + 1]);
	// Update the next pointer.
	p->next() = sib->next();
#if BTREE_LEAF_PREV_POINTER
	// Update the prev pointer.
	if (p->next() != 0) {
	    r = fetch_leaf(p->next());
	    r->prev() = p->bid();
	    release_leaf(r);
	}
#endif
	// Do the merge.
	p->merge(*sib);
	// Delete the sibling.
	sib->persist(PERSIST_DELETE);
	release_leaf(sib);
	// Delete the entry for the sibling from the father.
	f->erase_pos(pos, pos + 1);

    } else {

	// Merge with left sibling.
	sib = fetch_leaf(f->lk[pos - 1]);
	sib->next() = p->next();
#if BTREE_LEAF_PREV_POINTER
	if (sib->next() != 0) {
	    r = fetch_leaf(sib->next());
	    r->prev() = sib->bid();
	    release_leaf(r);
	}
#endif
	sib->merge(*p);
	p->persist(PERSIST_DELETE);
	release_leaf(p);
	f->erase_pos(pos - 1, pos);
	p = sib;
    }
  
}

/// *btree::merge_node* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
void btree<Key, Value, Compare, KeyOfValue, BTECOLL>::merge_node(BTREE_NODE* f, BTREE_NODE* &p, size_t pos) {

    BTREE_NODE * sib;

    // f will be the father of both p and sib.

    if (pos < f->size()) {
	sib = fetch_node(f->lk[pos + 1]);
	p->merge(*sib, f->el[pos]);
	// Delete the sibling.
	sib->persist(PERSIST_DELETE);
	release_node(sib);
	f->erase_pos(pos, pos + 1);
    } else {
	sib = fetch_node(f->lk[pos - 1]);
	sib->merge(*p, f->el[pos - 1]);
	p->persist(PERSIST_DELETE);
	release_node(p);
	f->erase_pos(pos - 1, pos);
	p = sib;
    }

}

/// *btree::erase* ///
template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
bool btree<Key, Value, Compare, KeyOfValue, BTECOLL>::erase(const Key& k) {

    bool ans;

    if (header_.height == 0) 
	return false;

    // Find the leaf where the data might be and fetch it.
    bid_t bid = find_leaf(k);
    BTREE_LEAF *p = fetch_leaf(bid);

    // Check for exact match and delete.
    ans = p->erase(k);

    // Sanity check.
    assert(ans || !underflow_leaf(p));

    // Update the size.
    header_.size -= ans ? 1: 0;

    if (!underflow_leaf(p)) { 
	// No underflow. Cleanup and exit.
	release_leaf(p);
	empty_stack();
	return ans;
    }

    BTREE_NODE * q=NULL;
    std::pair<bid_t, size_t> top;

    // Underflow. Balance or merge up the tree.
    // Treat the first iteration separately since it deals with leaves.
    if (!path_stack_.empty()) {

	// Pop the father of p from the stack.
	top = path_stack_.top();
	path_stack_.pop();

	// Load the father of p;
	q = fetch_node(top.first);

	// Can we borrow an element from a sibling?
	if (balance_leaf(q, p, top.second)) {
	    bid = 0; // Done.
	} else {

	    // Merge p with a sibling.
	    merge_leaf(q, p, top.second);

	    // Check for underflow in the father.
	    bid = (underflow_node(q) ? q->bid() : 0);
	}

	// Prepare for next iteration (or exit).
	release_leaf(p);
    }

    BTREE_NODE * pp = q;

    // The rest of the iterations up the tree.
    while (!path_stack_.empty() && bid != 0) {
      
	// Find the father of p.
	top = path_stack_.top();
	path_stack_.pop();

	// Load the father of p;
	q = fetch_node(top.first);

	// Try to balance p by borrowing from sibling(s).
	if (balance_node(q, pp, top.second)) {

	    bid = 0;

	} else {
      
	    // Merge p with right sibling.
	    merge_node(q, pp, top.second);

	    // Check for underflow in the father.
	    bid = (underflow_node(q) ? q->bid() : 0);
	}

	// Prepare for next iteration (or exit).
	release_node(pp);
	pp = q;

    } // end of while..

    // Check for root underflow.
    if (bid != 0) {
    
	assert(path_stack_.empty());
	assert(pp->bid() == header_.root_bid);

	// New root.
	header_.root_bid = pp->lk[0];

	// Remove old root from collection.
	pp->persist(PERSIST_DELETE);

	header_.height--;
    }

    release_node(pp);

    // Empty the path stack and return.
    empty_stack();
    return ans;
}

	template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	void btree<Key, Value, Compare, KeyOfValue, BTECOLL>::persist(persistence per) {
	    pcoll_leaves_->persist(per);
	    pcoll_nodes_->persist(per);
	}

	template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	btree<Key, Value, Compare, KeyOfValue, BTECOLL>::~btree() {
	    if (status_ == BTREE_STATUS_VALID) {
		// Write initialization info into the pcoll_nodes_ header.
		*((header_t *) pcoll_nodes_->user_data()) = header_;
	    }
	    tpie_delete(node_cache_);
	    tpie_delete(leaf_cache_);

	    // Delete the two collections.
	    tpie_delete(pcoll_leaves_);
	    tpie_delete(pcoll_nodes_);
	}

	template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	BTREE_NODE* btree<Key, Value, Compare, KeyOfValue, BTECOLL>::fetch_node(bid_t bid) {
	    BTREE_NODE* q;
	    stats_.record(NODE_FETCH);
	    // Warning: using short-circuit evaluation. Order is important.
	    if ((bid == 0) || !node_cache_->read(bid, q)) {
			q = tpie_new<BTREE_NODE>(pcoll_nodes_, bid);
	    }
	    return q;
	}

	template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	BTREE_LEAF* btree<Key, Value, Compare, KeyOfValue, BTECOLL>::fetch_leaf(bid_t bid) {
	    BTREE_LEAF* q;
	    stats_.record(LEAF_FETCH);
	    // Warning: using short-circuit evaluation. Order is important.
	    if ((bid == 0) || !leaf_cache_->read(bid, q)) {
		q = tpie_new<BTREE_LEAF>(pcoll_leaves_, bid);
	    }
	    return q;
	}

	template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	void btree<Key, Value, Compare, KeyOfValue, BTECOLL>::release_node(BTREE_NODE *p) {
	    stats_.record(NODE_RELEASE);
	    if (p->persist() == PERSIST_DELETE)
			tpie_delete(p);
	    else
		node_cache_->write(p->bid(), p);
	}

	template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	void btree<Key, Value, Compare, KeyOfValue, BTECOLL>::release_leaf(BTREE_LEAF *p) {
	    stats_.record(LEAF_RELEASE);
	    if (p->persist() == PERSIST_DELETE)
		delete p;
	    else
		leaf_cache_->write(p->bid(), p);
	}

	template <class Key, class Value, class Compare, class KeyOfValue, class BTECOLL>
	const stats_tree& btree<Key, Value, Compare, KeyOfValue, BTECOLL>::stats() {
	    node_cache_->flush();
	    leaf_cache_->flush();
	    stats_.set(LEAF_READ, pcoll_leaves_->stats().get(BLOCK_GET));
	    stats_.set(LEAF_WRITE, pcoll_leaves_->stats().get(BLOCK_PUT));
	    stats_.set(LEAF_CREATE, pcoll_leaves_->stats().get(BLOCK_NEW));
	    stats_.set(LEAF_DELETE, pcoll_leaves_->stats().get(BLOCK_DELETE));
	    stats_.set(LEAF_COUNT, pcoll_leaves_->size());
	    stats_.set(NODE_READ, pcoll_nodes_->stats().get(BLOCK_GET));
	    stats_.set(NODE_WRITE, pcoll_nodes_->stats().get(BLOCK_PUT));
	    stats_.set(NODE_CREATE, pcoll_nodes_->stats().get(BLOCK_NEW));
	    stats_.set(NODE_DELETE, pcoll_nodes_->stats().get(BLOCK_DELETE));
	    stats_.set(NODE_COUNT, pcoll_nodes_->size());
	    return stats_;
	}

// Undefine shortcuts.
#undef BTREE_NODE 
#undef BTREE_LEAF 
#undef BTREE      

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_BTREE_H
