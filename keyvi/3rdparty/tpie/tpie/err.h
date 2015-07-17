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

#ifndef _TPIE_AMI_ERR_H
#define _TPIE_AMI_ERR_H

// Windows headers defined this constant too,
// so undef it if the user insists on using the old
// error codes.
#ifdef NO_ERROR
#undef NO_ERROR
#endif

///////////////////////////////////////////////////////////////////////////
/// \file tpie/err.h 
/// Legacy AMI error types.
///////////////////////////////////////////////////////////////////////////

namespace tpie {

    namespace ami {
	
    ///////////////////////////////////////////////////////////////////////////
    /// Legacy TPIE error codes.
	///
	/// Functions in the AMI interface of TPIE typically return error codes of
	/// the enumerated type err.
    ///////////////////////////////////////////////////////////////////////////
    enum err {
      /** No error occurred.  The call the the entry point returned normally. */
      NO_ERROR = 0,
	    /** A low level I/O error occurred. */
      IO_ERROR,
	    /** An attempt was made to read past the end of a stream or 
	     * write past the end of a substream. */
      END_OF_STREAM,
	    /** An attempt was made to write to a read-only stream. */
      READ_ONLY,
	    /** An unexpected operating system error occurred.  Details should appear
	     *  in the log file if logging is  enabled. \sa sec_logging */
      OS_ERROR,
	    /** An attempt was made to call a member function of the virtual base
	     *  class of tpie::ami::stream.  This indicates a bug in the implementation of 
	     * TPIE streams. */
	    BASE_METHOD,
	    /** An error occurred at the BTE level. */
	    BTE_ERROR,
	    /** An error occurred within the memory manager */
	    MM_ERROR,
	    /** An TPIE entry point was not able to properly initialize the operation
	     *  management object that was passed to it.  This generally indicates 
	     * a bug in the operation management object's initialization code. */
	    OBJECT_INITIALIZATION,
      /** A passed object is invalid */
	    OBJECT_INVALID,
	    /** A passed object is inaccessible due to insufficient permissions. */
	    PERMISSION_DENIED,
      /** The memory manager could not make 
       * adequate main memory available to complete the 
       * requested operation.  Many operations adapt themselves to use whatever 
       * main memory is available, but in some cases, when memory is extremely
       * tight, they may not be able to function. */
      INSUFFICIENT_MAIN_MEMORY,
      /** TPIE could not allocate enough intermediate streams to perform
       * the requested operation.  Certain operating system restrictions
       * limit the number of streams that can be created on certain
       * platforms.  Only in unusual circumstances, such as when the
       *  application itself has a very large number of open streams, will
       * this error occur. */ 
	    INSUFFICIENT_AVAILABLE_STREAMS,
	    /** An environment variable necessary to initialize the TPIE
	     * accessing environment was not defined. */
	    ENV_UNDEFINED,
	    NO_MAIN_MEMORY_OPERATION,
	    /** A bit matrix larger than the number of bits in an offset into a
       * stream was passed. */ 
	    BIT_MATRIX_BOUNDS,
	    /** The length of a stream on which a bit permutation was to be
       * performed is not a power of two. */
	    NOT_POWER_OF_2,
	    /** An attempt was made to perform a
       * matrix operation on matrices whose bounds did not match appropriately. */
	    NULL_POINTER,

	    GENERIC_ERROR = 0xfff,

	    /** Value returned by a scan_object: Indicates that the function should be 
	     * called again with any "taken" inputs replaced by the next objects from 
	     * their respective streams */
	    SCAN_DONE = 0x1000,
	    /**  Value returned by a scan_object: Indicates that the scan is complete 
	     * and no more input needs to be processed. */
	    SCAN_CONTINUE,

	    /** Value returned by a \ref merge_management_object, signaling that the merge() completed. */
	    MERGE_DONE = 0x2000,
      /** Value returned by a \ref merge_management_object, telling merge()
       * to continue to call the operate() member function of the
       * management object with more data */
       MERGE_CONTINUE,
      /** Value returned by a \ref merge_management_object, signaling that the last 
       * merge() call generated output for the output stream. */
	    MERGE_OUTPUT,
	     /** Value returned by a \ref merge_management_object, telling merge() 
	      * that more than one input ob ject was consumed and
	      * thus the input flags should be consulted. */
	    MERGE_READ_MULTIPLE,

	    /** Matrix related error */
	    MATRIX_BOUNDS = 0x3000,

	    /** Values returned by sort routines if input does not require sorting */
	    SORT_ALREADY_SORTED = 0x4000
  
	};

    }  //  ami namespace

}  //  tpie namespace

#define USE_OLD_ERROR_CODES 1

#if USE_OLD_ERROR_CODES
	enum AMI_err {
	    AMI_ERROR_NO_ERROR = 0,
	    AMI_ERROR_IO_ERROR,
	    AMI_ERROR_END_OF_STREAM,
	    AMI_ERROR_READ_ONLY,
	    AMI_ERROR_OS_ERROR,
	    AMI_ERROR_BASE_METHOD,
	    AMI_ERROR_BTE_ERROR,
	    AMI_ERROR_MM_ERROR,
	    AMI_ERROR_OBJECT_INITIALIZATION,
	    AMI_ERROR_OBJECT_INVALID,
	    AMI_ERROR_PERMISSION_DENIED,
	    AMI_ERROR_INSUFFICIENT_MAIN_MEMORY,
	    AMI_ERROR_INSUFFICIENT_AVAILABLE_STREAMS,
	    AMI_ERROR_ENV_UNDEFINED,
	    AMI_ERROR_NO_MAIN_MEMORY_OPERATION,
	    AMI_ERROR_BIT_MATRIX_BOUNDS,
	    AMI_ERROR_NOT_POWER_OF_2,
	    AMI_ERROR_NULL_POINTER,

	    AMI_ERROR_GENERIC_ERROR = 0xfff,

	    // Values returned by scan objects.
	    AMI_SCAN_DONE = 0x1000,
	    AMI_SCAN_CONTINUE,

	    // Values returned by merge objects.
	    AMI_MERGE_DONE = 0x2000,
	    AMI_MERGE_CONTINUE,
	    AMI_MERGE_OUTPUT,
	    AMI_MERGE_READ_MULTIPLE,

	    // Matrix related errors
	    AMI_MATRIX_BOUNDS = 0x3000,

	    // Values returned by sort routines.
	    AMI_SORT_ALREADY_SORTED = 0x4000
  
	};
#endif

#endif // _TPIE_AMI_ERR_H
