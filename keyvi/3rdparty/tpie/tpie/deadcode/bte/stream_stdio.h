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

#ifndef _TPIE_BTE_STREAM_STDIO_H
#define _TPIE_BTE_STREAM_STDIO_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// For header's type field (83 == 'S').
#define STREAM_IMPLEMENTATION_STDIO 83

#include <tpie/tpie_assert.h>
#include <tpie/tpie_log.h>

#include <tpie/bte/stream_base.h>

#include <cstring>
#include <cstdio>

// File system streams are streams in a special format that is designed 
// to be stored in an ordinary file in a UN*X file system.  They are 
// predominatly designed to be used to store streams in a persistent way.
// They may disappear in later versions as persistence becomes an integral
// part of TPIE.
//
// For simplicity, we work through the standard C I/O library (stdio).
//

namespace tpie {
    
    namespace bte {
    
// A class of BTE streams implemented using ordinary stdio semantics.
	template <class T> 
	class stream_stdio: public stream_base<T, stream_stdio<T> > {
	public:
		typedef stream_base<T, stream_stdio<T> > base_t;
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
	
	    // Constructors
	    stream_stdio (const std::string& dev_path, 
			  const stream_type st, 
			  TPIE_OS_SIZE_T    lbf = 1);
	
	    // A psuedo-constructor for substreams.
	    err new_substream (stream_type    st, 
			       TPIE_OS_OFFSET sub_begin,
			       TPIE_OS_OFFSET sub_end,
			       base_t **sub_stream);
  
	    ~stream_stdio();
	
	    inline err read_item(T ** elt);
	    inline err write_item(const T & elt);
	
	    // Move to a specific position in the stream.
	    err seek (TPIE_OS_OFFSET offset);
	
	    // Truncate the stream.
	    err truncate (TPIE_OS_OFFSET offset);
	
	    // Return the number of items in the stream.
	    inline TPIE_OS_OFFSET stream_len () const;
	
	    // Return the current position in the stream.
	    inline TPIE_OS_OFFSET tell () const;
	
	    // Query memory usage
	    err main_memory_usage (TPIE_OS_SIZE_T * usage, 
							   stream_usage usage_type);
	
	    TPIE_OS_OFFSET chunk_size () const;
	
	private:
	    /////////////////////////////////////////////////////////
	    // A substream constructor. This constructor is 
	    // cannot be used in a meaningful way, since,
	    // in order to get new file pointer for the 
	    // substream, we need to reopen the file...this
	    // is done in the "standard" constructor.
	    //
	    //	stream_stdio(stream_stdio * super_stream,
	    //       	     stream_type st, 
	    //		     TPIE_OS_OFFSET sub_begin, 
	    //		     TPIE_OS_OFFSET sub_end) {};
	    ///////////////////////////////////////////////////////////
		
	    stream_header* map_header ();
	
	    inline TPIE_OS_OFFSET file_off_to_item_off(TPIE_OS_OFFSET fileOffset) const;
	    inline TPIE_OS_OFFSET item_off_to_file_off(TPIE_OS_OFFSET itemOffset) const;

	    // Descriptor of the underlying file.
	    FILE * m_file;

