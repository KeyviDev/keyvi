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

// Memory mapped streams.  This particular implementation explicitly manages
// blocks, and only ever maps in one block at a time.
//
#ifndef _TPIE_BTE_STREAM_MMAP_H
#define _TPIE_BTE_STREAM_MMAP_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// For header's type field (77 == 'M').
#define STREAM_IMPLEMENTATION_MMAP 77

#include <tpie/tpie_assert.h>
#include <tpie/tpie_log.h>

#ifdef STREAM_MMAP_READ_AHEAD
#  define STREAM_MMAP_MM_BUFFERS 2
#else
#  define STREAM_MMAP_MM_BUFFERS 1
#endif

// Get the stream_base class and other definitions.
#include <tpie/bte/stream_base.h>

#ifndef  STREAM_MMAP_BLOCK_FACTOR
#  define STREAM_MMAP_BLOCK_FACTOR 8
#endif

#include<cstring> //for memcpy 

namespace tpie {

    namespace bte {

// Figure out the block offset for an offset (pos) in file.
// os_block_size_ is assumed to be the header size.
//#define BLOCK_OFFSET(pos) (((pos - os_block_size_) / header->block_size) * header->block_size + os_block_size_)
    
#define BLOCK_OFFSET(pos) (((pos - m_osBlockSize) / m_header->m_blockSize) * m_header->m_blockSize + m_osBlockSize)
    
//
// stream_mmap<T>
//
// This is a class template for the mmap() based implementation of a 
// BTE stream of objects of type T.  This version maps in only one
// block of the file at a time. 
//
		template <class T> 
		class stream_mmap: public stream_base<T, stream_mmap<T> > {
		public:
			typedef stream_base<T, stream_mmap<T> > base_t;
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
			// this may cause errors in applications. For example,
			// AMI_partition_and merge computes memory requirements of temporary
			// streams based on the memory usage of the INPUT stream. However,
			// the input stream may have different block size from the temporary
			// streams created later. Until these issues are addressed, the
			// usage of lbf is discouraged.
			stream_mmap(const std::string& dev_path, 
						stream_type    st, 
						TPIE_OS_SIZE_T lbf = STREAM_MMAP_BLOCK_FACTOR);
	
			// A substream constructor.
			stream_mmap(stream_mmap    *super_stream,
						stream_type    st, 
						TPIE_OS_OFFSET sub_begin, 
						TPIE_OS_OFFSET sub_end);
	
			// A psuedo-constructor for substreams.
			err new_substream(stream_type    st, 
							  TPIE_OS_OFFSET sub_begin,
							  TPIE_OS_OFFSET sub_end,
							  base_t **sub_stream);
	
			// Destructor
			~stream_mmap ();
	
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
			err main_memory_usage(TPIE_OS_SIZE_T* usage, 
								  stream_usage usage_type);
	
			TPIE_OS_OFFSET chunk_size() const;
	
	
			inline err grow_file (TPIE_OS_OFFSET block_offset);
	
		private:
	
#ifdef STREAM_MMAP_READ_AHEAD
			// Read ahead into the next logical block.
			void read_ahead ();
#endif
	
			void initialize ();
	
			stream_header *map_header ();
			void unmap_header ();
	
			inline err validate_current ();
			inline err invalidate_current ();
	
			err map_current ();
			err unmap_current ();

			inline err advance_current ();
	
			inline TPIE_OS_OFFSET item_off_to_file_off (TPIE_OS_OFFSET item_off) const;
			inline TPIE_OS_OFFSET file_off_to_item_off (TPIE_OS_OFFSET item_off) const;

#ifdef COLLECT_STATS
			long stats_hits;
			long stats_misses;
			long stats_compulsory;
			long stats_eos;
#endif
	
			unsigned int m_mmapStatus;
	
			// descriptor of the mapped file.
			TPIE_OS_FILE_DESCRIPTOR m_fileDescriptor;	
	
			// Pointer to the current item (mapped in).
			T *m_currentItem;
	
			// Pointer to beginning of the currently mapped block.
			T *m_currentBlock;
	
			// True if current points to a valid, mapped block.
			bool m_blockValid;
	
			// True if the m_currentBlock is mapped.
			bool m_blockMapped;
	
			// for use in double buffering
			T *m_nextBlock;		        // ptr to next block
			TPIE_OS_OFFSET m_nextBlockOffset;	// position of next block
			bool m_haveNextBlock;		// is next block mapped
			bool m_writeOnly;			// stream is write-only
	
	
#ifdef STREAM_MMAP_READ_AHEAD
			T* m_nextBlock;
#endif
	
		};
    

/* ********************************************************************** */
    
/* definitions start here */

#ifdef _WIN32
		// Suppress warning 4505 (removed non-referenced local function) once.
#pragma warning(disable : 4505)
#endif    

		static int call_munmap (void *addr, size_t len) {
			return TPIE_OS_MUNMAP (addr, len);
		}
    
		static void *call_mmap (void *addr, size_t len, 
								bool r_only, 
#ifdef MACH_ALPHA
								bool w_only,
#else
								bool /* w_only */,
#endif
								TPIE_OS_FILE_DESCRIPTOR fd, 
								TPIE_OS_OFFSET off, int fixed) {
			void *ptr;
			int flags = 0;
	
			assert (!fixed || addr);
	
#ifdef MACH_ALPHA
			// for enhanced mmap calls
#define MAP_OVERWRITE 0x1000	// block will be overwritten
			flags = (MAP_FILE |
					 (fixed ? TPIE_OS_FLAG_MAP_FIXED :MAP_VARIABLE) |
					 (w_only ? MAP_OVERWRITE : 0));
#else
			flags = (fixed ? TPIE_OS_FLAG_MAP_FIXED : 0);
#endif
			flags |= TPIE_OS_FLAG_MAP_SHARED;
	
			ptr = TPIE_OS_MMAP (addr, len,
								(r_only ? TPIE_OS_FLAG_PROT_READ : TPIE_OS_FLAG_PROT_READ | TPIE_OS_FLAG_PROT_WRITE),
								flags, fd, off);
			assert (ptr);

			return ptr;

		}

