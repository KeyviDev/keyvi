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

// This header file can be included in one of two ways, either with a
// KB_KEY macro defined, in which case it is assumed to be the name of
// a function (typically inline) for extracting a key from an object
// of type T, or undefined, in which case the conversion operator
// kb_key() will be used by default.  This file can be included
// multiple times in the former case, but only once in the latter.

// If we have not already seen this file with KB_KEY undefined or
// KB_KEY is defined, we will process the file.
#if !(defined(_AMI_KB_SORT_H)) || defined(KB_KEY)

#include <iostream>

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/key.h>
#include <tpie/kb_dist.h>

namespace tpie {

    namespace ami {
	
#ifdef KB_KEY
	
#define _KB_CONCAT(a,b) a ## b
#define _KB_SORT(kbk) _KB_CONCAT(kb_sort_,kbk)
#define _MM_KB_SORT(kbk) _KB_CONCAT(mm_kb_sort_,kbk)
#define _KB_DIST(kbk) _KB_CONCAT(kb_dist_,kbk)

#else

// KB_KEY is not defined, so set the flag so we won't come through
// this file again with KB_KEY unset, and set KB_KEY to kb_key
// temporarily.  We also Set the macro for the name of the function
// defined in this file.

#define _KB_SORT_H
#define KB_KEY kb_key

#ifdef _HAVE_TEMP_KB_KEY_DEFINITION_
#error _HAVE_TEMP_KB_KEY_DEFINITION_ already defined.
#else
#define _HAVE_TEMP_KB_KEY_DEFINITION_
#endif

#define _KB_SORT(kbk) kb_sort
#define _MM_KB_SORT(kbk) mm_kb_sort
#define _KB_DIST(kbk) kb_dist

#endif

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

#ifndef _DEFINED_STATIC_KEY_MIN_MAX
#define _DEFINED_STATIC_KEY_MIN_MAX
	static key_range min_max(KEY_MIN, KEY_MAX);
#endif // _DEFINED_STATIC_KEY_MIN_MAX
	
    }  //  ami namespace

}  //  tpie namespace

#ifndef _TPIE_AMI_BUCKET_LIST_ELEM
#define _TPIE_AMI_BUCKET_LIST_ELEM

namespace tpie {
    
    namespace ami {
	
	template<class T>
	class bucket_list_elem {

	public:
	    T data;
	    bucket_list_elem<T> *next;

	    bucket_list_elem() : data(), next(0) {
		//  No code in this constructor.
	    };
	    
	    bucket_list_elem(const bucket_list_elem<T>& other) {
		*this = other;
	    }

	    bucket_list_elem<T>& operator=(const bucket_list_elem<T>& other) {
		if (this != &other) {
		    data = other.data;
		    next = other.next;
		}
		return *this;
	    }

	    ~bucket_list_elem() {
		//  No code in this destructor.
	    };

	};

    }  //  ami namespace

}  //  tpie namespace
	
#endif

namespace tpie {
    
    namespace ami {

