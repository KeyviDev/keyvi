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

// BTE streams with blocks I/Oed using read()/write().  This particular
// implementation explicitly manages blocks, and only ever maps in one
// block at a time.  This relies on the filesystem to do lookahead. It
// is assumed for the purpose of memory calculations that for each
// block used by TPIE, the filesystem uses up another block of the
// same size.
//
// Completely different from the old bte/ufs.h since this does
// blocking like bte/mmb, only it uses read()/write() to do so.

#ifndef _TPIE_BTE_STREAM_UFS_H
#define _TPIE_BTE_STREAM_UFS_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/array.h>
// Get error definitions
#include <tpie/bte/err.h>

// For memcpy
#include <cstring>

// For header's type field (85 == 'U').
#define STREAM_IMPLEMENTATION_UFS 85

//the code for double buffering is not here..
#define UFS_DOUBLE_BUFFER 0

#if STREAM_UFS_READ_AHEAD	
#  if !UFS_DOUBLE_BUFFER
#    error STREAM_UFS_READ_AHEAD requested, but no double buff mechanism in config.
#  endif
#  define STREAM_UFS_MMo_BUFFERS 2
#else
#  define STREAM_UFS_MM_BUFFERS 1
#endif

#if UFS_DOUBLE_BUFFER
#  error At present explicit DOUBLE BUFFER not supported.
#endif

// This code makes assertions and logs errors.
#include <tpie/tpie_assert.h>
#include <tpie/tpie_log.h>

// Get the stream_base class and related definitions.
#include <tpie/bte/stream_base.h>

// Define a sensible logical block factor, if not already defined.
#ifndef STREAM_UFS_BLOCK_FACTOR
#  ifdef BTE_STREAM_UFS_BLOCK_FACTOR
#    error The deprecated BTE_STREAM_UFS_BLOCK_FACTOR is defined, please use STREAM_UFS_BLOCK_FACTOR instead
#  endif
#  define STREAM_UFS_BLOCK_FACTOR 8
#endif

namespace tpie {
    
    namespace bte {
    
// This is a class template for the implementation of a 
// BTE stream of objects of type T such that the entire stream 
// resides on a single disk.  This version maps in only one
// block of the file at a time. The striped_stream class, such
// that it is comprised of several single disk streams, has  
// a member function that is a friend of this class.
	template <class T> 
	class stream_ufs: public stream_base<T, stream_ufs<T> > {
	public:
		typedef stream_base<T, stream_ufs<T> > base_t;
	    // CHECK THIS: Is this needed anymore?
// These are for gcc-3.4 compatibility
	protected:
	
	    using base_t::current_streams;
	    using base_t::m_substreamLevel;
	    using base_t::m_status;
	    using base_t::m_persistenceStatus;
	    using base_t::m_readOnly;
	    using base_t::m_path;
	    using base_t::m_osBlockSize;
	    using base_t::m_fileOffset;
	    using base_t::m_logicalBeginOfStream;
	    using base_t::m_logicalEndOfStream;
	    using base_t::m_fileLength;
	    using base_t::m_osErrno;
	    using base_t::m_header;
	
	    using base_t::check_header;
	    using base_t::init_header;
	    using base_t::register_memory_allocation;
	    using base_t::register_memory_deallocation;
	    using base_t::record_statistics;
	
	public:
	    using base_t::name;
	    using base_t::os_block_size;
// End: These are for gcc-3.4 compatibility

	public:
	    // Constructor.
	    // [tavi 01/09/02] Careful with the lbf (logical block factor)
	    // parameter. I introduced it in order to avoid errors when reading
	    // a stream having a different block factor from the default, but
	    // this make cause errors in applications. For example, the
	    // AMI_partition_and merge computes memory requirements of temporary
	    // streams based on the memory usage of the INPUT stream, However,
	    // the input stream may have different block size from the temporary
	    // streams created later. Until these issues are addressed, the
	    // usage of lbf is discouraged.
	    stream_ufs(const std::string& dev_path, 
		       stream_type    st,
		       TPIE_OS_SIZE_T lbf = STREAM_UFS_BLOCK_FACTOR);
	
	    // A substream constructor.
	    stream_ufs(stream_ufs     *super_stream,
		       stream_type    st, 
		       TPIE_OS_OFFSET sub_begin, 
		       TPIE_OS_OFFSET sub_end);
	
	    // A psuedo-constructor for substreams.
	    err new_substream(stream_type    st, 
			      TPIE_OS_OFFSET sub_begin,
			      TPIE_OS_OFFSET sub_end,
			      base_t **sub_stream);
    
	    // Destructor
	    ~stream_ufs();
	
	    inline err read_item(T ** elt);
	    inline err write_item(const T & elt);
	
	    // Move to a specific position in the stream.
	    err seek(TPIE_OS_OFFSET offset);
	
	    // Truncate the stream.
	    err truncate(TPIE_OS_OFFSET offset);

	    // Return the number of items in the stream.
	    inline TPIE_OS_OFFSET stream_len() const;
	
	    // Return the current position in the stream.
	    inline TPIE_OS_OFFSET tell() const;
	

	    // Query memory usage
	    err main_memory_usage(TPIE_OS_SIZE_T  *usage,
							  stream_usage usage_type);
	
	    TPIE_OS_SIZE_T chunk_size() const;
	
	private:
	
	    // Prohibit these.
	    stream_ufs(const stream_ufs<T>& other);
	    stream_ufs<T>& operator=(const stream_ufs<T>& other);
	
	    stream_header* map_header ();
	
	    inline err validate_current ();
	    inline err invalidate_current ();
	
	    err map_current ();
	    err unmap_current ();
	
	    inline err advance_current ();
	
	    inline TPIE_OS_OFFSET item_off_to_file_off (TPIE_OS_OFFSET itemOffset) const;
	    inline TPIE_OS_OFFSET file_off_to_item_off (TPIE_OS_OFFSET fileOffset) const;
	
	    // Descriptor of the mapped file.
	    TPIE_OS_FILE_DESCRIPTOR m_fileDescriptor;
	
	    bool m_itemsAlignedWithBlock;
	
	    // [tavi 01/27/02]
	    // This is the position in the file where the pointer is. We can
	    // save some lseek() calls by maintaining this.
	    TPIE_OS_OFFSET m_filePointer;
	
	    // The current item (mapped in)
	    T *m_currentItem;
	
	    // A pointer to the beginning of the currently mapped block.
		tpie::array<T> m_currentBlock;
	
	    // True if current points to a valid, mapped in block.
	    bool m_blockValid;
	
	    // If m_blockValid is true, then m_blockDirty is true if and only if
	    // mapped block is dirty; obviously m_blockDirty is always false for
	    // m_readOnly streams.
	    bool m_blockDirty;
	
	    // When m_blockValid is true, this is the Offset of m_currentBlock in the
	    // underlying Unix file.
	    TPIE_OS_OFFSET m_currentBlockFileOffset;	
	