		template <class T> void stream_mmap<T>::initialize () {
#ifdef COLLECT_STATS
			stats_misses = stats_hits = stats_compulsory = stats_eos = 0;
#endif
			m_haveNextBlock = false;
			m_blockValid    = false;
			m_blockMapped   = false;
			m_fileOffset    = m_logicalBeginOfStream = m_osBlockSize;
			m_nextBlock     = m_currentBlock = m_currentItem = NULL;
		}
    
//
// This constructor creates a stream whose contents are taken from the
// file whose path is given.
//
		template <class T>
			stream_mmap<T>::stream_mmap (const std::string& dev_path, stream_type st, size_t lbf) {
			m_status = STREAM_STATUS_NO_STATUS;
	
			if (stream_base_generic::available_streams()) {
				m_status = STREAM_STATUS_INVALID;
				TP_LOG_FATAL_ID ("BTE internal error: cannot open more streams.");
				return;
			}
	
			m_path = dev_path;
			m_readOnly  = (st == READ_STREAM);
			m_writeOnly = (st == WRITEONLY_STREAM);	
			m_osBlockSize = os_block_size();
	
			// This is a top level stream
			m_substreamLevel = 0;

			// Reduce the number of streams available.
			current_streams++;
	
			switch (st) {
			case READ_STREAM:
				// Open the file for reading.
				if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(m_fileDescriptor = TPIE_OS_OPEN_ORDONLY(m_path, TPIE_OS_FLAG_USE_MAPPING_TRUE))) { 
		
					m_status  = STREAM_STATUS_INVALID;
					m_osErrno = errno;
		
					TP_LOG_FATAL ("open() failed to open \"");
					TP_LOG_FATAL (m_path);
					TP_LOG_FATAL ("\": ");
					TP_LOG_FATAL (strerror (m_osErrno));
					TP_LOG_FATAL ("\n");
					TP_LOG_FLUSH_LOG;
					// [tavi 01/07/02] Commented this out. No need to panic.
					//assert (0);
					return;
				}
	    
				// Get ready to read the first item out of the file.
				initialize ();
	    
				m_header = map_header ();
				if (check_header() < 0) {
		
					m_status = STREAM_STATUS_INVALID;
		
					// [tavi 01/07/02] Commented this out. No need to panic.
					//assert (0);
					return;
				}
	    
				if (m_header->m_type != STREAM_IMPLEMENTATION_MMAP) {
					TP_LOG_WARNING_ID("Using MMAP stream implem. on another type of stream.");
					TP_LOG_WARNING_ID("Stream implementations may not be compatible.");
				}
	    
				if ((m_header->m_blockSize % m_osBlockSize != 0) || 
					(m_header->m_blockSize == 0)) {
		
					m_status = STREAM_STATUS_INVALID;
		
					TP_LOG_FATAL_ID ("m_header: incorrect logical block size;");
					TP_LOG_FATAL_ID ("expected multiple of OS block size.");
		
					return;
				}
	    
				if (m_header->m_blockSize != STREAM_MMAP_BLOCK_FACTOR * m_osBlockSize) {
					TP_LOG_WARNING_ID("Stream has different block factor than the default.");
					TP_LOG_WARNING_ID("This may cause problems in some existing applications.");
				}
	    
				break;

			case WRITE_STREAM:
			case WRITEONLY_STREAM:
			case APPEND_STREAM:
	    
				// Open the file for writing.  First we will try to open 
				// is with the O_EXCL flag set.  This will fail if the file
				// already exists.  If this is the case, we will call open()
				// again without it and read in the header block.
				if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(m_fileDescriptor = TPIE_OS_OPEN_OEXCL(m_path, TPIE_OS_FLAG_USE_MAPPING_TRUE))) {
		
					// Try again, hoping the file already exists.
					if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(m_fileDescriptor = TPIE_OS_OPEN_ORDWR(m_path, TPIE_OS_FLAG_USE_MAPPING_TRUE))) {
		    
						m_status = STREAM_STATUS_INVALID;
						m_osErrno = errno;
		    
						TP_LOG_FATAL ("open() failed to open \"");
						TP_LOG_FATAL (m_path);
						TP_LOG_FATAL ("\": ");
						TP_LOG_FATAL (strerror (m_osErrno));
						TP_LOG_FATAL ("\n");
						TP_LOG_FLUSH_LOG;
		    
						return;
					}
		
					initialize ();
		
					// The file already exists, so read the header.
					m_header = map_header ();

					if (check_header () < 0) {
		    
						m_status = STREAM_STATUS_INVALID;
		    
						// [tavi 01/07/02] Commented this out. No need to panic.
						//assert (0);
						return;
					}
		
					if (m_header->m_type != STREAM_IMPLEMENTATION_MMAP) {
						TP_LOG_WARNING_ID("Using MMAP stream implem. on another type of stream.");
						TP_LOG_WARNING_ID("Stream implementations may not be compatible.");
					}
		
					if ((m_header->m_blockSize % m_osBlockSize != 0) || 
						(m_header->m_blockSize == 0)) {
		    
						m_status = STREAM_STATUS_INVALID;
		    
						TP_LOG_FATAL_ID ("m_header: incorrect logical block size;");
						TP_LOG_FATAL_ID ("expected multiple of OS block size.");
		    
						return;
					}
		