	template<class T>
	err _MM_KB_SORT(KB_KEY)(stream<T> &instream,
				stream<T> &outstream,
				const key_range &range);
	
	
	template<class T>
	err _KB_SORT(KB_KEY)(stream<T> &instream,
			     stream<T> &outstream,
			     const key_range &range)
	{
	    err ae;

	    // Memory sizes.
	    size_t sz_avail, sz_stream;
	    // Stream sizes.
	    TPIE_OS_OFFSET max_size, stream_len;
    
	    // Check whether the problem fits in main memory.

	    sz_avail = get_memory_manager().available ();

	    instream.main_memory_usage(&sz_stream, STREAM_USAGE_MAXIMUM);

	    if (sz_avail < 4 * sz_stream) {
		return INSUFFICIENT_MAIN_MEMORY;
	    }

	    // Account for the two name streams.
    
	    sz_avail -= 2*sz_stream;
    
	    // If it fits, simply read it, sort it, and write it.

	    stream_len = instream.stream_len();

	    if (sz_avail >= stream_len * (sizeof(T) + 
					  sizeof(bucket_list_elem<T> *) + 
					  sizeof(bucket_list_elem<T>))) {
		return _MM_KB_SORT(KB_KEY)(instream, outstream, range);
	    }

	    // It did not fit, so we have to distribute it.
    
	    // Create streams of temporary file names.
	    stream<char> *name_stream, *name_stream2 = NULL;

	    name_stream = tpie_new<stream<char> >();

	    // Do the first level distribution.

	    ae = _KB_DIST(KB_KEY)(instream, *name_stream, range, max_size);

	    // Do the rest of the levels of distribution, continuing until all
	    // streams are small or have a single key and then, in a final
	    // iteration, sorting the streams internally and concatenating
	    // them.  In some bad cases we will end up with one or more large
	    // streams of all the same key, but these are detected and treated
	    // as small streams.
    
	    bool some_stream_is_large = ((max_size * 
					  (sizeof(T) + 4 +
					   sizeof(bucket_list_elem<T> *) + 
					   sizeof(bucket_list_elem<T>))) >
					 sz_avail);
    
	    while (1) {

		// Create a new stream of temporary stream names.
		tp_assert(name_stream2 == NULL, "Non-null target name stream.");
		name_stream2 = tpie_new<stream<char> >();

		// Is this the last (special) iteration.
		bool last_iteration = !some_stream_is_large;
        
		// We have not seen a large stream yet.
		some_stream_is_large = false;
        
		// Iterate over the streams by reading their names out of
		// stream_name and recursing on each one.

		name_stream->seek(0);
        
		while (1) {
		    // The range of keys in the stream being read.
		    key_range stream_range;
		    char *pc_read;
		    char *pc;
		    char stream_name[255];
            
		    // Read the next stream name.  We start by reading the
		    // first character and checking for an EOS condition
		    // indicating there are no more streams.
            
		    pc = stream_name;
		    ae = name_stream->read_item(&pc_read);
		    if (ae == END_OF_STREAM) {
			// We hit the end of the stream name stream, so we
			// should break out of the while loop over the streams
			// that are named.
			break;
		    } else if (ae != NO_ERROR) {
			return ae;
		    }
		    *pc = *pc_read;

		    // Now loop to read the rest of the name.

		    while (*pc != '\0') {
			ae = name_stream->read_item(&pc_read);
			if (ae != NO_ERROR) {
			    return ae;
			}
			*(++pc) = *pc_read;

			tp_assert(pc < stream_name+254, "Read too far.");
		    } 
            
		    // Read its range.
		    {
			TPIE_OS_OFFSET range_size = sizeof(stream_range);
			ae = name_stream->read_array(reinterpret_cast<char*>(&stream_range),
						     &range_size);
			if (ae != NO_ERROR) {
			    return ae;
			}
		    }
            
		    // Read its length.
		    {
			TPIE_OS_OFFSET length_size = sizeof(stream_len);
			ae = name_stream->read_array(reinterpret_cast<char*>(&stream_len),
						     &length_size);
			if (ae != NO_ERROR) {
			    return ae;
			}            
		    }
            
		    if (!last_iteration) {
			if ((stream_len * (sizeof(T) + 4 +
					   sizeof(bucket_list_elem<T> *) + 
					   sizeof(bucket_list_elem<T>)) >
			     sz_avail) &&
			    (stream_range.get_max() > stream_range.get_min() + 1)) {

			    // If it is too big but does not contain all the same key,
			    // distribute it again.
                
			    some_stream_is_large = true;
                    
			    stream<T> curr_stream(stream_name);
                
			    // We only need to read the intermediate stream once.

			    curr_stream.persist(PERSIST_READ_ONCE);
                    
			    ae = _KB_DIST(KB_KEY)(curr_stream, *name_stream2,
						  stream_range, max_size);
			    if (ae != NO_ERROR) {
				return ae;
			    }

			} else {

			    // It is either small or contains only a single
			    // key, so just pass it without further
			    // processing.
                    
			    // Write the name.
			    ae = name_stream2->write_array(stream_name,
							   strlen(stream_name) + 1);
			    if (ae != NO_ERROR) {
				return ae;
			    }

			    // Write the range.
                    
			    ae = name_stream2->write_array(reinterpret_cast<const char*>(&stream_range),
							   sizeof(stream_range));
			    if (ae != NO_ERROR) {
				return ae;
			    }
                    
			    // Write the length.
                    
			    ae = name_stream2->write_array(reinterpret_cast<const char*>(&stream_len),
							   sizeof(stream_len));
			    if (ae != NO_ERROR) {
				return ae;
			    }    
                    
			}
                
		    } else {
			// This is the last iteration.

			// Open the stream.

			stream<T> curr_stream(stream_name);

			// We only need to read the intermediate stream once.

			curr_stream.persist(PERSIST_READ_ONCE);
                    
			// Check whether it is truly small or just all one key.
                
			if (stream_range.get_min() == stream_range.get_max()) {

			    // If it is all one key, simply concatenate it
			    // onto the output.

			    T *pt;

			    while(1) {
				ae = curr_stream.read_item(&pt);
				if (ae == END_OF_STREAM) {
				    break;
				} else if (ae != NO_ERROR) {
				    return ae;
				}
				ae = outstream.write_item(*pt);
				if (ae != NO_ERROR) {
				    return ae;
				}
			    }
                    
			} else {

			    // Read the contents into main memory; sort them,
			    // and write them out.
                    
			    ae = _MM_KB_SORT(KB_KEY)(curr_stream, outstream,
						     stream_range);
			    if (ae != NO_ERROR) {
				return ae;
			    }
			}                
		    }
		}
        
		// We just finished reading all named streams at the current
		// level of recursion.  Now we want to set up for the next
		// level.  To do this, we get rid of the names we just
		// processed and replace them by the names we need to process
		// next.  Of course, this is only done if we are not in the last
		// iteration.
        
		tpie_delete(name_stream);

		if (!last_iteration) {
		    name_stream = name_stream2;
		    name_stream2 = NULL;
		} else {
			tpie_delete(name_stream2);
		    break;
		}
	    }


	    return NO_ERROR;
	}


// Main memory key bucket sorting.

