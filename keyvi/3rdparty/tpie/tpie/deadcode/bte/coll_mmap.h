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

// BTE_collection_mmap class definition.
#ifndef _TPIE_BTE_COLL_MMAP_H
#define _TPIE_BTE_COLL_MMAP_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
// Get the base class.
#include <tpie/bte/coll_base.h>

// For header's type field (77 == 'M').
#define COLLECTION_MMAP_ID 77

// Define write behavior, if not already defined by the user.
// Allowed values:
//  0    (synchronous writes)
//  1    (asynchronous writes using MS_ASYNC - see msync(2))
//  2    (asynchronous bulk writes) [default]
#ifndef COLLECTION_MMAP_LAZY_WRITE 
#  define COLLECTION_MMAP_LAZY_WRITE 2
#endif

namespace tpie {

    namespace bte {

	template<class BIDT = TPIE_BLOCK_ID_TYPE>
	class collection_mmap: public collection_base<BIDT> {

	protected:
	    using collection_base<BIDT>::header_;
	    using collection_base<BIDT>::freeblock_stack_;
	    using collection_base<BIDT>::bcc_fd_;
	    using collection_base<BIDT>::per_;
	    using collection_base<BIDT>::os_block_size_;
	    using collection_base<BIDT>::base_file_name_;
	    using collection_base<BIDT>::status_;
	    using collection_base<BIDT>::read_only_;
	    using collection_base<BIDT>::in_memory_blocks_;
	    using collection_base<BIDT>::file_pointer;
	    using collection_base<BIDT>::stats_;
	    using collection_base<BIDT>::gstats_;
	    using collection_base<BIDT>::bid_to_file_offset;
	    using collection_base<BIDT>::create_stack;
	    using collection_base<BIDT>::new_block_getid;
	    using collection_base<BIDT>::delete_block_shared;
	

	public:
	    // Constructor. Read and verify the header of the
	    // collection. Implemented in the base class.
	    collection_mmap(const std::string& base_file_name,
			    collection_type type = WRITE_COLLECTION,
			    TPIE_OS_SIZE_T logical_block_factor = 1):
		collection_base<BIDT>(base_file_name, 
				      type, 
				      logical_block_factor, 
				      TPIE_OS_FLAG_USE_MAPPING_TRUE) {

		header_.type = COLLECTION_MMAP_ID;
	    }
	
	    // Allocate a new block in block collection and then map that block
	    // into memory, allocating and returning an appropriately
	    // initialized Block. Main memory usage increases.
	    err new_block(BIDT &bid, void * &place) {

		err retval = NO_ERROR;

		// Get a block id.
		if ((retval = new_block_getid(bid)) != NO_ERROR) {
		    return retval;
		}

		// We have a bid, so we can call the get_block routine.
		if ((retval = get_block_internals(bid, place)) != NO_ERROR) {
		    return retval;   
		}

		header_.used_blocks++;

		stats_.record(BLOCK_NEW);
		gstats_.record(BLOCK_NEW);

		return NO_ERROR;
	    
	    }

	    // Delete a previously created, currently mapped-in BLOCK. This causes the
	    // number of free blocks in the collection to increase by 1, the bid is
	    // entered into the stdio_stack.  NOTE that it is the onus of the user of
	    // this class to ensure that the bid of this placeholder is correct. No
	    // check is made if the bid is an invalid or previously unallocated bid,
	    // which will introduce erroneous entries in the stdio_stack of free
	    // blocks. Main memory usage goes down.
	    err delete_block(BIDT bid, void * place) {
	    
		err retval = NO_ERROR;
	    
		if ((retval = put_block_internals(bid, place, 1)) != NO_ERROR) {
		    return retval; 
		}
	    
		if ((retval = delete_block_shared(bid)) != NO_ERROR) {
		    return retval;
		}

		header_.used_blocks--;
	    
		stats_.record(BLOCK_DELETE);
		gstats_.record(BLOCK_DELETE);

		return NO_ERROR;

	    }

	    // Map in the block with the indicated bid and allocate and initialize a
	    // corresponding placeholder. NOTE once more that it is the user's onus
	    // to ensure that the bid requested corresponds to a valid block and so
	    // on; no checks made here to ensure that that is indeed the case. Main
	    // memory usage increases.
	    err get_block(BIDT bid, void * &place) {

		err retval = NO_ERROR;
	    
		if ((retval = get_block_internals(bid, place)) != NO_ERROR) {
		    return retval;
		}
	    
		stats_.record(BLOCK_GET);
		gstats_.record(BLOCK_GET);

		return NO_ERROR;
	    }
	
