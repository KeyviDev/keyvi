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

#ifndef _TPIE_BTE_STREAM_HEADER_H
#define _TPIE_BTE_STREAM_HEADER_H

namespace tpie {

    namespace bte {

// BTE stream header info.
	struct stream_header {
	
	    // Unique header identifier. Set to BTE_STREAM_HEADER_MAGIC_NUMBER.
	    unsigned int m_magicNumber;
	
	    // Should be 2 for current version (version 1 has been deprecated).
	    unsigned int m_version;
	
	    // The type of BTE_STREAM that created this header. Not all types of
	    // BTE's are readable by all BTE implementations. For example,
	    // BTE_STREAM_STDIO streams are not readable by either
	    // BTE_STREAM_UFS or BTE_STREAM_MMAP implementations. The value 0 is
	    // reserved for the base class. Use numbers bigger than 0 for the
	    // various implementations.
	    unsigned int m_type;
	
	    // The number of bytes in this structure.
	    TPIE_OS_SIZE_T m_headerLength;
	
	    // The size of each item in the stream.
	    TPIE_OS_SIZE_T m_itemSize;
	
	    // The size of a physical block on the device this stream resides.
	    TPIE_OS_SIZE_T m_osBlockSize;
	
	    // Size in bytes of each logical block, if applicable.
	    TPIE_OS_SIZE_T m_blockSize;
	
	    // For all intents and purposes, the length of the stream in number
	    // of items.
	    TPIE_OS_OFFSET m_itemLogicalEOF;
	};
    
    }  //  bte namespace

}  //  tpie namespace 

#endif // _TPIE_BTE_STREAM_HEADER_H