		//This holds the temporary item used in read_item
		//It's really just a type "T" variable but it's
		//declared like this to avoid requiring that type
		//T has a default constructor.
	    char read_tmp[sizeof(T)];
	};

    
	template <class T>
	stream_stdio<T>::stream_stdio (const std::string& dev_path,
								   const stream_type st,
								   TPIE_OS_SIZE_T /* lbf */) {
	
	    // Reduce the number of streams avaialble.
		if(stream_base_generic::available_streams() == 0) {
			m_status = STREAM_STATUS_INVALID;
			return;
	    }
	
		m_path = dev_path;
	    m_status = STREAM_STATUS_NO_STATUS;
	
	    // Cache the OS block size.
	    m_osBlockSize = os_block_size();
	
	    // This is a top level stream
	    m_substreamLevel = 0;
	
	    // Set stream status to default values.
	    m_persistenceStatus = PERSIST_PERSISTENT;
	
	    m_fileOffset = m_logicalBeginOfStream = m_osBlockSize;
	    m_logicalEndOfStream = m_osBlockSize;
	
	    current_streams++;
		
	    switch (st) {
	    case READ_STREAM:
		// Open the file for reading.
		m_readOnly = true;
	    
		if ((m_file = TPIE_OS_FOPEN(dev_path, "rb")) == NULL) {
		
		    m_status  = STREAM_STATUS_INVALID;
		    m_osErrno = errno;
		
		    TP_LOG_FATAL_ID("Failed to open file:");
		    TP_LOG_FATAL_ID(dev_path);

		    return;
		}
	    
		// Read and check header
		m_header = map_header();
		if (check_header() < 0) {
		
		    m_status = STREAM_STATUS_INVALID;
		
		    TP_LOG_FATAL_ID("Bad header.");

		    return;
		}
	    
		// Seek past the end of the first block.
		if (this->seek(0) != NO_ERROR) {
		
		    m_status  = STREAM_STATUS_INVALID;
		    m_osErrno = errno;
		
		    TP_LOG_FATAL_ID("Cannot seek in file:");
		    TP_LOG_FATAL_ID(dev_path);

		    return;
		}
		break;
	    
	    case WRITE_STREAM:
	    case WRITEONLY_STREAM:
	    case APPEND_STREAM:
	    
		// Open the file for appending.
		m_readOnly = false;
	    
		if ((m_file = TPIE_OS_FOPEN(dev_path, "rb+")) == NULL) {
		    //m_file does not  exist - create it
		    if ((m_file = TPIE_OS_FOPEN (dev_path, "wb+")) == NULL) {
		    
			m_status = STREAM_STATUS_INVALID;
			m_osErrno = errno;
		    
			TP_LOG_FATAL_ID("Failed to open file:");
			TP_LOG_FATAL_ID(dev_path);

			return;
		    }
		
		    // Create and write the header
		    m_header = tpie_new<stream_header>();
		    init_header();
		
		    m_header->m_type = STREAM_IMPLEMENTATION_STDIO;
		
		    if (TPIE_OS_FWRITE (reinterpret_cast<char*>(m_header), 
					sizeof (stream_header), 1, m_file) != 1) {
		    
			m_status = STREAM_STATUS_INVALID;
		    
			TP_LOG_FATAL_ID("Failed to write header to file:");
			TP_LOG_FATAL_ID(dev_path);

			return;
		    }
		
		    // Truncate the file to header block
		    if (this->truncate(0) != NO_ERROR) {

			TP_LOG_FATAL_ID("Cannot truncate in file:");
			TP_LOG_FATAL_ID(dev_path);
		    
			return;
		    }
		
		    if (this->seek(0) != NO_ERROR) {

			TP_LOG_FATAL_ID("Cannot seek in file:");
			TP_LOG_FATAL_ID(dev_path);

			return;
		    }
		
		    m_logicalEndOfStream = m_osBlockSize;
		
		    record_statistics(STREAM_CREATE);
		
		} 
		else {
		    // File exists - read and check header
		    m_header = map_header();
		    if (check_header () < 0) {
		    
			TP_LOG_FATAL_ID("Bad header in file:");
			TP_LOG_FATAL_ID(dev_path);

			return;
		    }
		
		    // Seek to the end of the stream  if APPEND_STREAM
		    if (st == APPEND_STREAM) {
			if (TPIE_OS_FSEEK (m_file, 0, TPIE_OS_FLAG_SEEK_END)) {
		    
			    m_status = STREAM_STATUS_INVALID;
			    m_osErrno = errno;
		    
			    TP_LOG_FATAL_ID("Failed to go to EOF of file:");
			    TP_LOG_FATAL_ID(dev_path);
		    
			    return;
			}
		
			// Make sure there was at least a full block there to pass.
			if (TPIE_OS_FTELL (m_file) < static_cast<TPIE_OS_LONG>(m_osBlockSize)) {
		    
			    m_status = STREAM_STATUS_INVALID;
			    m_osErrno = errno;
		    
			    TP_LOG_FATAL_ID("File too short:");
			    TP_LOG_FATAL_ID(dev_path);
		    
			    return;
			}
		
		    }
		    else {
			// seek to 0 if  WRITE_STREAM
			if (this->seek(0) != NO_ERROR) {
		    
			    m_status = STREAM_STATUS_INVALID;
			    m_osErrno = errno;
		    
			    TP_LOG_FATAL_ID("Cannot seek in file:");
			    TP_LOG_FATAL_ID(dev_path);
			    return;
			}
		    }
		}
		break;
	    
	    default:
		// Either a bad value or a case that has not been implemented
		// yet.
		m_status = STREAM_STATUS_INVALID;
	    
		TP_LOG_WARNING_ID("Bad or unimplemented case.");
		break;
	    }
	
	    m_logicalEndOfStream = item_off_to_file_off(m_header->m_itemLogicalEOF);
	
	    // Register memory usage before returning.
	    // A quick and dirty guess.  One block in the buffer cache, one in
	    // user space. This is not accounted for by a "new" call, so we have to 
	    // register it ourselves. TODO: is 2*block_size accurate?
	    get_memory_manager().register_allocation (m_osBlockSize * 2);
	
	    record_statistics(STREAM_OPEN);
	
	}
    
// A psuedo-constructor for substreams.  This allows us to get around
// the fact that one cannot have virtual constructors.
	template <class T>
	err stream_stdio<T>::new_substream (stream_type    st,
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
	
	    if (m_substreamLevel) {
		if ((sub_begin * static_cast<TPIE_OS_OFFSET>(sizeof(T)) >=
		     (m_logicalEndOfStream - m_logicalBeginOfStream))
		    || (sub_end * static_cast<TPIE_OS_OFFSET>(sizeof(T)) >
			(m_logicalEndOfStream - m_logicalBeginOfStream))) {
		
		    *sub_stream = NULL;
		
		    return OFFSET_OUT_OF_RANGE;
		}
	    }

	    // We actually have to completely reopen the file in order to get
	    // another seek pointer into it.  We'll do this by constructing
	    // the stream that will end up being the substream.
	    stream_stdio *sub = tpie_new<stream_stdio>(m_path, st);
	
	    // Set up the beginning and end positions.
	    if (m_substreamLevel) {
		sub->m_logicalBeginOfStream =
		    m_logicalBeginOfStream + sub_begin * sizeof (T);
		sub->m_logicalEndOfStream = 
		    m_logicalBeginOfStream + (sub_end + 1) * sizeof (T);
	    } 
	    else {
		sub->m_logicalBeginOfStream = 
		    sub->m_osBlockSize + sub_begin * sizeof (T);
		sub->m_logicalEndOfStream =
		    sub->m_osBlockSize + (sub_end + 1) * sizeof (T);
	    }
	
	    // Set the current position.
	    TPIE_OS_FSEEK (sub->m_file, sub->m_logicalBeginOfStream, 0);
	
	    sub->m_substreamLevel = m_substreamLevel + 1;
	    sub->m_persistenceStatus =
		(m_persistenceStatus == PERSIST_READ_ONCE) ? PERSIST_READ_ONCE : PERSIST_PERSISTENT;
	    *sub_stream = dynamic_cast<base_t *>(sub);
	
	    record_statistics(SUBSTREAM_CREATE);
	
	    return NO_ERROR;
	}