					if (m_header->m_blockSize != STREAM_MMAP_BLOCK_FACTOR * m_osBlockSize) {
						TP_LOG_WARNING_ID("Stream has different block factor than the default.");
						TP_LOG_WARNING_ID("This may cause problems in some existing applications.");
					}
				}
				else {	 // The file was just created.
		
					m_logicalEndOfStream = m_osBlockSize;
					// Rajiv
					// [tavi 01/07/02] Commented this out. Aren't we sure the file is OK?
					//assert (lseek (m_fileDescriptor, 0, SEEK_END) == 0);
				
					// what does this do??? Rajiv
					// Create and map in the header.
					if (TPIE_OS_LSEEK(m_fileDescriptor, m_osBlockSize - 1, TPIE_OS_FLAG_SEEK_SET) != static_cast<TPIE_OS_OFFSET>(m_osBlockSize - 1)) {
		    
						m_status = STREAM_STATUS_INVALID;
						m_osErrno = errno;
		    
						TP_LOG_FATAL ("lseek() failed to move past header of \"");
						TP_LOG_FATAL (m_path);
						TP_LOG_FATAL ("\": ");
						TP_LOG_FATAL (strerror (m_osErrno));
						TP_LOG_FATAL ("\n");
						TP_LOG_FLUSH_LOG;
						// [tavi 01/07/02] Commented this out. No need to panic.
						//assert (0 == 1);
		    
						return;
					}
		
					initialize ();
		
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
					m_header->m_type = STREAM_IMPLEMENTATION_MMAP;
		
					record_statistics(STREAM_CREATE);
		
				}
				break;
			}
	
			// We can't handle streams of large objects.
			if (sizeof (T) > m_header->m_blockSize) {
	    
				m_status = STREAM_STATUS_INVALID;
	    
				TP_LOG_FATAL_ID ("Object is too big (object size/block size):");
				TP_LOG_FATAL_ID (sizeof(T));
				TP_LOG_FATAL_ID (static_cast<TPIE_OS_OUTPUT_SIZE_T>(m_header->m_blockSize));
	    
				return;
			}
	
			m_fileLength = TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_END);
			assert (m_fileLength >= 0);
	
			m_logicalEndOfStream = item_off_to_file_off (m_header->m_itemLogicalEOF);
	
			if (st == APPEND_STREAM) {
				m_fileOffset = m_logicalEndOfStream;
			} 
			else {
				m_fileOffset = m_osBlockSize;
			}
	
			// By default, all streams are deleted at destruction time.
			// [tavi 01/07/02] No. Streams initialized with given names are persistent.
			m_persistenceStatus = PERSIST_PERSISTENT;
	
			// Register memory usage before returning.
			// [The header is allocated using "new", so no need to register.]
			// Since blocks and header are allocated by mmap and not "new",
			// register memory manually. No mem overhead with mmap
			// register_memory_allocation (sizeof (stream_header));
			get_memory_manager().register_allocation (STREAM_MMAP_MM_BUFFERS * m_header->m_blockSize);
	
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
			stream_mmap<T>::stream_mmap (stream_mmap * super_stream, stream_type st, TPIE_OS_OFFSET sub_begin, TPIE_OS_OFFSET sub_end) {
	
			m_status = STREAM_STATUS_NO_STATUS;

			if (stream_base_generic::available_streams() == 0) {
				
				m_status = STREAM_STATUS_INVALID;

				TP_LOG_FATAL_ID ("BTE error: cannot open more streams.");

				return;
			}
	
			if (super_stream->m_status == STREAM_STATUS_INVALID) {

				m_status = STREAM_STATUS_INVALID;

				TP_LOG_FATAL_ID ("BTE error: super stream is invalid.");

				return;
			}
	
			if (super_stream->m_readOnly && (st != READ_STREAM)) {

				m_status = STREAM_STATUS_INVALID;

				TP_LOG_FATAL_ID ("BTE error: super stream is read only and substream is not.");
	    
				return;
			}

			initialize ();
	
			// Reduce the number of streams avaialble.
			current_streams++;
	
			// Copy the relevant fields from the super_stream.
			m_fileDescriptor = super_stream->m_fileDescriptor;
			m_osBlockSize    = super_stream->m_osBlockSize;
			m_header         = super_stream->m_header;
			m_fileLength     = super_stream->m_fileLength;
			m_substreamLevel = super_stream->m_substreamLevel + 1;
	
			m_persistenceStatus = PERSIST_PERSISTENT;
	
			// The arguments sub_start and sub_end are logical item positions
			// within the stream.  We need to convert them to offsets within
			// the stream where items are found.
	
			TPIE_OS_OFFSET super_item_begin = file_off_to_item_off (super_stream->m_logicalBeginOfStream);
	
			m_logicalBeginOfStream = item_off_to_file_off (super_item_begin + sub_begin);
			m_logicalEndOfStream   = item_off_to_file_off (super_item_begin + sub_end + 1);
	
			if (m_logicalEndOfStream > super_stream->m_logicalEndOfStream) {

				m_status = STREAM_STATUS_INVALID;

				return;
			}
	
			m_fileOffset = m_logicalBeginOfStream;
	
			m_currentBlock = NULL;
			m_blockValid = false;
	
			m_readOnly  = super_stream->m_readOnly;
			m_writeOnly = super_stream->m_writeOnly;
	
			m_path = super_stream->m_path;
	
			record_statistics(STREAM_OPEN);
			record_statistics(SUBSTREAM_CREATE);
	
			// substreams are considered to have no memory overhead!
		}
    
