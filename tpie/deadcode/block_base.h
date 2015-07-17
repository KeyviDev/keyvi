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

#ifndef _AMI_BLOCK_BASE_H
#define _AMI_BLOCK_BASE_H

///////////////////////////////////////////////////////////////////
/// \file block_base.h
/// Definition of the block_base class and supporting types:
///  bid, block_status.
/// \sa block.h
///////////////////////////////////////////////////////////////////


// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// The AMI error codes.
#include <tpie/err.h>
// The AMI_COLLECTION class.
#include <tpie/coll.h>



namespace tpie {

    namespace ami {
    
    /** TPIE block id type. */
    typedef TPIE_BLOCK_ID_TYPE bid_t;

    /** Status info about the block */
    enum block_status {
      /** Block is valid, that is, its contents are supposed to be reliable. */
      BLOCK_STATUS_VALID = 0,
      /** Block is invalid, that is, its contents are not guaranteed to be reliable. */
      BLOCK_STATUS_INVALID = 1
    };
	
///////////////////////////////////////////////////////////////////
/// Superclass of class \ref tpie::ami::block< E,I,BTECOLL >. 
/// \sa block.h 
///////////////////////////////////////////////////////////////////
  template<class BTECOLL>
	class block_base {

	protected:
	    
	    /** Pointer to the block collection */
	    BTECOLL * pcoll_;
	    
	    /** Unique ID. Represents the offset of the block in the blocks file. */
	    bid_t bid_;
	    
	    /** Dirty bit. If set, the block needs to be written back. */
	    char dirty_;
	    
	    /** Pointer to the actual data. */
	    void * pdata_;
	    
	    /** Persistence flag. */
	    persistence per_;
	    
	public:
	    
  	  ///////////////////////////////////////////////////////////////////
	    /// Constructor. Reads and initializes a block with a given ID.
	    /// When bid is missing or 0, a new block is created.
	    ///////////////////////////////////////////////////////////////////
	    block_base(collection_single<BTECOLL>* pacoll, bid_t bid = 0)
		: bid_(bid), dirty_(0), per_(PERSIST_PERSISTENT) {

		pcoll_ = pacoll->bte();

		if (bid != 0) {

		    // Get an existing block from disk.
		    if (pcoll_->get_block(bid_, pdata_) != bte::NO_ERROR) {
			pdata_ = NULL;
		    }

		} else {

		    // Create a new block in the collection.
		    if (pcoll_->new_block(bid_, pdata_) != bte::NO_ERROR) {
			pdata_ = NULL;
		    }
		}
	    }

      ///////////////////////////////////////////////////////////////////
	    /// Synchronize the in-memory image of the block with the one stored 
	    /// in external storage.
	    ///////////////////////////////////////////////////////////////////
	    err sync() {
		if (pcoll_->sync_block(bid_, pdata_) != bte::NO_ERROR)
		    return BTE_ERROR;
		else
		    return NO_ERROR;
	    }

      ///////////////////////////////////////////////////////////////////
	    /// Get the id of the block.
	    ///
      ///////////////////////////////////////////////////////////////////
	    bid_t bid() const { 
		return bid_; 
	    }

      ///////////////////////////////////////////////////////////////////
      /// Returns a reference to the dirty bit. The dirty bit is used to 
      /// optimize writing in some implementations of the block collection 
      /// class. It should be set to 1 whenever the block data 
      /// is modified to signal that the copy of the block on disk would
      /// have to be updated to be consistent with the block object.
      ///////////////////////////////////////////////////////////////////
	    char& dirty() { 
		return dirty_; 
	    };
	    
      ///////////////////////////////////////////////////////////////////
	    /// Returns the dirty bit. The dirty bit is used to 
	    /// optimize writing in some implementations of the block collection 
	    /// class. It should be set to 1 whenever the block data 
	    /// is modified to signal that the copy of the block on disk would
	    /// have to be updated to be consistent with the block object.
	    ///////////////////////////////////////////////////////////////////
	    char dirty() const { 
		return dirty_; 
	    }

      ///////////////////////////////////////////////////////////////////
	    /// Copy  block B into the current block, if both blocks are 
	    /// associated with the same collection. 
	    /// Returns a reference to this block. 
      ///////////////////////////////////////////////////////////////////
	    block_base<BTECOLL>& operator=(const block_base<BTECOLL>& rhs) { 
		if (pcoll_ == rhs.pcoll_) {
		    memcpy(pdata_, rhs.pdata_, pcoll_->block_size());
		    dirty_ = 1;
		} else 
		    pdata_ = NULL;
		return *this; 
	    }

      ///////////////////////////////////////////////////////////////////
      /// Returns the status of the block. The result is either 
      /// \ref BLOCK_STATUS_VALID or  \ref BLOCK_STATUS_INVALID. 
      /// The status of a
      /// block instance is set during construction. The methods of an
      /// invalid block can give erroneous results or fail.      
      ///////////////////////////////////////////////////////////////////
	    block_status status() const { 
		return (pdata_ == NULL) ? 
		    BLOCK_STATUS_INVALID : BLOCK_STATUS_VALID; 
	    }

      ///////////////////////////////////////////////////////////////////
	    /// Returns if the block's status is  \ref BLOCK_STATUS_VALID. 
	    /// \sa status().      
	    ///////////////////////////////////////////////////////////////////
	    bool is_valid() const {
		return (pdata_ != NULL);
	    }

      ///////////////////////////////////////////////////////////////////
      /// Returns if the block's status is not  \ref BLOCK_STATUS_VALID.
	    /// \sa is_valid() and status().
	    ///
	    ///////////////////////////////////////////////////////////////////
	    bool operator!() const {
		return (pdata_ == NULL);
	    }

      ///////////////////////////////////////////////////////////////////
      /// Set the persistency flag to p. 
	    /// The possible values for p are \ref PERSIST_PERSISTENT and 
	    /// \ref PERSIST_DELETE.
	    ///////////////////////////////////////////////////////////////////
	    void persist(persistence per) { 
		per_ = per; 
	    }

      ///////////////////////////////////////////////////////////////////
      /// Return the value of the persistency flag.
	    ///////////////////////////////////////////////////////////////////
	    persistence persist() const { 
		return per_; 
	    }

      ///////////////////////////////////////////////////////////////////
      /// Return the size of this block in bytes.
	    ///////////////////////////////////////////////////////////////////
	    size_t block_size() const {
		return pcoll_->block_size(); 
	    }

      ///////////////////////////////////////////////////////////////////
      /// Destructor. If persistency is \ref PERSIST_DELETE, remove the block 
	    /// from the collection. If it is \ref PERSIST_PERSISTENT, write the
	    /// block to the collection. Deallocate the memory.
	    ///////////////////////////////////////////////////////////////////
	    ~block_base() {
		// Check first the status of the collection. 
		if (pdata_ != NULL){
		    if (per_ == PERSIST_PERSISTENT) {
			// Write back the block.
			pcoll_->put_block(bid_, pdata_); 
		    } else {
			// Delete the block from the collection.
			pcoll_->delete_block(bid_, pdata_);
		    }
		}
	    }
	};

    }  //  ami namespace

}  //  tpie namespace

#endif //_TPIE_AMI_BLOCK_BASE_H