	template <class T> 
	stream_stdio<T>::~stream_stdio() {
	
	    if (m_file && !m_readOnly) {
		m_header->m_itemLogicalEOF = 
		    file_off_to_item_off(m_logicalEndOfStream);
		if (TPIE_OS_FSEEK (m_file, 0, TPIE_OS_FLAG_SEEK_SET) == -1) {
		
		    m_status = STREAM_STATUS_INVALID;
		
		    TP_LOG_WARNING_ID("Failed to seek in file:");
		    TP_LOG_WARNING_ID(m_path);
		}
		else {
		    if (TPIE_OS_FWRITE(reinterpret_cast<char*>(m_header), 
				       sizeof (stream_header), 1, m_file) != 1) {
		    
			m_status = STREAM_STATUS_INVALID;
		    
			TP_LOG_WARNING_ID("Failed to write header to file:");
			TP_LOG_WARNING_ID(m_path);
		    }
		}
	    }
	
	    if (m_file && TPIE_OS_FCLOSE (m_file) != 0) {
	    
		m_status = STREAM_STATUS_INVALID;
	    
		TP_LOG_WARNING_ID("Failed to close file:");
		TP_LOG_WARNING_ID(m_path);
	    }
	
	    if (m_file) {
		// Get rid of the file if not persistent and if not substream.
		if (!m_substreamLevel) {
		    if (m_persistenceStatus == PERSIST_DELETE) {
			if (m_readOnly) {
			    TP_LOG_WARNING_ID("Read only stream is PERSIST_DELETE:");
			    TP_LOG_WARNING_ID(m_path);
			    TP_LOG_WARNING_ID("Ignoring persistency request.");
			} 
			else {
			    if (TPIE_OS_UNLINK (m_path)) {
			    
				m_osErrno = errno;
			    
				TP_LOG_WARNING_ID("Failed to unlink() file:");
				TP_LOG_WARNING_ID(m_path);
				TP_LOG_WARNING_ID(strerror(m_osErrno));
			    } 
			    else {
				record_statistics(STREAM_DELETE);
			    }
			}
		    }
		} 
		else {
		    record_statistics(SUBSTREAM_DELETE);
		}
	    }
	
	
	    // Register memory deallocation before returning.
	    // A quick and dirty guess.  One block in the buffer cache, one in
	    // user space. TODO.
	    get_memory_manager().register_deallocation (m_osBlockSize * 2);
	
	    // In contrast to other BTE's this BTE allocates a header
	    // even for substreams (since the file effectively is
	    // opened each time a substream is constructed).
	    // TODO: Double-check this.
	    tpie_delete(m_header);
	
		current_streams--;

	
	    record_statistics(STREAM_CLOSE);
	}
    