// A psuedo-constructor for substreams.  This serves as a wrapper for
// the constructor above in order to get around the fact that one
// cannot have virtual constructors.
		template <class T>
			err stream_mmap<T>::new_substream (stream_type st, TPIE_OS_OFFSET sub_begin, TPIE_OS_OFFSET sub_end, base_t **sub_stream) {
			// Check permissions.
	
			if ((st != READ_STREAM) && ((st != WRITE_STREAM) || m_readOnly)) {

				*sub_stream = NULL;

				return PERMISSION_DENIED;
			}
	
			tp_assert (((st == WRITE_STREAM) && !m_readOnly) ||
					   (st == READ_STREAM),
					   "Bad things got through the permisssion checks.");
	
			stream_mmap < T > *sub =
				new stream_mmap < T > (this, st, sub_begin, sub_end);
	
			*sub_stream = dynamic_cast<base_t *>(sub);
	
			return NO_ERROR;
		}
    
		template <class T> stream_mmap<T>::~stream_mmap () {
	
			// If the stream is already invalid for some reason, then don't
			// worry about anything.
			if (m_status == STREAM_STATUS_INVALID) {

				TP_LOG_WARNING_ID ("BTE internal error: invalid stream in destructor.");

				return;
			}
	
			// Increase the number of streams avaialble.
			--current_streams;
	
			// If this is writable and not a substream, then put the logical
			// eos back into the header before unmapping it.
			if (!m_readOnly && !m_substreamLevel) {
				m_header->m_itemLogicalEOF = file_off_to_item_off (m_logicalEndOfStream);
			}
	
			assert (m_substreamLevel ||
					m_header->m_itemLogicalEOF == file_off_to_item_off (m_logicalEndOfStream));
	
			// Unmap the current block if necessary.
			if (m_blockMapped) {
				unmap_current();
			}
	
			// If this is not a substream then close the file.
			if (!m_substreamLevel) {
				// [Rajiv] make sure the length of the file is correct
				// [tavi 06/23/02] and file is not read-only! 
				if ((m_fileLength > m_logicalEndOfStream) && (!m_readOnly) &&
					(TPIE_OS_FTRUNCATE (m_fileDescriptor, BLOCK_OFFSET(m_logicalEndOfStream) + m_header->m_blockSize) < 0)) {
		
					m_osErrno = errno;
		
					TP_LOG_FATAL_ID("Failed to ftruncate() to the new end of " << m_path);
					TP_LOG_FATAL_ID("m_fileLength:" << m_fileLength << ", m_logicalEndOfStream:" << m_logicalEndOfStream);
					TP_LOG_FATAL_ID("argument to ftruncate:" << BLOCK_OFFSET(m_logicalEndOfStream) + m_header->m_blockSize);
					TP_LOG_FATAL_ID(strerror (m_osErrno));
		
				}
	    
				// Unmap the header.
				// [tavi 06/23/02] Added test for m_readOnly. 
				if (!m_readOnly) {
					unmap_header();
				}
	    
				// Close the file.
				if (TPIE_OS_CLOSE (m_fileDescriptor)) {

					m_osErrno = errno;

					TP_LOG_WARNING_ID("Failed to close() " << m_path);
					TP_LOG_WARNING_ID(strerror (m_osErrno));

				}
	    
				// If it should not persist, unlink the file.
				if (m_persistenceStatus == PERSIST_DELETE) {
					if (m_readOnly) {
						TP_LOG_WARNING_ID("PERSIST_DELETE for read-only stream in " << m_path);
					}
					else {
						if (TPIE_OS_UNLINK (m_path)) {
			
							m_osErrno = errno;
			
							TP_LOG_WARNING_ID ("unlink() failed during destruction of " << m_path);
							TP_LOG_WARNING_ID (strerror (m_osErrno));
						} 
						else {
							record_statistics(STREAM_DELETE);
						}
					}
				}
	    
				// Register memory deallocation before returning.
				get_memory_manager().register_deallocation (STREAM_MMAP_MM_BUFFERS *
											  m_header->m_blockSize);
	    
				delete m_header;
	    
			} 
			else {
				record_statistics(SUBSTREAM_DELETE);
			}
	
			record_statistics(STREAM_CLOSE);
	
		}
    
    
// m_logicalEndOfStream points just past the last item ever written, so if f_current
// is there we are at the end of the stream and cannot read.
    
		template <class T>
			inline err stream_mmap<T>::read_item (T ** elt) {
	
			err retval = NO_ERROR;
	
			if (m_writeOnly) {
				return WRITE_ONLY;
			}
	
			// Make sure we are not currently at the EOS.
			if (static_cast<TPIE_OS_OFFSET>(m_fileOffset + sizeof (T)) > m_logicalEndOfStream) {
				return END_OF_STREAM;
			}
	
			// Validate the current block.
			if ((retval = validate_current ()) != NO_ERROR) {
				return retval;
			}
	
			// Check and make sure that the current pointer points into the
			// current block.
			tp_assert ((static_cast<unsigned int>(
							reinterpret_cast<char*>(m_currentItem) - 
							reinterpret_cast<char*>(m_currentBlock)) <=
						static_cast<unsigned int>(m_header->m_blockSize - sizeof (T))),
					   "m_currentItem is past the end of the current block");
	
			tp_assert ((reinterpret_cast<char*>(m_currentItem) - 
						reinterpret_cast<char*>(m_currentBlock) >= 0),
					   "m_currentItem is before the begining of the current block");

			record_statistics(ITEM_READ);
	
			*elt = m_currentItem;	// Read
			advance_current ();		// move ptr to next elt
	
			// If we are in a substream, there should be no way for f_current to
			// pass m_logicalEndOfStream.
			tp_assert (!m_substreamLevel || (m_fileOffset <= m_logicalEndOfStream),
					   "Got past eos in a substream.");
	
			return NO_ERROR;
		}

// m_logicalEndOfStream points just past the last item ever written, so if f_current
// is there we are at the end of the stream and can only write if this
// is not a substream.
		template <class T>
			inline err stream_mmap<T>::write_item (const T & elt) {
	
			err retval = NO_ERROR;
	
			// This better be a writable stream.
			if (m_readOnly) {

				TP_LOG_WARNING_ID ("write on a read-only stream\n");

				return READ_ONLY;    
			}
	
			// Make sure we are not currently at the EOS of a substream.
			if (m_substreamLevel && (m_logicalEndOfStream <= m_fileOffset)) {

				tp_assert (m_logicalEndOfStream == m_fileOffset, "Went too far in a substream.");
	    
				return END_OF_STREAM;
			}
	
			// Validate the current block.
			if ((retval = validate_current ()) != NO_ERROR) {
				return retval;
			}
	
			// Check and make sure that the current pointer points into the
			// current block.
			tp_assert ((static_cast<unsigned int>(
							reinterpret_cast<char*>(m_currentItem) - 
							reinterpret_cast<char*>(m_currentBlock)) <=
						static_cast<unsigned int>(m_header->m_blockSize - sizeof (T))),
					   "m_currentItem is past the end of the current block");
	
			tp_assert ((reinterpret_cast<char*>(m_currentItem) - 
						reinterpret_cast<char*>(m_currentBlock) >= 0),
					   "m_currentItem is before the begining of the current block");
	
			record_statistics(ITEM_WRITE);
	
			*m_currentItem = elt;		// write
			advance_current ();		// Advance the current pointer.
	
			// If we are in a substream, there should be no way for f_current
			// to pass m_logicalEndOfStream.
			tp_assert (!m_substreamLevel || (m_fileOffset <= m_logicalEndOfStream),
					   "Got past eos in a substream.");
	
			// If we moved past eos, then update eos unless we are in a
			// substream, in which case EOS will be returned on the next call.
			if ((m_fileOffset > m_logicalEndOfStream) && !m_substreamLevel) {
				tp_assert (m_fileOffset <= m_fileLength, "Advanced too far somehow.");
				m_logicalEndOfStream = m_fileOffset;
				// this is the only place m_logicalEndOfStream is changed excluding 
				// constructors and truncate Rajiv
			}
	
			return NO_ERROR;
		}
    
