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

// AMI collection entry points implemented on top of a single BTE.

#ifndef _AMI_COLL_SINGLE_H
#define _AMI_COLL_SINGLE_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// For persist type.
#include <tpie/persist.h>
// Get an appropriate BTE collection.
#include <tpie/bte/coll.h>
// For AMI_collection_type and AMI_collection_status.
#include <tpie/coll_base.h>
// The tpie_tempnam() function.
#include <tpie/tempname.h>
// Get the stats_coll class for collection statistics.
#include <tpie/stats_coll.h>

namespace tpie {

    namespace ami {

  ///////////////////////////////////////////////////////////////////
  /// Class representing a collection of fixed size blocks. 
  /// Ech block can have links to other
  /// blocks and is identified by a block ID of type \ref bid_t.
  ///
  /// collection_single<BTE_COLL> is the for now the only implementation of a block
  /// collection in TPIE; therefore \ref AMI_collection is defined as
  /// \ref collection_single in coll.h
  ///////////////////////////////////////////////////////////////////////////
	template <class BTECOLL = bte::COLLECTION>       
	class collection_single {
	    
	public:
	    
	    //////////////////////////////////////////////////////////////////////////
	    /// Create a new collection with access type AMI_WRITE_COLLECTION using
	    /// temporary file names.  
	    /// \param[in] logical_block_factor parameter determines the size of the
	    /// blocks stored (as the parameter value times the operating system page 
	    /// size in bytes. The persistency of the collection is set to 
	    /// \ref PERSIST_DELETE. 
	    //////////////////////////////////////////////////////////////////////////
	    collection_single(TPIE_OS_SIZE_T logical_block_factor = 1);
	    
      //////////////////////////////////////////////////////////////////////////
	    /// Create a new or open an existing collection.
	    /// \param[in] path_name location and name of the collection file. 
	    /// \param[in] ct The access pattern of type \ref collection_type
      /// \param[in] logical_block_factor parameter determines the size of the
      /// blocks stored (as the parameter value times the operating system page 
      /// size in bytes. The persistency of the collection is set to 
      /// \ref PERSIST_DELETE. 
	    //////////////////////////////////////////////////////////////////////////
	collection_single(const std::string& path_name,
					  collection_type ct = READ_WRITE_COLLECTION,
					  TPIE_OS_SIZE_T logical_block_factor = 1);

      //////////////////////////////////////////////////////////////////////////
      /// Returns the total number of blocks used by the collection.
	    //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_OFFSET size() const { 
		return btec_->size(); 
	    }

      //////////////////////////////////////////////////////////////////////////
	    /// Return the size of a block stored in this collection, in bytes
	    /// (all blocks in a collection have the same size). 
	    //////////////////////////////////////////////////////////////////////////
	    // Return the logical block size in bytes.
	    TPIE_OS_SIZE_T block_size() const { 
		return btec_->block_size(); 
	    }
	    
      //////////////////////////////////////////////////////////////////////////
	    /// Returns the logical block factor. The block size is obtained by 
	    /// multiplying the operating system page size in bytes by this value. 
	    //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T block_factor() const {
		return btec_->block_factor(); 
	    }

      //////////////////////////////////////////////////////////////////////////
	    //// Set the persistency flag to p. The possible values for p are 
	    /// \ref PERSIST_PERSISTENT and \ref PERSIST_DELETE. 
	    //////////////////////////////////////////////////////////////////////////
	    // Set the persistence flag. 
	    void persist(persistence p) { 
		btec_->persist(p); 
	    }

      //////////////////////////////////////////////////////////////////////////
	    /// Return the value of the persistency flag.
	    //////////////////////////////////////////////////////////////////////////
	    persistence persist() const {
		return btec_->persist(); 
	    }

      //////////////////////////////////////////////////////////////////////////
	    /// Return the status of the collection. The result is either 
	    /// \ref COLLECTION_STATUS_VALID or 
	    /// \ref COLLECTION_STATUS_INVALID. 
	    /// The only operation that can leave the collection invalid 
	    /// is the constructor (if that happens, the log file contains more i
	    /// nformation). No blocks should be read from or written to an invalid 
	    /// collection.
	    //////////////////////////////////////////////////////////////////////////
	    collection_status status() const { 
		return status_; 
	    }
	    