	    TPIE_OS_SIZE_T m_itemsPerBlock;
	
	
#if UFS_DOUBLE_BUFFER
	    // for use in double buffering, when one is implemented using
	    // the aio interface.
		tpie::array<T, false> next_block;
	    TPIE_OS_OFFSET f_next_block;		// position of next block
	    int            have_next_block;		// is next block mapped?
	
#endif	/* UFS_DOUBLE_BUFFER */
	
#if STREAM_UFS_READ_AHEAD
	    // Read ahead into the next logical block.
	    void read_ahead ();
#endif
	
	
	};
    
// This constructor creates a stream whose contents are taken from the
// file whose path is given.
	template <class T>
	stream_ufs<T>::stream_ufs (const std::string& dev_path,
				   stream_type    st,
				   TPIE_OS_SIZE_T lbf) : 
	    m_fileDescriptor(),
	    m_itemsAlignedWithBlock(false),
	    m_filePointer(0),
	    m_currentItem(NULL),
	    m_blockValid(false),
	    m_blockDirty(false),
	    m_currentBlockFileOffset(0), 
	    m_itemsPerBlock(0) {
	
	    // Check if we have available streams. Don't decrease the number
	    // yet, since we may encounter an error.
	    if (stream_base_generic::available_streams() == 0)
		{
			m_status = STREAM_STATUS_INVALID;
	    
			TP_LOG_FATAL_ID ("BTE internal error: cannot open more streams.");
	    
			return;
	    }
	
	    m_path = dev_path;
	
	    // Cache the OS block size.
	    m_osBlockSize = os_block_size();
	
	    // This is a top level stream
	    m_substreamLevel = 0;
	
	    // Set stream status to default values.
	    m_persistenceStatus = PERSIST_PERSISTENT;
	    m_blockValid = false;
	    m_blockDirty = false;
	
	    // A field to remember the file offset of mapped in block.
	    m_currentBlockFileOffset = 0;
	    m_currentItem = NULL;
	    m_fileOffset = m_logicalBeginOfStream = m_osBlockSize;
	
	    // To be on the safe side, set this to -1. It will be set to the
	    // right value by map_header(), below.
	    m_filePointer = -1; 
	
	    // Decrease the number of available streams.
	    current_streams++;
	
	    switch (st) {
	    case READ_STREAM:
	    
		m_readOnly = true;
	    
		// Open the file for reading.
		if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(m_fileDescriptor = TPIE_OS_OPEN_ORDONLY(m_path, TPIE_OS_FLAG_USE_MAPPING_FALSE))) {
		
		    m_status = STREAM_STATUS_INVALID;
		    m_osErrno = errno;
		
		    TP_LOG_FATAL_ID ("open() failed to open " << m_path);
		    TP_LOG_FATAL_ID (strerror (m_osErrno));
		    // [tavi 01/07/02] Commented this out. Just because the file is
		    // unreadable is no reason to crash.
		    //assert (0); 
		
		    return;
		}
	    
		m_header = map_header ();
	    
		if (check_header() < 0) {
		    m_status = STREAM_STATUS_INVALID;
		    return;
		}
	    
		// Some more checking, specific to this stream type.
		if (m_header->m_type != STREAM_IMPLEMENTATION_UFS) {
		    TP_LOG_WARNING_ID("Using UFS stream implem. on another type of stream.");
		    TP_LOG_WARNING_ID("Stream implementations may not be compatible.");
		}
	    
		if ((m_header->m_blockSize % m_osBlockSize != 0) || 
		    (m_header->m_blockSize == 0)) {
		
		    m_status = STREAM_STATUS_INVALID;
		
		    TP_LOG_FATAL_ID ("header: incorrect logical block size;");
		    TP_LOG_FATAL_ID ("expected multiple of OS block size.");
		
		    return;
		}
	    
		if (m_header->m_blockSize != STREAM_UFS_BLOCK_FACTOR * m_osBlockSize) {
		    TP_LOG_WARNING_ID("Stream has different block factor than the default.");
		    TP_LOG_WARNING_ID("This may cause problems in some existing applications.");
		}
	    
		m_itemsPerBlock         =  m_header->m_blockSize / sizeof (T);
		m_itemsAlignedWithBlock = (m_header->m_blockSize % sizeof (T) == 0);
	    
		// Set the eos marker appropriately.
		m_logicalEndOfStream = item_off_to_file_off (m_header->m_itemLogicalEOF);
	    
		if (m_header->m_itemLogicalEOF >= 1) {
		    if (m_logicalEndOfStream - 
			item_off_to_file_off (m_header->m_itemLogicalEOF - 1) -
			sizeof (T) > 0) {
			// Meaning, 1. sizeof (T) does not divide the logical
			// blocksize. 2. the last item in the stream is the last
			// item that could have been placed on its logical block
			// (so that the valid file offset as far as TPIE goes, is
			// the beginning of a new block and so strictly greater
			// than the byte offset at which the last item ends). In
			// this situation, after reading the last item and
			// m_fileOffset gets incremented, it is strictly less than 
			// m_logicalEndOfStream; as a result the check 
			// (m_logicalEndOfStream <= m_fileOffset)? in ::read_item()
			// gets beaten when it shouldn't.  To remedy, we simply
			// reset m_logicalEndOfStream in this circumstance to be just 
			// past the last item's byte offset.
		    
			m_logicalEndOfStream = 
			    item_off_to_file_off (m_header->m_itemLogicalEOF - 1) +
			    sizeof (T);
		    }
		}
		break;
	    
	    case WRITE_STREAM:
	    case WRITEONLY_STREAM:
	    case APPEND_STREAM:
	
		m_readOnly = false;
	    
		// Open the file for writing.  First we will try to open 
		// is with the O_EXCL flag set.  This will fail if the file
		// already exists.  If this is the case, we will call open()
		// again without it and read in the header block.
		if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(m_fileDescriptor = TPIE_OS_OPEN_OEXCL(m_path, TPIE_OS_FLAG_USE_MAPPING_FALSE))) {
		
		    // Try again, hoping the file already exists.
		  if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(m_fileDescriptor = TPIE_OS_OPEN_ORDWR(m_path, TPIE_OS_FLAG_USE_MAPPING_FALSE))) {
		    
			m_status = STREAM_STATUS_INVALID;
			m_osErrno = errno;
		    
			TP_LOG_FATAL_ID ("open() failed to open " << m_path);
			TP_LOG_FATAL_ID (strerror (m_osErrno));
		    
			return;
		    }
		
		    // The file already exists, so read the header.
		    m_header = map_header ();
		
		    if (check_header() < 0) {
		    
			m_status = STREAM_STATUS_INVALID;
		    
			return;
		    }
		
		    // Some more checking, specific to this stream.
		    if (m_header->m_type != STREAM_IMPLEMENTATION_UFS) {
			TP_LOG_WARNING_ID("Using UFS stream implem. on another type of stream.");
			TP_LOG_WARNING_ID("Stream implementations may not be compatible.");
		    }
		
		    if ((m_header->m_blockSize % m_osBlockSize != 0) || 
			(m_header->m_blockSize == 0)) {
		    
			m_status = STREAM_STATUS_INVALID;
		    
			TP_LOG_FATAL_ID("Header: incorrect logical block size;");
			TP_LOG_FATAL_ID("Expected multiple of OS block size.");
		    
			return;
		    }
		
		    if (m_header->m_blockSize != 
			STREAM_UFS_BLOCK_FACTOR * m_osBlockSize) {
			TP_LOG_WARNING_ID("Stream has different block factor than the default;");
			TP_LOG_WARNING_ID("\tStream block factor: " << static_cast<TPIE_OS_LONGLONG>(m_header->m_blockSize/m_osBlockSize));
			TP_LOG_WARNING_ID("\tDefault block factor: " << STREAM_UFS_BLOCK_FACTOR);
			TP_LOG_WARNING_ID("This may cause problems in some existing applications.");
		    }
		