// Query memory usage
    
// Note that in a substream we do not charge for the memory used by
// the header, since it is accounted for in the 0 level superstream.
		template <class T>
		err stream_mmap<T>::main_memory_usage (TPIE_OS_SIZE_T * usage, stream_usage usage_type) {

			switch (usage_type) {

			case STREAM_USAGE_OVERHEAD:
				//Fixed costs. Only 2*mem overhead, because only class and base
				//are allocated dynamicall via "new". Header is read via mmap
				*usage = sizeof(*this) + sizeof(stream_header);

				break;
	    
			case STREAM_USAGE_BUFFER:
				//no mem manager overhead when allocated via mmap
				*usage = STREAM_MMAP_MM_BUFFERS * m_header->m_blockSize;

				break;
	    
			case STREAM_USAGE_CURRENT:
				*usage = (sizeof(*this) + sizeof(stream_header) +
						  ((m_currentBlock == NULL) ? 0 :
						   STREAM_MMAP_MM_BUFFERS * m_header->m_blockSize));
	    
				break;
	    
			case STREAM_USAGE_MAXIMUM:
			case STREAM_USAGE_SUBSTREAM:
				*usage = (sizeof(*this) + sizeof(stream_header) +
						  STREAM_MMAP_MM_BUFFERS * m_header->m_blockSize);
	    
				break;
			}
	
			return NO_ERROR;
		};
    
// Return the number of items in the stream.
		template <class T> 
			TPIE_OS_OFFSET stream_mmap<T>::stream_len () const {
			return file_off_to_item_off (m_logicalEndOfStream) - 
				file_off_to_item_off (m_logicalBeginOfStream);
		};
    
// Move to a specific position.
		template <class T> 
			err stream_mmap<T>::seek (TPIE_OS_OFFSET offset) {
	
			err retval = NO_ERROR;
			TPIE_OS_OFFSET new_offset;
    
			// Looks like we can only seek within the file Rajiv
			if ((offset < 0) ||
				(offset >
				 file_off_to_item_off (m_logicalEndOfStream) - 
				 file_off_to_item_off (m_logicalBeginOfStream))) {
	    
				return OFFSET_OUT_OF_RANGE;
			}
	
			// Compute the new offset
			new_offset =
				item_off_to_file_off (file_off_to_item_off (m_logicalBeginOfStream) + 
									  offset);
	
			if (m_readOnly) {
				tp_assert (new_offset <= m_logicalEndOfStream, "Advanced too far somehow.");
			}
	
	
			if ((static_cast<TPIE_OS_SIZE_T>(reinterpret_cast<char *>(m_currentItem) - 
											 reinterpret_cast<char *>(m_currentBlock)) >=
				 m_header->m_blockSize) ||
				(((new_offset - m_osBlockSize) / m_header->m_blockSize) !=
				 ((m_fileOffset - m_osBlockSize) / m_header->m_blockSize))) {
	    
				if (m_blockValid && ((retval = unmap_current()) != NO_ERROR)) {
		
					return retval;
				}
			} 
			else {
				if (m_blockValid) {
		
					// We have to adjust current.
					register TPIE_OS_OFFSET internal_block_offset;
		
					internal_block_offset = file_off_to_item_off (new_offset) %
						(m_header->m_blockSize / sizeof (T));
		
					m_currentItem = m_currentBlock + internal_block_offset;
				}
			}
	
			m_fileOffset = new_offset;
	
			record_statistics(ITEM_SEEK);
	
			return NO_ERROR;
		}

		template <class T> 
			TPIE_OS_OFFSET stream_mmap<T>::tell() const {
			return file_off_to_item_off(m_fileOffset);
		}
    
// Truncate the stream.
		template <class T> 
			err stream_mmap<T>::truncate (TPIE_OS_OFFSET offset) {
	
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
			new_offset =
				item_off_to_file_off (file_off_to_item_off (m_logicalBeginOfStream) + 
									  offset);
	
			// If it is not in the same block as the current position then
			// invalidate the current block.
	
			// We also need to check that we have the correct block mapped in
			// (m_fileOffset does not always point into the current block!) -
			// see comment in seek()
	
			if ((static_cast<TPIE_OS_SIZE_T>(reinterpret_cast<char *>(m_currentItem) - 
											 reinterpret_cast<char *>(m_currentBlock)) >= 
				 m_header->m_blockSize) ||
				(((new_offset - m_osBlockSize) / m_header->m_blockSize) !=
				 ((m_fileOffset - m_osBlockSize) / m_header->m_blockSize))) {
	    
				if (m_blockValid && ((retval = unmap_current()) != NO_ERROR)) {
					return retval;
				}
			}

			// If it is not in the same block as the current end of stream
			// then truncate the file to the end of the new last block.
			if (((new_offset - m_osBlockSize) / m_header->m_blockSize) !=
				((m_logicalEndOfStream - m_osBlockSize) / m_header->m_blockSize)) {
	    
				if(m_blockMapped) {	
					unmap_current();
				}
	    
				// Determine the offset of the block that new_offset is in.
				block_offset = BLOCK_OFFSET (new_offset);
				m_fileLength = block_offset + m_header->m_blockSize;
	    
				if (TPIE_OS_FTRUNCATE (m_fileDescriptor, m_fileLength)) {
		
					m_osErrno = errno;
		
					TP_LOG_FATAL ("Failed to ftruncate() to the new end of \"");
					TP_LOG_FATAL (m_path);
					TP_LOG_FATAL ("\": ");
					TP_LOG_FATAL (strerror (m_osErrno));
					TP_LOG_FATAL ('\n');
					TP_LOG_FLUSH_LOG;
		
					return OS_ERROR;
				}
			}
	
			if (m_blockValid) {
				// This can happen if we didn't truncate much 
				// and stayed within the current block
				// then the current block is still valid, but the current item
				// pointer may not be valid. We have to adjust current.
				TPIE_OS_OFFSET internal_block_offset;
				internal_block_offset = file_off_to_item_off (new_offset) %
					(m_header->m_blockSize / sizeof (T));
				m_currentItem = m_currentBlock + internal_block_offset;
			}
	
			// Reset the current position to the end.    
			m_fileOffset = m_logicalEndOfStream = new_offset;
	
			return NO_ERROR;
		}
    
