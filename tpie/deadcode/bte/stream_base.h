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

#ifndef _TPIE_BTE_STREAM_BASE_H
#define _TPIE_BTE_STREAM_BASE_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/tpie_assert.h>
#include <tpie/persist.h>
// Get the BTE error codes.
#include <tpie/bte/err.h>
// Get statistics definitions.
#include <tpie/stats_stream.h>

#include <tpie/memory.h>


// The magic number of the file storing the stream.
// (in network byteorder, it spells "TPST": TPie STream)
#define STREAM_HEADER_MAGIC_NUMBER	0x54505354 

#include <tpie/stream_usage.h>

namespace tpie {

    namespace bte {
    
// BTE stream types passed to constructors.
	enum stream_type {
	    READ_STREAM = 1, // Open existing stream for reading.
	    WRITE_STREAM,    // Open for read/writing. Create if non-existent.
	    APPEND_STREAM,   // Open for writing at end. Create if needed.
	    WRITEONLY_STREAM // Open only for writing (allows mmb optimization)
	    // (must be sequential write through whole file)
	};
    
// BTE stream status. 
	enum stream_status {
	    STREAM_STATUS_NO_STATUS = 0,
	    STREAM_STATUS_INVALID = 1,
	    STREAM_STATUS_EOS_ON_NEXT_CALL,
	    STREAM_STATUS_END_OF_STREAM
	};

    }  //  bte namespace
}  //  tpie namespace

#include <tpie/bte/stream_header.h>
#include <tpie/bte/stream_base_generic.h>
    
// An abstract class template which implements a single stream of objects 
// of type T within the BTE.  This is the superclass of all actual 
// implementations of streams of T within the BTE (e.g. mmap() streams, 
// UN*X file system streams, and kernel streams).

namespace tpie {

    namespace bte {


    
	template<class T, class C> 
	class stream_base: public bte::stream_base_generic {
	
	public:
	    stream_base() :
		m_header(NULL),
		m_persistenceStatus(PERSIST_DELETE),
		m_status(STREAM_STATUS_NO_STATUS),
		m_substreamLevel(0),
		m_readOnly(true),
		m_streamStatistics(),
		m_osBlockSize(0),
		m_fileOffset(0),
		m_logicalBeginOfStream(0),
		m_logicalEndOfStream(0),
		m_fileLength(0),
		m_osErrno(0) {
		// No code in this constructor.
	    };
	
	    // Tell the stream whether to leave its data on the disk or not
	    // when it is destructed.
	    void persist (persistence p) { 
		m_persistenceStatus = p; 
	    }
	
	    // Inquire the persistence status of this BTE stream.
	    persistence persist() const { 
		return m_persistenceStatus; 
	    }
	
	    // Return true if a read-only stream.
	    bool read_only () const { 
		return m_readOnly;
	    }
	
	    // Inquire the status.
	    stream_status status() const { 
		return m_status; 
	    }
	
	    // Inquire the OS block size.
	    TPIE_OS_SIZE_T os_block_size () const {
		return TPIE_OS_BLOCKSIZE();
	    }
	
	    const stats_stream& stats() const { 
		return m_streamStatistics; 
	    }
	
	    // Return the path name in newly allocated space.
	    std::string name() const;

		inline err write_array(const T * elms, TPIE_OS_SIZE_T count) {
			for(TPIE_OS_SIZE_T i=0; i < count; ++i) {
				err e = reinterpret_cast<C*>(this)->write_item(elms[i]);
				if(e != NO_ERROR) return e;
			}
			return NO_ERROR;
		}

		inline err read_array(T * elms, TPIE_OS_SIZE_T & count) {
			TPIE_OS_SIZE_T c=count;
			for(count=0; count < c; ++count) {
				T * x;
				err e = reinterpret_cast<C*>(this)->read_item(&x);
				if(e != NO_ERROR) return e;
				elms[count] = *x;
			}
			return NO_ERROR;
		}
	protected:
	
	    using stream_base_generic::current_streams;
	    using stream_base_generic::gstats_;
	
	    // Check the given header for reasonable values.
	    int check_header();
	
	    // Initialize the header with as much information as is known here.
	    void init_header();
	
	    inline err register_memory_allocation (TPIE_OS_SIZE_T sz);
	    inline err register_memory_deallocation (TPIE_OS_SIZE_T sz);
	
	    // Record statistics both globally (on base-class level) and
	    // locally (on instance level).
	    inline void record_statistics(stats_stream_id event);
	
	    // A pointer to the mapped in header block for the stream.
	    stream_header     *m_header;
	
	    // The persistence status of this stream.
	    persistence       m_persistenceStatus;
	
	    // The status (integrity) of this stream.
	    stream_status     m_status;
	
	    // How deeply is this stream nested.
	    unsigned int      m_substreamLevel;
	
	    // Non-zero if this stream was opened for reading only.
	    bool              m_readOnly; 
	
	    // Statistics for this stream only.
	    stats_stream      m_streamStatistics;
	