		    m_itemsPerBlock         =  m_header->m_blockSize / sizeof (T);
		    m_itemsAlignedWithBlock = (m_header->m_blockSize % sizeof (T) == 0);
		
		    m_logicalEndOfStream = 
			item_off_to_file_off (m_header->m_itemLogicalEOF);
		
		    if (m_header->m_itemLogicalEOF >= 1) {
			if (m_logicalEndOfStream - 
			    item_off_to_file_off (m_header->m_itemLogicalEOF - 1) -
			    sizeof (T) > 0) {
			    // Meaning, 1. sizeof (T) does not divide the logical
			    // blocksize. 2. the last item in the stream is the last
			    // item that could have been placed on its logical block
			    // (so that the valid file offset as far as TPIE goes,
			    // is the beginning of a new block and so strictly
			    // greater than the byte offset at which the last item
			    // ends). In this situation, after reading the last
			    // item and m_fileOffset gets incremented, it is strictly
			    // less than m_logicalEndOfStream; as a result the check 
			    // (m_logicalEndOfStream <= m_fileOffset)? in ::read_item() 
			    // gets beaten when it shouldn't. To remedy, we simply 
			    // reset m_logicalEndOfStream in this circumstance to be 
			    // just past the last item's byte offset.
			    m_logicalEndOfStream =  
				item_off_to_file_off (m_header->m_itemLogicalEOF - 1) +
				sizeof (T);
			}
		    }
		
		    if (st == APPEND_STREAM) {
			m_fileOffset = m_logicalEndOfStream;
		    }
		} 
		else {	// The file was just created.
		
		    // Create and map in the header. File does not exist, so
		    // first establish a mapping and then write into the file via
		    // the mapping.
		    m_header = map_header ();
		
		    if (m_header == NULL) {

			m_status = STREAM_STATUS_INVALID;

			return;
		    }
		
		    init_header();
		
		    if (lbf == 0) {
			lbf = 1;
			TP_LOG_WARNING_ID("Block factor 0 requested. Using 1 instead.");
		    }
		
		    // Set the logical block size.
		    m_header->m_blockSize = lbf * m_osBlockSize;
		
		    // Set the type.
		    m_header->m_type = STREAM_IMPLEMENTATION_UFS;
		
		    m_itemsPerBlock         =  m_header->m_blockSize / sizeof (T);
		    m_itemsAlignedWithBlock = (m_header->m_blockSize % sizeof (T) == 0);
		
		    m_logicalEndOfStream = m_osBlockSize;
		
		    record_statistics(STREAM_CREATE);
		}
	    
		break;
	    }				// end of switch
	
	    // We can't handle streams of large objects.
	    if (static_cast<TPIE_OS_SSIZE_T>(sizeof (T)) > m_header->m_blockSize) {
	    
		m_status = STREAM_STATUS_INVALID;
	    
		TP_LOG_FATAL_ID ("Object is too big (object size/block size):");
		TP_LOG_FATAL_ID (sizeof(T));
		TP_LOG_FATAL_ID (static_cast<TPIE_OS_LONGLONG>(m_header->m_blockSize));
	    
		return;
	    }
	
#if UFS_DOUBLE_BUFFER
	    f_next_block    = 0;
	    have_next_block = 0;
#endif
	
	
	    // Memory-usage for the object, base class, header and the stream buffers
	    // are registered automatically by Darren's modified new() function.
	    m_fileLength  = TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_END);
	    m_filePointer = m_fileLength;
	
	    record_statistics(STREAM_OPEN);
	}
    
    
// A substream constructor.
// sub_begin is the item offset of the first item in the stream.
// sub_end is the item offset that of the last item in the stream.
// Thus, m_logicalEndOfStream in the new substream will be set to point one item beyond
// this.
//
// For example, if a stream contains [A,B,C,D,...] then substream(1,3)
// will contain [B,C,D].
	template <class T>
	stream_ufs<T>::stream_ufs (stream_ufs     *super_stream,
				   stream_type    st,
				   TPIE_OS_OFFSET sub_begin, 
				   TPIE_OS_OFFSET sub_end) :
	    m_fileDescriptor(),
	    m_itemsAlignedWithBlock(false),
	    m_filePointer(0),
	    m_currentItem(NULL),
	    m_blockValid(false),
	    m_blockDirty(false),
	    m_currentBlockFileOffset(0), 
	    m_itemsPerBlock(0) {
	
	    // Reduce the number of streams avaialble.
	    if (stream_base_generic::available_streams() <= 0) {
	    
		m_status = STREAM_STATUS_INVALID;
	
		TP_LOG_FATAL_ID ("BTE error: cannot open more streams.");
	
		return;
	    }
	
	    if (super_stream->status() == STREAM_STATUS_INVALID) {
	    
		m_status = STREAM_STATUS_INVALID;
	    
		TP_LOG_FATAL_ID ("BTE error: super stream is invalid.");
	    
		return;
	    }
	
	    if (super_stream->read_only() && (st != READ_STREAM)) {
	    
		m_status = STREAM_STATUS_INVALID;
	    
		TP_LOG_FATAL_ID ("BTE error: super stream is read only and substream is not.");
	    
		return;
	    }
	
	    // If you are going to access a substream of a previously created
	    // (super)stream we want to make sure that the superstream 's
	    // currently valid block, if any, is committed to the underlying
	    // Unix file. Note that with memory mapped implementation such a
	    // "committing" is automatic but in our case we need to keep track
	    // of such things.
	    if (!super_stream->read_only() && super_stream->m_blockValid) {
	    
		super_stream->unmap_current ();
	    
		if (super_stream->status() == STREAM_STATUS_INVALID) {
		
		    m_status = STREAM_STATUS_INVALID;
		
		    TP_LOG_FATAL_ID ("BTE internal error: super stream is invalid.");
		
		    return;
		}
	    }
	
		current_streams++;
	
	    // Copy the relevant fields from the super_stream.
	
	    m_path = super_stream->m_path;
	
	    m_readOnly              = super_stream->read_only();
	    m_osBlockSize           = super_stream->m_osBlockSize;
	    m_itemsPerBlock         = super_stream->m_itemsPerBlock;
	    m_itemsAlignedWithBlock = super_stream->m_itemsAlignedWithBlock;
	    m_header                = super_stream->m_header;
	
	    m_substreamLevel = super_stream->m_substreamLevel + 1;
	
	    // Each substream should have a local file descriptor 
	    // so m_filePointer and m_fileDescriptor position match
	    // Only READ and WRITE streams allowed
	    switch(st){
	    case READ_STREAM:
	      m_fileDescriptor=TPIE_OS_OPEN_ORDONLY(m_path, TPIE_OS_FLAG_USE_MAPPING_FALSE);
		break;
	    
	    case WRITE_STREAM:
		//file better exist if super_stream exists
	      m_fileDescriptor=TPIE_OS_OPEN_ORDWR(m_path, TPIE_OS_FLAG_USE_MAPPING_FALSE);
		break;
	    
	    default:
		m_status = STREAM_STATUS_INVALID;
	    
		TP_LOG_FATAL_ID ("BTE internal error: Invalid subtream type.");
	    
		return;
	    }
	
	    if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(m_fileDescriptor)) {
	    
		m_status = STREAM_STATUS_INVALID;
		m_osErrno = errno;
	    
		TP_LOG_FATAL_ID ("open() failed to open " << m_path);
		TP_LOG_FATAL_ID (strerror (m_osErrno));
		assert(0);
	    
		return;
	    }
	
	    m_persistenceStatus = PERSIST_PERSISTENT;
	
	    // The arguments sub_start and sub_end are logical item positions
	    // within the stream.  We need to convert them to offsets within
	    // the stream where items are found.
	
	    TPIE_OS_OFFSET super_item_begin = 
		file_off_to_item_off (super_stream->m_logicalBeginOfStream);
	
	    m_logicalBeginOfStream = item_off_to_file_off(super_item_begin + sub_begin);
	    m_logicalEndOfStream   = item_off_to_file_off(super_item_begin + sub_end + 1);
	
	    // sanity check
	    tp_assert (m_logicalBeginOfStream <= m_logicalEndOfStream, "bos beyond eos");
	
	    if (super_item_begin + sub_end + 1 >= 1) {
		if (m_logicalEndOfStream - 
		    item_off_to_file_off (super_item_begin + sub_end) -
		    sizeof (T) > 0) {
		    // Meaning, 1. sizeof (T) does not divide the logical
		    // blocksize. 2. the last item in the stream is the last item
		    // that could have been placed on its logical block (so that
		    // the valid file offset as far as TPIE goes, is the beginning
		    // of a new block and so strictly greater than the byte offset
		    // at which the last item ends.)  In this situation, after
		    // reading the last item and m_fileOffset gets incremented, it is
		    // strictly less than m_logicalEndOfStream; as a result the check 
		    // (m_logicalEndOfStream <= m_fileOffset)? in ::read_item() gets 
		    // beaten when it shouldn't. To remedy, we simply reset
		    // m_logicalEndOfStream in this circumstance to be
		    // just past the last item's byte offset.
		    m_logicalEndOfStream = 
			item_off_to_file_off (super_item_begin + sub_end) +
			sizeof (T);
		}
	    }
	
	    // sanity check
	    tp_assert (m_logicalBeginOfStream <= m_logicalEndOfStream, "bos beyond eos");
	
	    m_fileLength = super_stream->m_fileLength;
	
	    if (m_logicalEndOfStream > super_stream->m_logicalEndOfStream) {
	    
		m_status = STREAM_STATUS_INVALID;
	    
		TP_LOG_FATAL_ID ("BTE internal error: reached beyond super stream eof.");
	    
		return;
	    }
	
	    m_fileOffset   = m_logicalBeginOfStream;
	    m_filePointer  = -1; // I don't know where the file pointer is.
	    m_currentItem  = NULL;
	    m_blockValid   = false;
	    m_blockDirty   = false;
	    m_currentBlockFileOffset = 0;
	