// Map in the header from the file.  This assumes that the path
// has been cached in path and that the file has been opened and
// m_fileDescriptor contains a valid descriptor.
		template <class T>
			stream_header * stream_mmap<T>::map_header () {
	
			TPIE_OS_OFFSET file_end;
			stream_header *mmap_hdr;
			stream_header *ptr_to_header;
	
			// If the underlying file is not at least long enough to contain
			// the header block, then, assuming the stream is writable, we have
			// to create the space on disk by doing an explicit write().
			if ((file_end = TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_END)) 
				< static_cast<TPIE_OS_OFFSET>(m_osBlockSize) ) {
				if (m_readOnly) {
		
					m_status = STREAM_STATUS_INVALID;
		
					TP_LOG_FATAL ("No header block in read only stream \"");
					TP_LOG_FATAL (m_path);
					TP_LOG_FATAL ('\n');
					TP_LOG_FLUSH_LOG;
		
					return NULL;
		
				} 
				else {
					// A writable stream, so we can ftruncate() space for a
					// header block.
					if (TPIE_OS_FTRUNCATE (m_fileDescriptor, m_osBlockSize)) {
		    
						m_osErrno = errno;
		    
						TP_LOG_FATAL ("Failed to ftruncate() to end of header of \"");
						TP_LOG_FATAL (m_path);
						TP_LOG_FATAL ("\": ");
						TP_LOG_FATAL (strerror (m_osErrno));
						TP_LOG_FATAL ('\n');
						TP_LOG_FLUSH_LOG;
		    
						return NULL;
					}
				}
			}
	
			// Map in the header block.  If the stream is writable, the header 
			// block should be too.
			// took out the SYSTYPE_BSD ifdef for convenience
			// changed from MAP_FIXED to MAP_VARIABLE because we are using NULL
			mmap_hdr = reinterpret_cast<stream_header *>(
				call_mmap ((NULL), 
						   sizeof (stream_header), 
						   m_readOnly, m_writeOnly, 
						   m_fileDescriptor, 0, 0));
	
			if (mmap_hdr == reinterpret_cast<stream_header *>(-1)) {
	    
				m_status = STREAM_STATUS_INVALID;
				m_osErrno = errno;
	    
				TP_LOG_FATAL ("mmap() failed to map in header from \"");
				TP_LOG_FATAL (m_path);
				TP_LOG_FATAL ("\": ");
				TP_LOG_FATAL (strerror (m_osErrno));
				TP_LOG_FATAL ("\n");
				TP_LOG_FLUSH_LOG;
	    
				return NULL;
			}
	
			ptr_to_header = new stream_header();
			memcpy (ptr_to_header, mmap_hdr, sizeof (stream_header));
			call_munmap (mmap_hdr, sizeof (stream_header));
	
			return ptr_to_header;
		}
    
// Map in the header from the file.  This assumes that the path
// has been cached in path and that the file has been opened and
// m_fileDescriptor contains a valid descriptor.
		template <class T> 
			void stream_mmap<T>::unmap_header () {
	
	
			stream_header *mmap_hdr;
			TPIE_OS_OFFSET file_end;
	
			// If the underlying file is not at least long enough to contain
			// the header block, then, assuming the stream is writable, we have
			// to create the space on disk by doing an explicit write().
			if ((file_end = TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_END)) 
			< static_cast<TPIE_OS_OFFSET>(m_osBlockSize)) {

				if (m_readOnly) {
		
					m_status = STREAM_STATUS_INVALID;
		
					TP_LOG_FATAL ("No header block in read only stream \"");
					TP_LOG_FATAL (m_path);
					TP_LOG_FATAL ('\n');
					TP_LOG_FLUSH_LOG;
		
					return;
		
				} 
				else {
					// A writable stream, so we can ftruncate() space for a
					// header block.
					if (TPIE_OS_FTRUNCATE (m_fileDescriptor, m_osBlockSize)) {
		    
						m_osErrno = errno;
		    
						TP_LOG_FATAL ("Failed to ftruncate() to end of header of \"");
						TP_LOG_FATAL (m_path);
						TP_LOG_FATAL ("\": ");
						TP_LOG_FATAL (strerror (m_osErrno));
						TP_LOG_FATAL ('\n');
						TP_LOG_FLUSH_LOG;
		    
						return;
					}
				}
			}
	
			// Map in the header block.  If the stream is writable, the header 
			// block should be too.
			// took out the SYSTYPE_BSD ifdef for convenience
			// changed from MAP_FIXED to MAP_VARIABLE because we are using NULL
			mmap_hdr = reinterpret_cast<stream_header *>(
				call_mmap ((NULL), sizeof (stream_header),
						   m_readOnly, m_writeOnly,
						   m_fileDescriptor, 0, 0));
	
			if (mmap_hdr == reinterpret_cast<stream_header *>(-1)) {
	    
				m_status = STREAM_STATUS_INVALID;
				m_osErrno = errno;
	    
				TP_LOG_FATAL ("mmap() failed to map in header from \"");
				TP_LOG_FATAL (m_path);
				TP_LOG_FATAL ("\": ");
				TP_LOG_FATAL (strerror (m_osErrno));
				TP_LOG_FATAL ("\n");
				TP_LOG_FLUSH_LOG;
	    
				return;
			}
	
			memcpy (mmap_hdr, m_header, sizeof (stream_header));
			call_munmap (mmap_hdr, sizeof (stream_header));
		}
    