	template <class T> 
	err stream_stdio<T>::read_item (T ** elt) {

	    TPIE_OS_SIZE_T stdio_ret;
	    err retval = NO_ERROR;
	
	    if ((m_logicalEndOfStream >= 0) && 
		(TPIE_OS_FTELL (m_file) >= m_logicalEndOfStream)) {
	    
		tp_assert ((m_logicalBeginOfStream >= 0), "eos set but bos not.");

		m_status = STREAM_STATUS_END_OF_STREAM;
	    
		retval = END_OF_STREAM;
	    
	    } else {
	    
		stdio_ret = TPIE_OS_FREAD (read_tmp, sizeof (T), 1, m_file);
	    
		if (stdio_ret == 1) {
		
		    m_fileOffset += sizeof(T);
		    *elt = reinterpret_cast<T*>(read_tmp);
		
		    retval = NO_ERROR;
		}
		else {
		    // Assume EOF.  Fix this later.
		    m_status = STREAM_STATUS_END_OF_STREAM;
		    retval = END_OF_STREAM;
		}
	    }
	
	    record_statistics(ITEM_READ);
	
	    return retval;
	}

	template <class T> 
	err stream_stdio<T>::write_item (const T & elt) {
	
	    TPIE_OS_SIZE_T stdio_ret;
	    err retval = NO_ERROR;
	
	    if ((m_logicalEndOfStream >= 0) && 
		(TPIE_OS_FTELL (m_file) > m_logicalEndOfStream)) {
	    
		tp_assert ((m_logicalBeginOfStream >= 0), "eos set but bos not.");
		m_status = STREAM_STATUS_END_OF_STREAM;
	    
		retval = END_OF_STREAM;
	    } 
	    else {
		stdio_ret = TPIE_OS_FWRITE (reinterpret_cast<const char*>(&elt),
					    sizeof (T), 1, m_file);
		if (stdio_ret == 1) {
		
		    if (m_logicalEndOfStream == m_fileOffset) {
			m_logicalEndOfStream += sizeof(T);
		    }
		    m_fileOffset += sizeof(T);
		
		    retval = NO_ERROR;
		}
		else {
		    TP_LOG_FATAL_ID("write_item failed.");
		    m_status = STREAM_STATUS_INVALID;
		
		    retval = IO_ERROR;
		}
	    }
	
	    record_statistics(ITEM_WRITE);
	
	    return retval;
	}
    
    
	template <class T>
	err stream_stdio<T>::main_memory_usage (TPIE_OS_SIZE_T * usage,
											stream_usage
						usage_type) {

	    switch (usage_type) {
	    case STREAM_USAGE_OVERHEAD:
		//Fixed overhead per object. *this includes base class.
		//Need to include 2*overhead per "new" that sizeof doesn't 
		//know about.
			*usage = sizeof(*this);

		break;
	    
	    case STREAM_USAGE_BUFFER:
			//Amount used by stdio buffers. No "new" calls => no overhead
			*usage = 2 * m_osBlockSize;
			
			break;
	    
	    case STREAM_USAGE_CURRENT:
	    case STREAM_USAGE_MAXIMUM:
	    case STREAM_USAGE_SUBSTREAM:
			*usage = sizeof(*this) + 2*m_osBlockSize;
		break;
	    }

	    return NO_ERROR;
	}
    
// Move to a specific position.
	template <class T> 
	err stream_stdio<T>::seek (TPIE_OS_OFFSET offset) {
	
	    TPIE_OS_OFFSET filePosition;
	
	    if ((offset < 0) ||
		(offset  > file_off_to_item_off(m_logicalEndOfStream))) {
	    
		TP_LOG_WARNING_ID ("seek() out of range (off/bos/eos)");
		TP_LOG_WARNING_ID (offset);
		TP_LOG_WARNING_ID (m_logicalEndOfStream);
	    
		return OFFSET_OUT_OF_RANGE;
	    } 
	
	    if (m_substreamLevel) {
		if (offset * static_cast<TPIE_OS_OFFSET>(sizeof(T)) > 
		    static_cast<TPIE_OS_OFFSET>(m_logicalEndOfStream - m_logicalBeginOfStream)) {

		    TP_LOG_WARNING_ID ("seek() out of range (off/bos/eos)");
		    TP_LOG_WARNING_ID (m_logicalBeginOfStream);
		    TP_LOG_WARNING_ID (m_logicalEndOfStream);
		
		    return OFFSET_OUT_OF_RANGE;
		}
		else {
		    filePosition = offset * sizeof (T) + m_logicalBeginOfStream;
		}
	    } 
	    else {
		filePosition = offset * sizeof (T) + m_osBlockSize;
	    }
	
	    if (TPIE_OS_FSEEK (m_file, filePosition, TPIE_OS_FLAG_SEEK_SET)) {
	    
		TP_LOG_FATAL("fseek failed to go to position " << filePosition << 
			     " of \"" << "\"\n");
		TP_LOG_FLUSH_LOG;
	    
		return OS_ERROR;
	    }
	
	    m_fileOffset = filePosition;
	
	    record_statistics(ITEM_SEEK);
	
	    return NO_ERROR;
	}
    
// Truncate the stream.
	template <class T>
	err stream_stdio<T>::truncate (TPIE_OS_OFFSET offset) {
//	TPIE_OS_TRUNCATE_STREAM_TEMPLATE_CLASS_BODY;
	    TPIE_OS_OFFSET filePosition; 
	
	    if (m_substreamLevel) { 
		return STREAM_IS_SUBSTREAM; 
	    } 
	
	    if (offset < 0) {   
		return OFFSET_OUT_OF_RANGE; 
	    } 
	
	    filePosition = offset * sizeof (T) + m_osBlockSize; 
	
	    if (TPIE_OS_TRUNCATE(m_file, m_path, filePosition) == -1) {   
	    
		m_osErrno = errno;   
	    
		TP_LOG_FATAL_ID("Failed to truncate() to the new end of file:");   
		TP_LOG_FATAL_ID(m_path);   
		TP_LOG_FATAL_ID(strerror (m_osErrno));   
	    
		return OS_ERROR; 
	    } 
	
	    if (TPIE_OS_FSEEK(m_file, filePosition, SEEK_SET) == -1) { 
	    
		TP_LOG_FATAL("fseek failed to go to position " << 
			     filePosition << " of \"" << "\"\n"); 
		TP_LOG_FLUSH_LOG;  

		return OS_ERROR; 
	    } 
	
	    m_fileOffset         = filePosition; 
	    m_logicalEndOfStream = filePosition; 
	
	    return NO_ERROR;
	}
    