#if UFS_DOUBLE_BUFFER
	    f_next_block    = 0;
	    have_next_block = 0;
#endif
	
	    record_statistics(STREAM_OPEN);
	    record_statistics(SUBSTREAM_CREATE);
	}
    
// A psuedo-constructor for substreams.  This serves as a wrapper for
// the constructor above in order to get around the fact that one
// cannot have virtual constructors.
	template <class T>
	err stream_ufs<T>::new_substream (stream_type    st,
					  TPIE_OS_OFFSET sub_begin,
					  TPIE_OS_OFFSET sub_end,
					  base_t **sub_stream) {
	    // Check permissions.
	    if ((st != READ_STREAM) && 
		((st != WRITE_STREAM) || m_readOnly)) {

		*sub_stream = NULL;

		return PERMISSION_DENIED;
	    }
	
	    tp_assert (((st == WRITE_STREAM) && !m_readOnly) ||
		       (st == READ_STREAM),
		       "Bad things got through the permisssion checks.");
	
	    stream_ufs<T> *sub =
			tpie_new<stream_ufs<T> >(this, st, sub_begin, sub_end);
	
	    *sub_stream = dynamic_cast<base_t *>(sub);
	
	    return NO_ERROR;
	}
    
	template <class T>
	stream_ufs<T>::~stream_ufs () {
	
	    // If the stream is already invalid for some reason, then don't
	    // worry about anything.
	    if (m_status == STREAM_STATUS_INVALID) {
	    
		TP_LOG_WARNING_ID ("BTE internal error: invalid stream in destructor.");
	    
		return;
	    }
		
		current_streams--;
	
	    record_statistics(STREAM_DELETE);
	
	    // If this is writable and not a substream, then put the logical
	    // eos back into the header before unmapping it.
	    if (!m_readOnly && !m_substreamLevel) {
		m_header->m_itemLogicalEOF = file_off_to_item_off (m_logicalEndOfStream);
	    }
	
	    // Unmap the current block if necessary.
	    if (m_blockValid) {
		unmap_current ();
	    }
	
	    // If this is not a substream then cleanup.
	    if (!m_substreamLevel) {
		// If a writeable stream, write back the header.  But only if
		// the stream is persistent. Otherwise, don't waste time with
		// the system calls.
		if (!m_readOnly && m_persistenceStatus != PERSIST_DELETE) {
		
		    if (TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_SET) != 0) {
		    
			m_status  = STREAM_STATUS_INVALID;
			m_osErrno = errno;
		    
			TP_LOG_FATAL_ID ("lseek() failed to move past header of " << m_path);
			TP_LOG_FATAL_ID (strerror (m_osErrno));
			// [tavi 01/07/02] Commented this out. Why panic?
			//assert (0);
			// TODO: Should we really return? If we do, we have memory leaks.
		    
			return;
		    }
		
		    if (TPIE_OS_WRITE (m_fileDescriptor, 
				       reinterpret_cast<char*>(m_header), 
				       sizeof (stream_header))
			!= sizeof (stream_header)) {
		    
			m_status = STREAM_STATUS_INVALID;
			m_osErrno = errno;
		    
			TP_LOG_FATAL_ID ("write() failed during stream destruction for "
					 << m_path);
			TP_LOG_FATAL_ID (strerror (m_osErrno));

			#ifdef TPIE_USE_EXCEPTIONS
			if (m_osErrno == ENOSPC) {
				throw out_of_space_error
					("Out of space writing to stream: " + m_path);
			}
			#endif 
			
			// [tavi 01/07/02] Commented this out. Why panic?
			//assert (0);
			// TODO: Should we really return? If we do, we have memory leaks.
			return;
		    }
		
		    // Invalidate the cached file pointer.
		    m_filePointer = -1;
		
		}
	    
		if (m_header) {
		    tpie_delete(m_header);
		}
	    
		if (TPIE_OS_CLOSE (m_fileDescriptor)) {
		
		    m_osErrno = errno;
		
		    TP_LOG_FATAL_ID ("Failed to close() " << m_path);
		    TP_LOG_FATAL_ID (strerror (m_osErrno));
		    // [tavi 01/07/02] Commented this out. Why panic?
		    //assert (0);
		
		    return;
		}
	    
		// If it should not persist, unlink the file.
		if (m_persistenceStatus == PERSIST_DELETE) {
		    if (m_readOnly) {
			TP_LOG_WARNING_ID("PERSIST_DELETE for read-only stream in " << m_path);
		    } 
		    else  {
				if (TPIE_OS_UNLINK (m_path)) {
			
			    m_osErrno = errno;
			
			    TP_LOG_WARNING_ID ("unlink failed during destruction of:");
			    TP_LOG_WARNING_ID (m_path);
			    TP_LOG_WARNING_ID (strerror (m_osErrno));

			}
			else {
			    record_statistics(STREAM_DELETE);
			}
		    }
		}
	    } 
	    else {				// end of if (!m_substreamLevel) 
		//Each substream has its own file descriptor so close it. 
		if (TPIE_OS_CLOSE (m_fileDescriptor)) {
		
		    m_osErrno = errno;
		
		    TP_LOG_FATAL_ID ("Failed to close() substream" << m_path);
		    TP_LOG_FATAL_ID (strerror (m_osErrno));
		
		    return;
		}
	    
		record_statistics(SUBSTREAM_DELETE);
	    }
	
	    if (!m_currentBlock.empty()) {
			m_currentBlock.resize(0);
	    
		// If you really want to be anal about memory calculation
		// consistency then if IMPLICIT_FS_READAHEAD flag is set you
		// should register a memory deallocation of header->block_size AT
		// THIS POINT of time in code.  At present, since we havent
		// registered allocation for these ``implicitly read-ahead''
		// blocks, we don't register the dealloc either.
	    }
	