//
// Make sure the current block is mapped in and all internal pointers are
// set as appropriate.  
// 
// 
    
		template <class T>
			inline err stream_mmap<T>::validate_current () {

			err retval = NO_ERROR;
			TPIE_OS_SIZE_T blockSpace;		// The space left in the current block.
	
			// If the current block is valid and current points into it and has
			// enough room in the block for a full item, we are fine.  If it is
			// valid but there is not enough room, invalidate it.
			if (m_blockValid) {
				assert (m_currentItem);		// sanity check - rajiv
				if ((blockSpace = m_header->m_blockSize -
					 (reinterpret_cast<char *>(m_currentItem) - 
					  reinterpret_cast<char *>(m_currentBlock))) >= 
					sizeof (T)) {

					return NO_ERROR;
				} 
				else {			// Not enough room left.
					// no real need to call invalidate here 
					// since we call map_current anyway Rajiv
					if ((retval = unmap_current ()) != NO_ERROR) {
						return retval;
					}
					m_fileOffset += blockSpace;
				}
			}
			// The current block is invalid, since it was either invalid to start
			// with or we just invalidated it because we were out of space.
	
			tp_assert (!m_blockValid, "Block is already mapped in.");
	
			// Now map it the block.
			retval = map_current ();
			assert (m_currentItem);
	
	
			// Rajiv
			tp_assert (static_cast<TPIE_OS_OFFSET>(m_fileOffset + sizeof (T)) <=
				m_fileLength, "Advanced too far somehow.");

			return retval;
		}
    
// Map in the current block.
// m_fileOffset is used to determine what block is needed.
		template <class T> 
			err stream_mmap<T>::map_current () {
	
			err retval = NO_ERROR;
			TPIE_OS_OFFSET blockOffset;
			bool do_mmap = false;
	
			// We should not currently have a valid block.
			tp_assert (!m_blockValid, "Block is already mapped in.");
	
			// Determine the offset of the block that the current item is in.
			blockOffset = BLOCK_OFFSET (m_fileOffset);
	
			// If the block offset is beyond the logical end of the file, then
			// we either record this fact and return (if the stream is read
			// only) or ftruncate() out to the end of the current block.
			assert (TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_END) ==
					m_fileLength);
	
			// removed -1 from rhs of comparison below Rajiv
			if (m_fileLength < 
				  static_cast<TPIE_OS_OFFSET>(blockOffset + m_header->m_blockSize) ) 
			{
				if (m_readOnly) {
					return END_OF_STREAM;
				} 
				else {	
					if ((retval = grow_file (blockOffset)) != NO_ERROR) {
						return retval;
					}
				}
			}
			// this is what we just fixed. Rajiv
			tp_assert (static_cast<TPIE_OS_OFFSET>(m_fileOffset + sizeof (T))
					<= m_fileLength, "Advanced too far somehow.");
	
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
	
			// Map it in either r/w or read only.  
#ifdef STREAM_MMAP_READ_AHEAD
			if (m_haveNextBlock && (blockOffset == m_nextBlockOffset)) {
				T *temp;
	    
				temp            = m_currentBlock;
				m_currentBlock  = m_nextBlock;
				m_nextBlock     = temp;
				m_haveNextBlock = false;
#ifdef COLLECT_STATS
				stats_hits++;
#endif
			} 
			else {
#ifdef COLLECT_STATS
				if (m_haveNextBlock) {
					// not sequential access
					//munmap((caddr_t)m_nextBlock, m_header->m_blockSize);
					//m_haveNextBlock = false;
					//m_nextBlock = NULL;
					stats_misses++;
				}
				stats_compulsory++;
#endif
				do_mmap = true;
			}
#else
			do_mmap = true;
#endif
			if (do_mmap) {
	    
				// took out the SYSTYPE_BSD ifdef for convenience
				// MAP_VARIABLE the first time round
				// (m_currentBlock ? MAP_FIXED : MAP_VARIABLE) |
				if (static_cast<TPIE_OS_OFFSET>(blockOffset + m_header->m_blockSize) > m_fileLength) {
					grow_file(blockOffset);
				}
	    
				m_currentBlock = reinterpret_cast<T *>(
					call_mmap (m_currentBlock, 
							   m_header->m_blockSize,
							   m_readOnly, m_writeOnly, 
							   m_fileDescriptor, 
							   blockOffset,
							   (m_currentBlock != NULL)));
				m_blockMapped = false;
			}
	
			assert (reinterpret_cast<void *>(m_currentBlock) != 
					reinterpret_cast<void *>(m_header));
	
			if (m_currentBlock == reinterpret_cast<T *>(-1)) {
	    
				m_status = STREAM_STATUS_INVALID;
				m_osErrno = errno;
	    
				TP_LOG_FATAL ("mmap() failed to map in block at ");
				TP_LOG_FATAL (blockOffset);
				TP_LOG_FATAL (" from \"");
				TP_LOG_FATAL (m_path);
				TP_LOG_FATAL ("\": ");
				TP_LOG_FATAL (strerror (m_osErrno));
				TP_LOG_FATAL ('\n');
				TP_LOG_FLUSH_LOG;
				perror ("mmap failed");	// Rajiv
	    
				return OS_ERROR;
			}
	
			m_blockValid = true;    
	
#ifdef STREAM_MMAP_READ_AHEAD
			// Start the asyncronous read of the next logical block.
			read_ahead ();
#endif
	
			// The offset, in terms of number of items, that current should
			// have relative to m_currentBlock.
	
			register TPIE_OS_OFFSET internalBlockOffset;
	
			internalBlockOffset = file_off_to_item_off (m_fileOffset) %
				(m_header->m_blockSize / sizeof (T));
	
			m_currentItem = m_currentBlock + internalBlockOffset;
			assert (m_currentItem);
	
			record_statistics(BLOCK_READ);
	
			return NO_ERROR;
		}
    
		template <class T>
			inline err stream_mmap<T>::invalidate_current () {
	
			// We should currently have a valid block.
			tp_assert (m_blockValid, "No block is mapped in.");
			m_blockValid = false;
	
			return NO_ERROR;
		}
    
		template <class T>
			err stream_mmap<T>::unmap_current () {
	
			// Unmap it.
			if (call_munmap (m_currentBlock, m_header->m_blockSize)) {
	    
				m_status = STREAM_STATUS_INVALID;
				m_osErrno = errno;
	    
				TP_LOG_FATAL ("munmap() failed to unmap current block");
				TP_LOG_FATAL ("\": ");
				TP_LOG_FATAL (strerror (m_osErrno));
				TP_LOG_FATAL ('\n');
				TP_LOG_FLUSH_LOG;
	    
				return OS_ERROR;
			}
	
			m_currentBlock = NULL;		// to be safe
			m_blockMapped  = false;
			m_blockValid   = false;
	
			record_statistics(BLOCK_WRITE);
	
			return NO_ERROR;
		}
    