      //////////////////////////////////////////////////////////////////////////
      /// Return if the status of the collection is 
	    /// \ref COLLECTION_STATUS_VALID.
	    /// \sa status()
	    //////////////////////////////////////////////////////////////////////////
	    bool is_valid() const { 
		return status_ == COLLECTION_STATUS_VALID; 
	    }
	    
      //////////////////////////////////////////////////////////////////////////
      /// Return if the status of the collection is not 
	    /// \ref COLLECTION_STATUS_VALID. 
	    /// \sa is_valid(), status()
	    //////////////////////////////////////////////////////////////////////////
	    bool operator!() const { 
		return !is_valid(); 
	    }
	    
      //////////////////////////////////////////////////////////////////////////
      /// Return a pointer to a 512-byte array stored in the header of the 
	    /// collection. This can be used by the application to store
	    /// initialization information (e.g., in case the collection represents 
	    /// a B-tree, the id of the block containing the root of the B-tree).
	    //////////////////////////////////////////////////////////////////////////
	    void *user_data() { 
		return btec_->user_data(); 
	    }
	    
	    //////////////////////////////////////////////////////////////////////////
	    /// Destructor. Closes all files of the collection. If persistency is set to 
	    /// \ref PERSIST_DELETE, it also removes the files. There should be no
	    /// blocks in memory; if the destructor detects in-memory blocks, it 
	    /// issues a warning in the TPIE log file (if logging is turned on). 
	    /// Note, that The memory held by those blocks cannot be recoeverd and is 
	    /// lost to the TPIE program instance.  	    
	    //////////////////////////////////////////////////////////////////////////
	    ~collection_single() { 
			tpie_delete(btec_);
	    }
	    
      //////////////////////////////////////////////////////////////////////////
      /// Returns a pointer to the BTE collection underlying this collection.
	    //////////////////////////////////////////////////////////////////////////
	    BTECOLL* bte() { 
		return btec_; 
	    }
	    
      //////////////////////////////////////////////////////////////////////////
      /// Returns an object of type \ref tpie::stats_collection containing the 
	    ////statistics of this collection. 
	    /// \sa gstats().
	    //////////////////////////////////////////////////////////////////////////
	    const stats_collection& stats() const { 
		return btec_->stats(); 
	    }
	    
      //////////////////////////////////////////////////////////////////////////
	    /// Returns an object of type \ref stats_collection containing the 
	    /// statistics of all collections opened by the application (global 
	    /// statistics). 
	    /// \sa stats().
	    //////////////////////////////////////////////////////////////////////////
	    static const stats_collection& gstats() { 
		return BTECOLL::gstats(); 
	    }
	    
	private:
	    
	    /** The BTE collection underlying this collection. */
	    BTECOLL *btec_;
	    
	    /** Validity status information about this collection. */
	    collection_status status_;
	    
	};
	
	template <class BTECOLL>
	collection_single<BTECOLL>::collection_single(TPIE_OS_SIZE_T lbf) {
	    
		std::string temp_path = tempname::tpie_name("");
	    
	    btec_ = tpie_new< BTECOLL>(temp_path, bte::WRITE_COLLECTION, lbf);
	    tp_assert(btec_ != NULL, "new failed to create a new BTE_COLLECTION."); 
	    btec_->persist(PERSIST_DELETE);
	    
	    if (btec_->status() == bte::COLLECTION_STATUS_VALID) {
		status_ = COLLECTION_STATUS_VALID;
	    }
	    else {
		status_ = COLLECTION_STATUS_INVALID;
	    }
	}
	
	template <class BTECOLL>
	collection_single<BTECOLL>::collection_single(const std::string& path_name,
												  collection_type ct, 
												  TPIE_OS_SIZE_T lbf) {

	    bte::collection_type btect;
	    
	    if (ct == READ_COLLECTION) {
		btect = bte::READ_COLLECTION;
	    }
	    else {
		btect = bte::WRITE_COLLECTION;
	    }
   
	    btec_ = tpie_new<BTECOLL>(path_name, btect, lbf);
	    
	    tp_assert(btec_ != NULL, "new failed to create a new BTE_COLLECTION.");
	    btec_->persist(PERSIST_PERSISTENT);

	    if (btec_->status() == bte::COLLECTION_STATUS_VALID) {
		status_ = COLLECTION_STATUS_VALID;
	    }
	    else {
		status_ = COLLECTION_STATUS_INVALID;
	    }
	}

    }  //  ami namespace

} //  tpie namespace

#endif // _TPIE_AMI_COLL_SINGLE_H