	template<class T>
	err _MM_KB_SORT(KB_KEY)(stream<T> &instream,
                                stream<T> &outstream,
                                const key_range &range)
	{
	    err ae;
    
	    size_t sz_avail;
	    TPIE_OS_OFFSET stream_len;

	    TPIE_OS_SIZE_T ii,jj;
    
	    // Check available main memory.
	    sz_avail = get_memory_manager().available ();
    
	    // How long is the input stream?
	    stream_len = instream.stream_len();

	    // Verify that we have enough memory.
	    if (sz_avail < stream_len * (sizeof(T) +
					 sizeof(bucket_list_elem<T> *) + 
					 sizeof(bucket_list_elem<T>))) {
		std::cerr << '\n' 
			  << static_cast<TPIE_OS_LONGLONG>(sz_avail)
			  << ' ' 
			  << static_cast<TPIE_OS_LONGLONG>(stream_len)
			  << '\n';
		std::cerr << sizeof(T) 
			  << ' ' 
			  << sizeof(bucket_list_elem<T> *) 
			  << ' ' << sizeof(bucket_list_elem<T>);

		return INSUFFICIENT_MAIN_MEMORY;
	    }
    
	    // Allocate the space for the data and for the bucket list elements.
	    // We know that stream_len items fit in main memory, so it is safe to cast.
	    T *indata = tpie_new_array<T>(static_cast<TPIE_OS_SIZE_T>(stream_len));
    
	    bucket_list_elem<T> **buckets = tpie_new_array<bucket_list_elem<T> *>(
			static_cast<TPIE_OS_SIZE_T>(stream_len));

	    bucket_list_elem<T> *list_space = tpie_new_array<bucket_list_elem<T> >(
			static_cast<TPIE_OS_SIZE_T>(stream_len) );

	    // Read the input stream.

	    instream.seek(0);
	    {
		TPIE_OS_OFFSET sl2 = stream_len;
		ae = instream.read_array(indata, &sl2);
		if (ae != NO_ERROR) {
		    tpie_delete_array(indata, stream_len);
		    tpie_delete_array(buckets, stream_len);
		    tpie_delete_array(list_space, stream_len);
		    return ae;
		}
	    }
    
	    // Empty out the buckets.

	    for (ii = static_cast<TPIE_OS_SIZE_T>(stream_len); ii--; ) {
		buckets[ii] = NULL;
	    }
    
	    // Scan the input, assigning each item to the appropriate bucket.

	    bucket_list_elem<T> *list_elem;
    
	    unsigned int bucket_index_denom = static_cast<unsigned int>(((range.get_max() - range.get_min()) /
									 stream_len) + 1);

	    if (!bucket_index_denom) {
		bucket_index_denom = 1;
	    }

	    for (ii = static_cast<TPIE_OS_SIZE_T>(stream_len), list_elem = list_space;
		 ii--; list_elem++ ) {
		unsigned int bucket_index;

		bucket_index = (static_cast<unsigned int>(static_cast<KB_KEY>(indata[ii])) - range.get_min()) /
		    bucket_index_denom; 

		tp_assert(bucket_index < static_cast<unsigned long>(stream_len), "Bucket index too large.");
        
		list_elem->data = indata[ii];
		list_elem->next = buckets[bucket_index];
		buckets[bucket_index] = list_elem;
	    }

	    tp_assert(list_elem == list_space + stream_len,
		      "Didn't use the right number of list elements.");

    
	    // Scan the buckets to put the data back in the input array.  In
	    // order to make the sort stable, we have to be sure to read the
	    // lists corresponding to the buckets in the correct order.  The
	    // elements that appeared earlier in the orginal data were written
	    // later (i.e. towards the front of the lists) so we should
	    // rebuild the data array from the 0th element upwards.
    
#define VERIFY_OCCUPANCY 0
#if VERIFY_OCCUPANCY
	    unsigned int max_occupancy = 0;
	    unsigned int buckets_occupied = 0;
#endif        
	    for (ii = 0, jj = 0; ii < static_cast<TPIE_OS_SIZE_T>(stream_len); ii++) {
#if VERIFY_OCCUPANCY
		unsigned int cur_occupancy = 0;
#endif        
		for (list_elem = buckets[ii]; list_elem != NULL;
		     list_elem = list_elem->next) {
#if VERIFY_OCCUPANCY
		    cur_occupancy++;
#endif                    
		    indata[jj++] = list_elem->data; 
		}
#if VERIFY_OCCUPANCY
		if (cur_occupancy > max_occupancy) {
		    max_occupancy = cur_occupancy;
		}
		if (cur_occupancy != 0) {
		    buckets_occupied++;
		}
#endif                    
	    }
    
#if VERIFY_OCCUPANCY
	    std::cerr << "Max occupancy = " << max_occupancy << '\n';
	    std::cerr << "Buckets occupied = " << buckets_occupied << '\n';
	    std::cerr << "Stream length = " << stream_len << '\n';
#endif
    
	    // Do an insertion sort across the whole data set.

	    {
		T *p, *q, test;

		for (p = indata + 1; p < indata + stream_len; p++) {
		    for (q = p - 1, test = *p;
			 (q >= indata) && (KB_KEY(*q) > KB_KEY(test)); q--) {
			*(q+1) = *q;
		    }
		    *(q+1) = test;
		}
	    }
    
	    // Write the results.

	    ae = outstream.write_array(indata, static_cast<TPIE_OS_SIZE_T>(stream_len));
		tpie_delete_array(indata, stream_len);
		tpie_delete_array(buckets, stream_len);
		tpie_delete_array(list_space, stream_len);
	    if (ae != NO_ERROR) {
		return ae;
	    }    
    
	    return NO_ERROR;
	}

#ifdef _HAVE_TEMP_KB_KEY_DEFINITION_
#undef _HAVE_TEMP_KB_KEY_DEFINITION_
#undef KB_KEY
#endif

#ifdef _KB_CONCAT
#undef _KB_CONCAT
#endif
#undef _KB_SORT
#undef _KB_MM_SORT
#undef _KB_MM_DIST

    }  //   ami namespace

}  //  tpie namespace

#endif // !(defined(_AMI_KB_SORT_H)) || defined(KB_KEY)
