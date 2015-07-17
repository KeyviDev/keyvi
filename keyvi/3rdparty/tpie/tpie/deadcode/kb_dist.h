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

///////////////////////////////////////////////////////////////////////////
/// \file kb_dist.h
/// Radix based distribution for single or striped AMI layers.
///////////////////////////////////////////////////////////////////////////


// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// If we have not already seen this file with KB_KEY undefined or
// KB_KEY is defined, we will process the file.

namespace tpie {

    namespace ami {
	
#if !(defined(_KB_DIST_H)) || defined(KB_KEY)
	
#ifdef KB_KEY
	
#define _KB_CONCAT(a,b) a ## b
#define _KB_DIST(kbk) _KB_CONCAT(kb_dist_,kbk)
	
#else
	
// KB_KEY is not defined, so set the flag so we won't come through
// this file again with KB_KEY unset, and set KB_KEY to kb_key
// temporarily.  We also Set the macro for the name of the function
// defined in this file.
	
#define _KB_DIST_H
#define KB_KEY kb_key
	
#ifdef _HAVE_TEMP_KB_KEY_DEFINITION_
#error _HAVE_TEMP_KB_KEY_DEFINITION_ already defined.
#else
#define _HAVE_TEMP_KB_KEY_DEFINITION_
#endif
	
#define _KB_DIST(kbk) kb_dist
	
#endif

    }  //  ami namespace

}  //  tpie namespace
	
#include <tpie/stream.h>
#include <tpie/key.h>
	

namespace tpie {

    namespace ami {
	
// This is a hack.  The reason it is here is that if AMI_STREAM<char>
// is used directly in the template for AMI_kb_dist() a parse error is
// generated at compile time.  I suspect this may be a bug in the
// template instantiation code in g++ 2.6.3.
	
#ifndef _DEFINED_TYPE_AMISC_
#define _DEFINED_TYPE_AMISC_
	typedef stream<char> type_amisc;
#endif
	