#if UFS_DOUBLE_BUFFER
	    //Have to think this out since if UFS_DOUBLE_BUFFERING is implemented
	    //there is the possibility that the aio_read for the next block is
	    //ongoing at the time of the destruction, in which case trying to
	    //delete next_block may cause a run-time error. Most probably
	    // the aio read op may have to be suspended if ongoing. 
		next_block.resize(0);
#endif
	
	    record_statistics(STREAM_CLOSE);
	}
    
	template <class T>
	inline err stream_ufs<T>::read_item (T ** elt) {
	
	    // Make sure we are not currently at the EOS.
	    if (m_fileOffset >= m_logicalEndOfStream) {
			tp_assert (m_logicalEndOfStream == m_fileOffset, "Can't read past eos.");
			return END_OF_STREAM;
	    }

		err retval;
	    // Validate the current block.
	    if ((retval = validate_current ()) != NO_ERROR) {
			return retval;
	    }
	
	    // Check and make sure that the current pointer points into the
	    // current block.
	    tp_assert ((static_cast<unsigned int>(
			    reinterpret_cast<char*>(m_currentItem) - 
			    reinterpret_cast<char*>(m_currentBlock.get())) <=
			static_cast<unsigned int>(m_header->m_blockSize - sizeof (T))),
		       "m_currentItem is past the end of the current block");
	
	    tp_assert ((reinterpret_cast<char*>(m_currentItem) - 
					reinterpret_cast<char*>(m_currentBlock.get()) >= 0),
		       "m_currentItem is before the begining of the current block");
	
	    record_statistics(ITEM_READ);
	
	    // Read
	    *elt = m_currentItem;
	
	    // Advance the current pointer.
	    advance_current ();
	
	    // If we are in a substream, there should be no way for f_current
	    // to pass m_logicalEndOfStream.
	    tp_assert (!m_substreamLevel || (m_fileOffset <= m_logicalEndOfStream),
		       "Got past eos in a substream.");
	
	    return NO_ERROR;
	}

	template <class T>
	inline err stream_ufs<T>::write_item (const T & elt) {
	
	    // This better be a writable stream.
	    if (m_readOnly) {
		return READ_ONLY;
	    }
	
	    // Make sure we are not currently at the EOS of a substream.
	    if (m_substreamLevel && (m_logicalEndOfStream <= m_fileOffset)) {
	    
		tp_assert (m_logicalEndOfStream == m_fileOffset, "Went too far in a substream.");
	    
		return END_OF_STREAM;
	    
	    }
	
		err retval;
	    // Validate the current block.
	    if ((retval = validate_current ()) != NO_ERROR) {	   
		return retval;
	    }
	
	    // Check and make sure that the current pointer points into the
	    // current block.
	    tp_assert ((static_cast<unsigned int>(
			    reinterpret_cast<char*>(m_currentItem) - 
			    reinterpret_cast<char*>(m_currentBlock.get())) <=
			static_cast<unsigned int>(m_header->m_blockSize - sizeof (T))),
		       "m_currentItem is past the end of the current block");
	
	    tp_assert ((reinterpret_cast<char*>(m_currentItem) - 
					reinterpret_cast<char*>(m_currentBlock.get()) >= 0),
		       "m_currentItem is before the begining of the current block");
	
	    record_statistics(ITEM_WRITE);
	
	    // Write.
	    *m_currentItem = elt;
	    m_blockDirty   = true;
	
	    // Advance the current pointer.
	    advance_current ();
	
	    // If we are in a substream, there should be no way for f_current to
	    // pass m_logicalEndOfStream.
	    tp_assert (!m_substreamLevel || (m_fileOffset <= m_logicalEndOfStream),
		       "Got past eos in a substream.");
	
	    // If we moved past eos, then update eos unless we are in a
	    // substream, in which case EOS will be returned on the next call.
	    if ((m_fileOffset > m_logicalEndOfStream) && !m_substreamLevel) {
		// disable the assertion below because it is violated when
		// the end of a block is reached and the item size does not
		// divide the block size completely (so there is some space left)
		// tp_assert(m_fileOffset == m_logicalEndOfStream + sizeof(T), "Advanced too far somehow.");
		m_logicalEndOfStream = m_fileOffset;
	    }
	
	    return NO_ERROR;
	}
    
// Query memory usage
// Note that in a substream we do not charge for the memory used by
// the header, since it is accounted for in the 0 level superstream.
	template <class T>
	err stream_ufs<T>::main_memory_usage (TPIE_OS_SIZE_T  *usage,
										  stream_usage usage_type) {

	    switch (usage_type) {
			
	    case STREAM_USAGE_OVERHEAD:
		//sizeof(*this) includes base class. 
		//m_header is allocated dynamically, but always allocated, 
		//even for substreams. Don't forget space overhead per
		//"new" on (class, base class, m_header) 
			*usage = sizeof(*this) + sizeof(stream_header);
			break;
	    
	    case STREAM_USAGE_BUFFER: 
			//space used by buffers, when allocated
			*usage = STREAM_UFS_MM_BUFFERS * m_header->m_blockSize;
			break;
	    
	    case STREAM_USAGE_CURRENT:
			//overhead + buffers (if in use)
			*usage = sizeof(*this) +  sizeof(stream_header) +
				(m_currentBlock.empty() ? 0 : (STREAM_UFS_MM_BUFFERS *
											   m_header->m_blockSize ));
			break;
	
	    case STREAM_USAGE_MAXIMUM:
	    case STREAM_USAGE_SUBSTREAM:
			*usage = sizeof(*this) +  sizeof(stream_header) +
				STREAM_UFS_MM_BUFFERS * m_header->m_blockSize;
		break;

	    }
	
	    return NO_ERROR;
	}
    
// Return the number of items in the stream.
	template <class T> 
	TPIE_OS_OFFSET stream_ufs<T>::stream_len () const {
	    return file_off_to_item_off (m_logicalEndOfStream) - 
		file_off_to_item_off (m_logicalBeginOfStream);
	};
    