	template <class T> 
	TPIE_OS_OFFSET stream_stdio<T>::chunk_size () const {
	    // Quick and dirty guess.
	    return (m_osBlockSize * 2) / sizeof (T);
	}
    
    
	template<class T> 
	stream_header* stream_stdio<T>::map_header () { 
	
	    stream_header *ptr_to_header = tpie_new<stream_header>();
	
	    // Read the header.
	    if ((TPIE_OS_FREAD (reinterpret_cast<char*>(ptr_to_header), 
				sizeof (stream_header), 1, m_file)) != 1) {

		m_status = STREAM_STATUS_INVALID;
		m_osErrno = errno;
	    
		TP_LOG_FATAL_ID("Failed to read header from file:");
		TP_LOG_FATAL_ID(m_path);
	    
		tpie_delete(ptr_to_header);
	    
		return NULL;
	    }
	
	    return ptr_to_header;
	}

	template <class T> 
	TPIE_OS_OFFSET stream_stdio<T>::tell() const {
	    return file_off_to_item_off(m_fileOffset);
	}
    
// Return the number of items in the stream.
	template <class T> 
	TPIE_OS_OFFSET stream_stdio<T>::stream_len () const {
	    return file_off_to_item_off (m_logicalEndOfStream) - 
		file_off_to_item_off (m_logicalBeginOfStream);
	};
    
	template<class T> 
	TPIE_OS_OFFSET stream_stdio<T>::file_off_to_item_off (TPIE_OS_OFFSET fileOffset) const {
	    return (fileOffset - m_osBlockSize) / sizeof (T);
	}
    
	template<class T> 
	TPIE_OS_OFFSET stream_stdio<T>::item_off_to_file_off (TPIE_OS_OFFSET itemOffset) const {
	    return (m_osBlockSize + itemOffset * sizeof (T));
	}
    
    } // bte namespace

} // tpie namespace

#endif // _TPIE_BTE_STREAM_STDIO_H
    