	    // Unmap a currently mapped in block. NOTE once more that it is the user's
	    // onus to ensure that the bid is correct and so on; no checks made here
	    // to ensure that that is indeed the case. Main memory usage decreases.
	    err put_block(BIDT bid, void * place, char dirty = 1) {

		err retval = NO_ERROR;

		if ((retval = put_block_internals(bid, place, dirty)) != NO_ERROR) {	       
		    return retval;
		}
	    
		stats_.record(BLOCK_PUT);
		gstats_.record(BLOCK_PUT);

		return NO_ERROR;
	    }

	    // Synchronize the in-memory block with the on-disk block.
	    err sync_block(BIDT bid, void* place, char dirty = 1);

	protected:
	    err get_block_internals(BIDT bid, void *&place);
	    err put_block_internals(BIDT bid, void* place, char dirty);
	};


	template<class BIDT>
	err collection_mmap<BIDT>::get_block_internals(BIDT bid, void * &place) {

	    place = TPIE_OS_MMAP(NULL, header_.block_size,
				 read_only_ ? TPIE_OS_FLAG_PROT_READ : 
				 TPIE_OS_FLAG_PROT_READ | TPIE_OS_FLAG_PROT_WRITE, 
#ifdef SYSTYPE_BSD
				 MAP_FILE | MAP_VARIABLE | MAP_NOSYNC |
#endif
				 TPIE_OS_FLAG_MAP_SHARED, 
				 bcc_fd_, 
				 bid_to_file_offset(bid));
	
	    if (place == (void *)(-1)) {

		TP_LOG_FATAL_ID("mmap() failed to map in a block from file.");
		TP_LOG_FATAL_ID(strerror(errno));
	    
		return MEMORY_ERROR;
	    }

	    //  madvise(place, header_.block_size, MADV_RANDOM);
	
	    // Register the memory allocation since mmapped memory is
	    // not accounted for otherwise.
	    get_memory_manager().register_allocation(header_.block_size);
	
	    in_memory_blocks_++;
	
	    return NO_ERROR;
	}


	template<class BIDT>
	err collection_mmap<BIDT>::put_block_internals(BIDT bid, void* place, char /* dirty */) {
	
	    // The dirty parameter is not used in this implemetation.
	
	    if ((bid <= 0) || (bid >= header_.last_block)) {

		TP_LOG_FATAL_ID("Incorrect block ID in placeholder.");

		return INVALID_PLACEHOLDER;
	    }
	
#if (COLLECTION_MMAP_LAZY_WRITE < 2)
	    if (!read_only_) {
		if (TPIE_OS_MSYNC((char*)place, header_.block_size, 
#  if (COLLECTION_MMAP_LAZY_WRITE == 1)
				  TPIE_OS_FLAG_MS_ASYNC
#  else
				  TPIE_OS_FLAG_MS_SYNC
#  endif
			) == -1) {

		    TP_LOG_FATAL_ID("Failed to msync() block to file.");
		    TP_LOG_FATAL_ID(strerror(errno));

		    return IO_ERROR;
		}    
	    }
#endif
	
	    if (TPIE_OS_MUNMAP((char*)place, header_.block_size) == -1) {
	    
		TP_LOG_FATAL_ID("Failed to unmap() block of file.");
		TP_LOG_FATAL_ID(strerror(errno));
	    
		return IO_ERROR;
	    }

		get_memory_manager().register_allocation(header_.block_size);
	
	    in_memory_blocks_--;
	
	    return NO_ERROR;
	}
    

	template<class BIDT>
	err collection_mmap<BIDT>::sync_block(BIDT bid, void* place, char) {
	
	    if ((bid <= 0) || (bid >= header_.last_block)) {
	    
		TP_LOG_FATAL_ID("Incorrect block ID in placeholder.");
	    
		return INVALID_PLACEHOLDER;
	    }
	
	    if (!read_only_) {
		if (TPIE_OS_MSYNC((char*)place, header_.block_size, TPIE_OS_FLAG_MS_SYNC)) {

		    TP_LOG_FATAL_ID("Failed to msync() block to file.");
		    TP_LOG_FATAL_ID(strerror(errno));
		
		    return IO_ERROR;
		}
	    }
	
	    stats_.record(BLOCK_SYNC);
	    gstats_.record(BLOCK_SYNC);

	    return NO_ERROR;
	}

    }  //  bte namespace
 
}  //  tpie namespace

#endif //_TPIE_BTE_COLL_MMAP_H