// Move to a specific position.
	template <class T> 
	err stream_ufs<T>::seek (TPIE_OS_OFFSET offset) {
	
	    err retval = NO_ERROR;
	    TPIE_OS_OFFSET new_offset;
	
	    if ((offset < 0) ||
		(offset > file_off_to_item_off (m_logicalEndOfStream) - 
		 file_off_to_item_off (m_logicalBeginOfStream))) {
	    
		TP_LOG_WARNING_ID ("seek() out of range (off/bos/eos)");
		TP_LOG_WARNING_ID (offset);
		TP_LOG_WARNING_ID (file_off_to_item_off (m_logicalBeginOfStream));
		TP_LOG_WARNING_ID (file_off_to_item_off (m_logicalEndOfStream));
	    
		return OFFSET_OUT_OF_RANGE;
	    }
	
	    // Compute the new offset.
	    new_offset = item_off_to_file_off (
		file_off_to_item_off (m_logicalBeginOfStream) + offset);
	
	    if ((static_cast<TPIE_OS_SIZE_T>(reinterpret_cast<char*>(m_currentItem) - 
										 reinterpret_cast<char*>(m_currentBlock.get())) 
		 >= m_header->m_blockSize)
		|| (((new_offset - m_osBlockSize) / m_header->m_blockSize) !=
		    ((m_fileOffset - m_osBlockSize) / m_header->m_blockSize))) {
		if (m_blockValid && ((retval = unmap_current ()) != NO_ERROR)) {
		    return retval;
		}
	    } 
	    else {
		if (m_blockValid) {
		
		    // We have to adjust current.
		    register TPIE_OS_OFFSET internal_block_offset;
		
		    internal_block_offset = 
			file_off_to_item_off (new_offset) % m_itemsPerBlock;
		
		    m_currentItem = m_currentBlock.get() + internal_block_offset;
		}
	    }
	
	    m_fileOffset = new_offset;
	
	    record_statistics(ITEM_SEEK);
	
	    return NO_ERROR;
	}

	template <class T> 
	TPIE_OS_OFFSET stream_ufs<T>::tell() const {
	    return file_off_to_item_off(m_fileOffset);
	}

// Truncate the stream.
	template <class T> 
	err stream_ufs<T>::truncate (TPIE_OS_OFFSET offset) {
	
	    err retval = NO_ERROR;
	    TPIE_OS_OFFSET new_offset;
	    TPIE_OS_OFFSET block_offset;
	
	    // Sorry, we can't truncate a substream.
	    if (m_substreamLevel) {
		return STREAM_IS_SUBSTREAM;
	    }
	
	    if (offset < 0) {
		return OFFSET_OUT_OF_RANGE;
	    }
	
	    // Compute the new offset
	    new_offset = item_off_to_file_off (
		file_off_to_item_off (m_logicalBeginOfStream) + offset);
	
	    // If it is not in the same block as the current position then
	    // invalidate the current block.
	    // We also need to check that we have the correct block mapped in (
	    // m_fileOffset does not always point into the current block!) 
	    // - see comment in seek()
	    if ((static_cast<TPIE_OS_SIZE_T>(reinterpret_cast<char*>(m_currentItem) - 
										 reinterpret_cast<char*>(m_currentBlock.get())) 
		 >= m_header->m_blockSize)
		|| (((new_offset - m_osBlockSize) / m_header->m_blockSize) !=
		    ((m_fileOffset - m_osBlockSize) / m_header->m_blockSize))) {
		if (m_blockValid && ((retval = unmap_current ()) != NO_ERROR)) {

		    return retval;

		}
	    }
	
	    // If it is not in the same block as the current end of stream
	    // then truncate the file to the end of the new last block.
	    if (((new_offset - m_osBlockSize) / m_header->m_blockSize) !=
		((m_logicalEndOfStream - m_osBlockSize) / m_header->m_blockSize)) {
	    
		// Determine the offset of the block that new_offset is in.
		block_offset = ((new_offset - m_osBlockSize) / m_header->m_blockSize)
		    * m_header->m_blockSize + m_osBlockSize;
		m_fileLength = block_offset + m_header->m_blockSize;
	    
		if (TPIE_OS_FTRUNCATE (m_fileDescriptor, block_offset + m_header->m_blockSize)) {
		
		    m_osErrno = errno;
		
		    TP_LOG_FATAL_ID ("Failed to ftruncate() to the new end of " << m_path);
		    TP_LOG_FATAL_ID (strerror (m_osErrno));
		
		    return OS_ERROR;
		}
	    
		// Invalidate the file pointer.
		m_filePointer = -1;
	    }
	
	    if (m_blockValid) {
		// This can happen if we didn't truncate much and stayed within 
		// the current block then the current block is still valid, 
		// but the current item pointer may not be valid. 
		// We have to adjust m_currentItem.
		TPIE_OS_OFFSET internal_block_offset = file_off_to_item_off (new_offset) % m_itemsPerBlock;
		m_currentItem = m_currentBlock.get() + internal_block_offset;
	    }
	
	    // Reset the current position to the end.    
	    m_fileOffset = m_logicalEndOfStream = new_offset;
	
	    return NO_ERROR;
	}
    
// Map in the header from the file.  This assumes that the path
// has been cached in path and that the file has been opened and
// m_fileDescriptor contains a valid descriptor.
	template <class T>
	stream_header * stream_ufs<T>::map_header () {
	
	    TPIE_OS_OFFSET file_end = TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_END);
	    stream_header *ptr_to_header = NULL;
	
	    // If the underlying file is not at least long enough to contain
	    // the header block, then, assuming the stream is writable, we have
	    // to create the space on disk by doing an explicit write().  
	    if (file_end  < static_cast<TPIE_OS_OFFSET>(m_osBlockSize)) {
 
		if (m_readOnly) {
		
		    m_status = STREAM_STATUS_INVALID;
		
		    TP_LOG_FATAL_ID ("No header block in read only stream " << m_path);
		
		    return NULL;
		} 
		else {	    
		    // A writable stream, but it doesn't have a header block,
		    // which means the file was just created and we have to leave
		    // space for the header block at the beginning of the fille.
		    // In this case we choose simply to allocate space for header
		    // fields and return a pointer to ufs_stream_header but first
		    // we write a dummy m_osBlockSize sized block at the
		    // beginning of the file.  This will trigger off sequential
		    // write optimizations that are useful unless non-sequential
		    // accesses to data are made.

			tpie::array<char> tmp_buffer(m_osBlockSize, 0);
		    if (file_end != 0) {
		    
			if (TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_SET) != 0) {
			
			    m_osErrno = errno;
			
			    TP_LOG_FATAL_ID ("Failed to lseek() in stream " << m_path);
			    TP_LOG_FATAL_ID (strerror (m_osErrno));
			
			    return NULL;
			}
		    }
		
		    if (TPIE_OS_WRITE (m_fileDescriptor, tmp_buffer.get(), m_osBlockSize) !=
			static_cast<TPIE_OS_SSIZE_T>(m_osBlockSize)) {
		    
			m_osErrno = errno;
		    
			TP_LOG_FATAL_ID ("Failed to write() in stream " << m_path);
			TP_LOG_FATAL_ID (strerror (m_osErrno));
		    
			#ifdef TPIE_USE_EXCEPTIONS
			if (m_osErrno == ENOSPC) {
				throw out_of_space_error
					("Out of space writing to stream: " + m_path);
			}
			#endif 


			return NULL;
		    }
	    
		    m_filePointer = m_osBlockSize;
		
		    ptr_to_header = tpie_new<stream_header>();
		    if (ptr_to_header != NULL) {
			return ptr_to_header;
		    } 
		    else {
		    
			m_status  = STREAM_STATUS_INVALID;
			m_osErrno = errno;
		    
			TP_LOG_FATAL_ID ("Failed to alloc space for header.");
			TP_LOG_FATAL_ID (strerror (m_osErrno));
		    
			return NULL;
		    }
		}
	    }
	
	    // Instead of mmap() we simply read in the m_osBlockSize leading
	    // bytes of the file, copy the leading sizeof(ufs_stream_header)
	    // bytes of the m_osBlockSize bytes into the ptr_to_header
	    // structure and return ptr_to_header. Note that even though we
	    // could have read only the first sizeof(ufs_stream_header) of the
	    // file we choose not to do so in order to avoid confusing
	    // sequential prefetcher.
		tpie::array<char> tmp_buffer(m_osBlockSize);
	
	    if (TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_SET) != 0) {
	    
		m_osErrno = errno;
	    
		TP_LOG_FATAL_ID ("Failed to lseek() in stream " << m_path);
		TP_LOG_FATAL_ID (strerror (m_osErrno));
	    
		return NULL;
	    }
	
	    if (TPIE_OS_READ (m_fileDescriptor, 
						  tmp_buffer.get(), 
			      m_osBlockSize) !=
		static_cast<TPIE_OS_SSIZE_T>(m_osBlockSize)) {
	    
		m_osErrno = errno;
	    
		TP_LOG_FATAL_ID ("Failed to read() in stream " << m_path);
		TP_LOG_FATAL_ID (strerror (m_osErrno));
	    
		return NULL;
	    }
	
	    m_filePointer = m_osBlockSize;
	
	    ptr_to_header = tpie_new<stream_header>();
	    std::memcpy(ptr_to_header, tmp_buffer.get(), sizeof(stream_header));
	
	    return ptr_to_header;
	}
    