	    // The size of a physical block.
	    TPIE_OS_SIZE_T    m_osBlockSize;
	
	    // Offset of the current item in the file. This is the logical
	    // offset of the item within the file, that is, the place we would
	    // have to lseek() to in order to read() or write() the item if we
	    // were using ordinary (i.e. non-mmap()) file access methods.
	    TPIE_OS_OFFSET      m_fileOffset;
	
	    // Beginning of the file.  Can't write before here.
	    TPIE_OS_OFFSET      m_logicalBeginOfStream;
	
	    // Offset just past the end of the last item in the stream. If this
	    // is a substream, we can't write here or anywhere beyond.
	    TPIE_OS_OFFSET      m_logicalEndOfStream;
	
	    // Length of the underlying file.
	    TPIE_OS_OFFSET       m_fileLength;
	
	    // A place to cache OS error values. It is normally set after each
	    // call to the OS.
	    int m_osErrno;
	
	    //  Name of the underlying file.
	    std::string m_path;
	
	private:
	    // Prohibit these.
	    stream_base(const stream_base<T,C>& other);
	    stream_base<T,C>& operator=(const stream_base<T,C>& other);
	};
    
	template<class T,class C>
	int stream_base<T,C>::check_header() {

#ifdef _WIN32
		// Suppress warning 4267 (size mismatch of size_t and unsigned int) once.
		// This is recommended by Visual Studio's dynamic help for warning C4267.
#pragma warning(disable : 4267)
#endif


	    if (m_header == NULL) {
	
		TP_LOG_FATAL_ID ("Could not map header.");

		return -1;

	    }

	    if (m_header->m_magicNumber != STREAM_HEADER_MAGIC_NUMBER) {

		TP_LOG_FATAL_ID ("header: magic number mismatch (expected/obtained):");
		TP_LOG_FATAL_ID (STREAM_HEADER_MAGIC_NUMBER);
		TP_LOG_FATAL_ID (m_header->m_magicNumber);

		return -1;

	    }

	    if (m_header->m_headerLength != sizeof (*m_header)) {

		TP_LOG_FATAL_ID ("header: incorrect header length; (expected/obtained):");
		TP_LOG_FATAL_ID (sizeof (stream_header));
		TP_LOG_FATAL_ID (m_header->m_headerLength);
		TP_LOG_FATAL_ID ("This could be due to a stream written without 64-bit support.");
	    
		return -1;

	    }

	    if (m_header->m_version != 2) {
	    
		TP_LOG_FATAL_ID ("header: incorrect version (expected/obtained):");
		TP_LOG_FATAL_ID (2);
		TP_LOG_FATAL_ID (m_header->m_version);
	    
		return -1;

	    }
	
	    if (m_header->m_type == 0) {
	    
		TP_LOG_FATAL_ID ("header: type is 0 (reserved for base class).");
	    
		return -1;

	    }

	    if (m_header->m_itemSize != sizeof (T)) {

		TP_LOG_FATAL_ID ("header: incorrect item size (expected/obtained):");
		TP_LOG_FATAL_ID (sizeof(T));
		TP_LOG_FATAL_ID (static_cast<TPIE_OS_LONGLONG>(m_header->m_itemSize));
	    
		return -1;
	    
	    }

	    if (m_header->m_osBlockSize != os_block_size()) {
	    
		TP_LOG_FATAL_ID ("header: incorrect OS block size (expected/obtained):");
		TP_LOG_FATAL_ID (static_cast<TPIE_OS_LONGLONG>(os_block_size()));
		TP_LOG_FATAL_ID (static_cast<TPIE_OS_LONGLONG>(m_header->m_osBlockSize));
	    
		return -1;
	    
	    }

#ifdef _WIN32
		//  Reset to the default state.
#pragma warning(default : 4267)
#endif
	
	    return 0;
	}
    
	template<class T,class C>
	void stream_base<T,C>::init_header () {

	    tp_assert(m_header != NULL, "NULL header pointer");
	
	    m_header->m_magicNumber    = STREAM_HEADER_MAGIC_NUMBER;
	    m_header->m_version        = 2;
	    m_header->m_type           = 0; // Not known here.
	    m_header->m_headerLength   = sizeof(*m_header);
	    m_header->m_itemSize       = sizeof(T);
	    m_header->m_osBlockSize    = os_block_size();
	    m_header->m_blockSize      = 0; // Not known here.
	    m_header->m_itemLogicalEOF = 0;

	}
    
// Return the path name in newly allocated space.
	template <class T, class C>
	std::string stream_base<T,C>::name() const 
	{
	    return std::string(m_path);
	}

	template <class T, class C>
	inline void stream_base<T,C>::record_statistics(stats_stream_id event) {
	
	    //  Record for base class, i.e., globally.
	    gstats_.record(event);
	
	    //  Record for instance, i.e., locally.
	    m_streamStatistics.record(event);
	
	};

    }  //  bte namespace
}  //  tpie namespace
#endif // _TPIE_BTE_STREAM_BASE_H 