// A uniform method for advancing the current pointer.  No mapping,
// unmapping, or anything like that is done here.
		template <class T>
			inline err stream_mmap<T>::advance_current () {
	
			tp_assert (m_fileOffset <= m_fileLength, "Advanced too far somehow.");
	
			// Advance the current pointer and the file offset of the current
			// item.
			m_currentItem++;
			m_fileOffset += sizeof (T);
	
			return NO_ERROR;
		}
    
    
// increase the length of the file, to at least 
// blockOffset + m_header->m_blockSize
		template <class T> inline err
			stream_mmap<T>::grow_file (TPIE_OS_OFFSET blockOffset)
		{
			// can't grow substreams (except if called for the
			// last substream in a stream. this may happen if map_current
			// maps in the last block of a (sub-)stream).
			// (tavi) I took this out since ignoreSubstream is not declared...
			//    assert (ignoreSubstream || !m_substreamLevel);
			assert(!m_substreamLevel);
	
			m_fileLength = blockOffset + m_header->m_blockSize;

			if (TPIE_OS_FTRUNCATE (m_fileDescriptor, m_fileLength) < 0) {
	    
				m_osErrno = errno;
	    
				TP_LOG_FATAL ("Failed to ftruncate() out a new block of \"");
				TP_LOG_FATAL (m_path);
				TP_LOG_FATAL ("\": ");
				TP_LOG_FATAL (strerror (m_osErrno));
				TP_LOG_FATAL ('\n');
				TP_LOG_FLUSH_LOG;
	    
				return END_OF_STREAM;	// generate an error Rajiv
			}
	
			assert (TPIE_OS_LSEEK(m_fileDescriptor, 0, TPIE_OS_FLAG_SEEK_END) ==
					m_fileLength);

			return NO_ERROR;
		}


		template <class T>
			TPIE_OS_OFFSET stream_mmap<T>::item_off_to_file_off (TPIE_OS_OFFSET itemOffset) const {

			TPIE_OS_OFFSET fileOffset;
	
			// Move past the header.
			fileOffset = m_osBlockSize;
	
			// Add m_header->m_blockSize for each full block.
			fileOffset += m_header->m_blockSize *
				(itemOffset / (m_header->m_blockSize / sizeof (T)));
	
			// Add sizeof(T) for each item in the partially full block.
			fileOffset += sizeof (T) * 
				(itemOffset % (m_header->m_blockSize / sizeof (T))); 
	
			return fileOffset;
		}
    
		template <class T>
			TPIE_OS_OFFSET stream_mmap<T>::file_off_to_item_off (TPIE_OS_OFFSET fileOffset) const {

			TPIE_OS_OFFSET itemOffset;
	
			// Subtract off the header.
			fileOffset -= m_osBlockSize;
	
			// Account for the full blocks.
			itemOffset = (m_header->m_blockSize / sizeof (T)) *
				(fileOffset / m_header->m_blockSize);
	
			// Add in the number of items in the last block.
			itemOffset += (fileOffset % m_header->m_blockSize) / sizeof (T);
	
			return itemOffset;
		}
    
		template <class T> 
			TPIE_OS_OFFSET stream_mmap<T>::chunk_size () const {
			return m_header->m_blockSize / sizeof (T);
		}
    
    
#ifdef STREAM_MMAP_READ_AHEAD
		template <class T> 
			void stream_mmap<T>::read_ahead () {
	
			TPIE_OS_OFFSET currentBlock;
	
			// The current block had better already be valid or we made a
			// mistake in being here.
	
			tp_assert (m_blockValid,
					   "Trying to read ahead when current block is invalid.");
	
			// Check whether there is a next block.  If we are already in the
			// last block of the file then it makes no sense to read ahead.
			// What if we are writing?? Rajiv
			currentBlock = ((m_fileOffset - m_osBlockSize) / m_header->m_blockSize) *
				m_header->m_blockSize + m_osBlockSize;
	
			if (m_logicalEndOfStream < currentBlock + 2 * m_header->m_blockSize) {
				return;			// XXX
				// need to fix this    
				// if not read only, we can extend the file and prefetch.
				// (only if not a substream)
				// Rajiv
				// prefetch only if write only and not substream
				if (!m_writeOnly || m_substreamLevel) {
#ifdef COLLECT_STATS
					stats_eos++;
#endif
					return;
				}
				if (m_writeOnly &&
					!m_substreamLevel &&
					(currentBlock + 2 * m_header->m_blockSize > m_fileLength)) {
					grow_file (currentBlock);
				}
			}
	
			m_nextBlockOffset = currentBlock + m_header->m_blockSize;
	
			// Rajiv
			assert (m_nextBlockOffset + m_header->m_blockSize <= m_fileLength);
			assert (m_nextBlock != m_currentBlock);
			
			// took out the SYSTYPE_BSD ifdef for readability Rajiv
			m_nextBlock = (T *) (call_mmap (m_nextBlock, m_header->m_blockSize,
											m_readOnly, m_writeOnly,
											m_fileDescriptor, m_nextBlockOffset, (m_nextBlock != NULL)));
			assert (m_nextBlock != (T *) - 1);
			m_haveNextBlock = true;
		}
    
#endif				// STREAM_MMAP_READ_AHEAD
    
#undef STREAM_MMAP_MM_BUFFERS
	} //  bte namespace

} // tpie namespace

#endif	// _TPIE_BTE_STREAM_MMAP_H