//
// Make sure the current block is mapped in and all internal pointers are
// set as appropriate.  
//  
	template <class T>
	inline err stream_ufs<T>::validate_current () {
	
	    TPIE_OS_SIZE_T block_space;	// The space left in the current block.
	
	    // If the current block is valid and current points into it and has
	    // enough room in the block for a full item, we are fine.  If it is
	    // valid but there is not enough room, unmap it.
	    if (m_blockValid) {
		if ((block_space = m_header->m_blockSize -
		     (reinterpret_cast<char*>(m_currentItem) - 
		      reinterpret_cast<char*>(m_currentBlock.get()))) >= 
		    sizeof (T)) {
		
		    return NO_ERROR;
		
		} 
		else {			// Not enough room left.
			err retval = NO_ERROR;
		    if ((retval = unmap_current ()) != NO_ERROR) {
			return retval;
		    }
		    m_fileOffset += block_space;
		}
	    }

	    // The current block is invalid, since it was either invalid to start
	    // with or we just invalidated it because we were out of space.
	    tp_assert (!m_blockValid, "Block is already mapped in.");
	
	    // Now map in the block.
	    return map_current ();
	}
    
	template <class T>
	inline err stream_ufs<T>::invalidate_current () {

	    // We should currently have a valid block.
	    tp_assert (m_blockValid, "No block is mapped in.");
	
	    m_blockValid = false;
	
	    return NO_ERROR;
	}
    
// Map in the current block.
// m_fileOffset is used to determine what block is needed.
	template <class T> err stream_ufs<T>::map_current (void) {
	
	    TPIE_OS_OFFSET block_offset;
	    bool do_mmap = false;
	
	    // We should not currently have a valid block.
	    tp_assert (!m_blockValid, "Block is already mapped in.");
	
	    // Determine the offset of the block that the current item is in.
	    block_offset = ((m_fileOffset - m_osBlockSize) / m_header->m_blockSize)
		* m_header->m_blockSize + m_osBlockSize;
	
	    // If the block offset is beyond the logical end of the file, then
	    // we either record this fact and return (if the stream is read
	    // only) or ftruncate() out to the end of the current block.
	    if (m_fileLength < 
		block_offset + static_cast<TPIE_OS_OFFSET>(m_header->m_blockSize)) {
		if (m_readOnly) {

		    return END_OF_STREAM;

		} 
		else {
		
		    // An assumption here is that !m_readOnly implies that you won't try to read
		    // items beyond offset block_offset. This is justified because one invariant
		    // being maintained is that file length is m_osBlockSize + an INTEGRAL
		    // number of Logical Blocks: By this invariant, since lseek returns something 
		    //  smaller than block_offset + m_header->m_blockSize - 1 (meaning that filesize
		    // is smaller than block_offset + m_header->m_blockSize), 
		    //
		    // A consequence of this assumption is that the block being mapped in
		    // is being written/appended. Now while using mmapped I/O, what this 
		    // means is that we need to first ftruncate() and then map in the requisite
		    // block. On the other hand, if we are using the read()/write() BTE, we
		    // simply do nothing: the unmap_current() call executed during
		    // validate_current() and before map_current() would have ensured that
		    // we do not overwrite some previously mapped block. 
		
		    // Not mapped I/O  
		    // means we assume we are using the read()/write() BTE
		    // This means we do an unmap_current() in validate_current()
		    // just before  map_current() so there's no danger of overwriting
		    // a dirty block.  
		
		    if (m_currentBlock.empty()) {
				TPIE_OS_SIZE_T z = (sizeof(T)-1+m_header->m_blockSize)/sizeof(T);
				m_currentBlock.resize(z);
#ifndef NDEBUG
				memset(m_currentBlock.get(), 0, z);
#endif
		    
			// If you really want to be anal about memory calculation
			// consistency then if IMPLICIT_FS_READAHEAD flag is
			// set you should register a memory allocation of
			// m_header->m_blockSize AT THIS POINT of time in code.
		    }
		
		    m_currentBlockFileOffset = block_offset;
		    m_blockValid = true;
		    m_blockDirty = false;
		
		    register TPIE_OS_OFFSET internal_block_offset;
		
		    internal_block_offset =
			file_off_to_item_off (m_fileOffset) % m_itemsPerBlock;
		
		    m_currentItem = m_currentBlock.get() + internal_block_offset;
		
		    return NO_ERROR;
		
		}
	    }

	    // If the current block is already mapped in by this process then
	    // some systems, (e.g. HP-UX), will not allow us to map it in
	    // again.  This presents all kinds of problems, not only with
	    // sub/super-stream interactions, which we could probably detect
	    // by looking back up the path to the level 0 stream, but also
	    // with overlapping substreams, which are very hard to detect
	    // since the application can build them however it sees fit.  We
	    // can also have problems if we break a stream into two substreams
	    // such that their border is in the middle of a block, and then we
	    // read to the end of the fisrt substream while we are still at
	    // the beginning of the second.
	
#if UFS_DOUBLE_BUFFER
	    if (have_next_block && (block_offset == f_next_block)) {

		m_currentBlock.swap(next_block);
	    
		have_next_block = 0;
	    
	    } else {
		do_mmap = true;
	    }
#else
	    do_mmap = true;
#endif
	
	    if (do_mmap) {
	    
		if (m_filePointer == -1 || block_offset != m_filePointer) {
		    if (TPIE_OS_LSEEK(m_fileDescriptor, block_offset, TPIE_OS_FLAG_SEEK_SET) != block_offset) {
		    
			m_status = STREAM_STATUS_INVALID;
			m_osErrno = errno;
		    
			TP_LOG_FATAL_ID ("seek failed in file:");
			TP_LOG_FATAL_ID (m_path);
			TP_LOG_FATAL_ID(strerror(m_osErrno));
		    
			return OS_ERROR;
		    }
		}

		if (m_currentBlock.empty()) {
			TPIE_OS_SIZE_T z = (sizeof(T)-1+m_header->m_blockSize)/sizeof(T);
			m_currentBlock.resize(z);
#ifndef NDEBUG
			memset(m_currentBlock.get(), 0, z);
#endif
		    // If you really want to be anal about memory calculation
		    // consistency then if IMPLICIT_FS_READAHEAD flag is set
		    // you shd register a memory allocation of m_header->m_blockSize
		    // AT THIS POINT of time in code.
		}
	    
		if (TPIE_OS_READ (m_fileDescriptor, 
						  reinterpret_cast<char*>(m_currentBlock.get()), 
				  m_header->m_blockSize) !=
		    static_cast<TPIE_OS_SSIZE_T>(m_header->m_blockSize)) {
		
		    m_status = STREAM_STATUS_INVALID;
		    m_osErrno = errno;
		
		    TP_LOG_FATAL_ID ("read failed in file ");
		    TP_LOG_FATAL_ID(m_path);
		    TP_LOG_FATAL_ID(strerror(m_osErrno));
		
		    return OS_ERROR;
		}
	    
		// Advance file pointer.
		m_filePointer = block_offset + m_header->m_blockSize;
	    }
	
	
	    m_blockValid             = true;
	    m_currentBlockFileOffset = block_offset;
	    m_blockDirty             = false;
	
#if STREAM_UFS_READ_AHEAD
	    // Start the asyncronous read of the next logical block.
	    read_ahead ();
#endif
	
	    // The offset, in terms of number of items, that current should
	    // have relative to curr_block.
	    register TPIE_OS_OFFSET internal_block_offset;
	
	    internal_block_offset =
		file_off_to_item_off (m_fileOffset) % m_itemsPerBlock;
	
	    m_currentItem = m_currentBlock.get() + internal_block_offset;
	
	    record_statistics(BLOCK_READ);
	
	    return NO_ERROR;
	}
    
	template <class T> 
	err stream_ufs<T>::unmap_current (void) {
	
	    ///   TPIE_OS_OFFSET lseek_retval;
	
	    // We should currently have a valid block.
	    tp_assert (m_blockValid, "No block is mapped in.");
	
	    if (!m_readOnly && m_blockDirty) {
	    
		if (m_filePointer == -1 || m_currentBlockFileOffset != m_filePointer) {
		    if (TPIE_OS_LSEEK(m_fileDescriptor, m_currentBlockFileOffset, TPIE_OS_FLAG_SEEK_SET) !=
			m_currentBlockFileOffset) {
		    
			m_status  = STREAM_STATUS_INVALID;
			m_osErrno = errno;
		    
			TP_LOG_FATAL_ID ("lseek() failed while unmapping current block.");
			TP_LOG_FATAL_ID (strerror(m_osErrno));
		    
			return OS_ERROR;
		    }
		}
	    
		if (m_currentBlockFileOffset == m_fileLength) {
		    m_fileLength += m_header->m_blockSize;
		}

		if (TPIE_OS_SIZE_T(TPIE_OS_WRITE (m_fileDescriptor, 
										  reinterpret_cast<char*>(m_currentBlock.get()), 
						   m_header->m_blockSize)) != m_header->m_blockSize) {
		
		    m_status  = STREAM_STATUS_INVALID;
		    m_osErrno = errno;
		
		    TP_LOG_FATAL_ID ("write() failed to unmap current block.");
		    TP_LOG_FATAL_ID (static_cast<TPIE_OS_OFFSET>(m_header->m_blockSize));
		    TP_LOG_FATAL_ID (strerror(m_osErrno));

			#ifdef TPIE_USE_EXCEPTIONS
			if (m_osErrno == ENOSPC) {
				throw out_of_space_error
					("Out of space writing to stream: " + m_path);
			}
			#endif 
		
		    return OS_ERROR;
		}
	    
		// Advance file pointer.
		m_filePointer = m_currentBlockFileOffset + m_header->m_blockSize;
	    
	    }
	
	    m_blockDirty = false;
	    m_blockValid = false;
	    m_currentBlockFileOffset = 0;
	
	    record_statistics(BLOCK_WRITE);
	
	    return NO_ERROR;
	}
    