	template<class T>
	err _KB_DIST(KB_KEY)(stream<T> &instream,
                             type_amisc &name_stream,
                             const key_range &range, TPIE_OS_OFFSET &max_size) {
	    err ae;
	    
	    TPIE_OS_SIZE_T sz_avail;
	    TPIE_OS_SIZE_T single_stream_usage;
	    
	    // How many ouput streams will there be?
	    unsigned int output_streams;
	    
	    unsigned int ii;
	    
	    // How much main memory do we have?
	    sz_avail = get_memory_manager().available();
	    
	    // How much memory does a single stream need in the worst case?
	    if ((ae = instream.main_memory_usage(&single_stream_usage,
											 STREAM_USAGE_MAXIMUM)) !=
		NO_ERROR) {
		return ae;
	    }
	    
	    // How many output streams can we buffer in that amount of space?
	    // Recall that we also need a pointer and a range for each stream.
	    output_streams = static_cast<unsigned int>((sz_avail - 
							2 * single_stream_usage) /
						       (single_stream_usage + 
							sizeof(stream<T> *) + 
							sizeof(range)));
	    
	    // We need at least two output streams.
	    if (output_streams < 2) {
		return INSUFFICIENT_MAIN_MEMORY;
	    }
	    
	    // Make sure we don't use more streams than are available.
	    {
		unsigned available_streams = instream.available_streams();

#ifdef _WIN32
		// Suppress warning 4146 (unary minus for unsigned type) once.
		// Is there a better way to do this (w/o using exceptions)?
#pragma warning(disable : 4146)
#endif		
		if ((available_streams != -1u) &&
		    (available_streams < output_streams)) {
		    output_streams = available_streams;
		}
#ifdef _WIN32
		//  Reset to the default state.
#pragma warning(default : 4146)
#endif
	    }
	    
#ifdef RADIX_POWER_OF_TWO
	    // Adjust the number of output streams so that it is a power of two.
	    
#endif
	    
	    // Create the output streams and initialize the ranges they cover to
	    // be empty.
	    
	    stream<T> **out_streams = tpie_new_array<stream<T> *>(output_streams);
	    key_range *out_ranges = tpie_new_array<key_range>(output_streams);
	    
	    for (ii = 0; ii < output_streams; ii++) {
		
		// This needs to be fixed to eliminate the max size parameter.
		// This should be done system-wide.
			out_streams[ii] = tpie_new< stream<T> >();;
		
			out_ranges[ii].put_min(KEY_MAX);  // Simulate +infty.
			out_ranges[ii].put_max(KEY_MIN);  // Simulate -infty
	    }
	    
	    // Scan the input putting each item in the right output stream.
	    
	    instream.seek(0);
	    
	    unsigned int index_denom = (((range.get_max() - range.get_min()) / output_streams)
					+ 1);
	    
	    //  PLEASE FIX THIS
	    while (1) {
		T      *in;
		kb_key k;
		
		ae = instream.read_item(&in);
		if (ae == END_OF_STREAM) {
		    break;
		} else if (ae != NO_ERROR) {
		    return ae;
		}
		
		k = KB_KEY(*in);
		
#ifdef AMI_RADIX_POWER_OF_TWO
		// Do it with shifting and masking.
		
#else
		ii = (k - range.get_min()) / index_denom;
#endif
		
		ae = out_streams[ii]->write_item(*in);
		if (ae != NO_ERROR) {
		    return ae;
		}
		
		if (k < out_ranges[ii].get_min()) {
		    out_ranges[ii].put_min(k);
		}
		if (k > out_ranges[ii].get_max()) {
		    out_ranges[ii].put_max(k);
		}
		
	    }

	    // Write the names and ranges of all non-empty output streams.	    
	    for (ii = 0, max_size = 0; ii < output_streams; ++ii) {
		std::string stream_name;
		TPIE_OS_OFFSET stream_len;
		
		if ((stream_len = out_streams[ii]->stream_len()) > 0) {
		    
		    // Is it the biggest one so far?		    
		    if (stream_len > max_size) {
			max_size = stream_len;
		    }
		    
		    // Get the and write the name of the stream.		    
		    stream_name = out_streams[ii]->name();
		    ae = name_stream.write_array(stream_name.c_str(),
						 stream_name.size());
		    if (ae != NO_ERROR) {
			return ae;
		    }
		    ae = name_stream.write_item('\0');
		    if (ae != NO_ERROR) {
			return ae;
		    }
		    
		    // For purposes of efficiency, and to avoid having another
		    // stream, we are going to cast the new range to an array of
		    // characters and tack it onto the stream name stream.
		    // We then do the same thing with the length of the stream.
		    
		    ae = name_stream.write_array(reinterpret_cast<const char*>(out_ranges+ii),
						 sizeof(key_range));
		    if (ae != NO_ERROR) {
			return ae;
		    }
		    
		    ae = name_stream.write_array(reinterpret_cast<const char*>(&stream_len),
						 sizeof(stream_len));
		    if (ae != NO_ERROR) {
			return ae;
		    }
		    
		    // Make this stream persist on disk since it is not empty.
		    out_streams[ii]->persist(PERSIST_PERSISTENT);
		    
		}

		// Delete the stream.
		tpie_delete(out_streams[ii]);
		
	    }
	    
		tpie_delete_array(out_streams, output_streams);
		tpie_delete_array(out_ranges, output_streams);
    
	    // We're done.	   
	    return NO_ERROR;
	}
	
#ifdef _HAVE_TEMP_KB_KEY_DEFINITION_
#undef _HAVE_TEMP_KB_KEY_DEFINITION_
#undef KB_KEY
#endif
	
#ifdef _KB_CONCAT
#undef _KB_CONCAT
#endif
#undef _AMI_KB_DIST

    }  //  ami namespace

}  //  tpie namespace

#endif // !(defined(_AMI_KB_DIST_H)) || defined(KB_KEY)