// A uniform method for advancing the current pointer.  No mapping,
// unmapping, or anything like that is done here.
	template <class T>
	inline err stream_ufs<T>::advance_current (void) {
	
	    // Advance the current pointer and the file offset of the current
	    // item.
	    m_currentItem++;
	    m_fileOffset += sizeof (T);
	
	    return NO_ERROR;
	}

	template <class T>
	inline TPIE_OS_OFFSET stream_ufs<T>::item_off_to_file_off (TPIE_OS_OFFSET item_off) const {
	    TPIE_OS_OFFSET file_off;
	
	    if (!m_itemsAlignedWithBlock) {
	    
		// Move past the header.  
		file_off = m_osBlockSize;
	    
		// Add m_header->m_blockSize for each full block.
		file_off += m_header->m_blockSize * (item_off / m_itemsPerBlock);
	    
		// Add sizeof(T) for each item in the partially full block.
		file_off += sizeof (T) * (item_off % m_itemsPerBlock);
	    
		return file_off;
	    
	    } else {
	    
		return (m_osBlockSize + item_off * sizeof (T));
	    
	    }
	}
    
	template <class T>
	inline TPIE_OS_OFFSET stream_ufs<T>::file_off_to_item_off (TPIE_OS_OFFSET file_off) const {
	    TPIE_OS_OFFSET item_off;
	
	    if (!m_itemsAlignedWithBlock) {
	    
		// Subtract off the header.
		file_off -= m_osBlockSize;
	    
		// Account for the full blocks.
		item_off = m_itemsPerBlock * (file_off / m_header->m_blockSize);
	    
		// Add in the number of items in the last block.
		item_off += (file_off % m_header->m_blockSize) / sizeof (T);
	    
		return item_off;
	    
	    } else {
	    
		return (file_off - m_osBlockSize) / sizeof (T);
	    
	    }
	}
    
	template <class T> 
	TPIE_OS_SIZE_T stream_ufs<T>::chunk_size (void) const {
	    return m_itemsPerBlock;
	}
    
    
#if STREAM_UFS_READ_AHEAD
	template <class T> void stream_ufs<T>::read_ahead (void) {

	    TPIE_OS_OFFSET f_curr_block;
	
	    // The current block had better already be valid or we made a
	    // mistake in being here.
	    tp_assert (m_blockValid,
		       "Trying to read ahead when current block is invalid.");
	
	    // Check whether there is a next block.  If we are already in the
	    // last block of the file then it makes no sense to read ahead.
	    f_curr_block = ((m_fileOffset - m_osBlockSize) / m_header->m_blockSize) *
		m_header->m_blockSize + m_osBlockSize;
	
	    if (m_logicalEndOfStream < f_curr_block + m_header->m_blockSize) {
		return;
	    }
	
	    f_next_block = f_curr_block + m_header->m_blockSize;
	
#if UFS_DOUBLE_BUFFER
#error Explicit double buffering not supported using read/write BTE
#endif
	}
    
#endif	/* STREAM_UFS_READ_AHEAD */
    
#undef STREAM_UFS_MM_BUFFERS

    } // bte namespace

}  // tpie namespace

#endif

