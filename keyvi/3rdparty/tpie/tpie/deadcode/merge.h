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

#ifndef _TPIE_AMI_MERGE_H
#define _TPIE_AMI_MERGE_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie/merge.h
/// Merge management objects.
///////////////////////////////////////////////////////////////////////////


// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// For log() and such as needed to compute tree heights.
#include <cmath>

#include <tpie/stream.h>

#include <tpie/memory.h>
#include <tpie/tpie_assert.h>

namespace tpie {

    namespace ami {
	
  /** \deprecated */
	enum merge_output_type {
	    MERGE_OUTPUT_OVERWRITE = 1,
	    MERGE_OUTPUT_APPEND
	};

    }  //  ami namespace

}  //  tpie namespace

namespace tpie {
	
  /** Intended to signal in a merge which of the input streams are non-empty */ 
	typedef int            merge_flag;
	/** Intended to signal the number of input streams in a merge */ 
	typedef TPIE_OS_SIZE_T arity_t;

    namespace ami {

		using tpie::merge_flag;
		using tpie::arity_t;
	
#define CONST const


	template<class T> class merge_base;


  ///////////////////////////////////////////////////////////////////////////
  /// Merges arity streams using a merge management object and writes
  /// result into outstream. It is assumed that the available memory can
  /// fit the arity streams, the output stream and also the space
  /// required by the merge management object; merge() checks this and
  /// then calls single_merge();
  ///////////////////////////////////////////////////////////////////////////
	template<class T, class M>
	err merge(stream<T> **instreams, arity_t arity,
		  stream<T> *outstream, M *m_obj);


  ///////////////////////////////////////////////////////////////////////////
	/// Partitions a stream into substreams small enough to fit
  /// in main memory, operates on each in main memory, and then merges them
  /// together, possibly in several passes if low memory conditions dictate.
  /// This function takes three arguments: \p instream  points to
  /// the input stream,  \p outstream points to the output stream,
  /// and \p mo points to a merge management object that controls
  /// the merge.  This function takes care of all the details of determining
  /// how much main memory is available, how big the initial substreams can
  /// be, how many streams can be merged at a time, and how many levels of
  /// merging must take place.
  /// 
  /// In order to complete the merge successfully, the function needs
  /// sufficient memory for a binary merge. If not enough memory is
  /// available, the function fails and it returns
  /// \ref INSUFFICIENT_MAIN_MEMORY. Otherwise, it returns
  /// \ref NO_ERROR.
	
	
  /// \par Merge Management Objects for partition_and_merge:
  /// The partition_and_merge() entry point requires a merge
  /// management object similar to the one described 
  /// \ref merge_management_object	"here". 
  /// The following three additional member
  /// functions must also be provided.
  /// \par main_mem_operate():
  /// <tt> err main_mem_operate(T* mm_stream, size_t len);</tt>
  /// where
	/// \p mm_stream is a pointer to an array of objects that have been read into 
	/// main memory, \p len is the number of objects in the array.
	/// This function is called by AMI_partition_and_merge()
  /// when a substream of the data is small enough to fit into main
  /// memory, and the (application-specific) processing of this subset
  /// of the data can therefore be completed in internal memory.
  /// \par space_usage_per_stream():
  /// <tt> size_t space_usage_per_stream(void); </tt>
  /// This function should return the amount of main memory that the merge
  /// management object will need per per input stream. Merge management
  /// objects are allowed to maintain data structures whose size is linear
  /// in the number of input streams being processed.
  ///  \par space_usage_overhead():
	/// <tt>size_t space_usage_overhead(void);</tt>
  /// This function should return an upper bound on the number of bytes of
  /// main memory the merge management object will allocate in addition to
  /// the portion that is linear in the number of streams.
  ///////////////////////////////////////////////////////////////////////////
	template<class T, class M>
	err partition_and_merge(stream<T> *instream,
				stream<T> *outstream, M *m_obj);

	///////////////////////////////////////////////////////////////////////////
	/// Merges <var>arity</var> streams in memory using a merge management
	/// object and write result into <var>outstream</var>.
	///////////////////////////////////////////////////////////////////////////
  template<class T, class M>
  err  single_merge(stream<T> **instreams, arity_t arity,
			  stream<T> *outstream, M *m_obj);


  ///////////////////////////////////////////////////////////////////////////
  /// Reads <var>instream</var> in memory and merges it using
  /// m_obj->main_mem_operate(); if <var>instream</var> does not fit in main
  /// memory returns INSUFFICIENT_MAIN_MEMORY;
  /////////////////////////////////////////////////////////////////////////////
	template<class T, class M>
	err main_mem_merge(stream<T> *instream,
			   stream<T> *outstream, M *m_obj);







  ///////////////////////////////////////////////////////////////////////////
  /// Superclass for merge management objects.
  /// \anchor merge_management_object
  /// A merge management object class must inherit from
  /// merge_base:
  ///
  /// <tt>template<class T> class MergeMgr: public merge_base;</tt>
  ///
  /// In addition, a merge management object must provide
  /// \ref initialize() and \ref operate() member functions,
  /// whose purposes are analogous to their namesakes for scan management
  /// objects.
	///
	/// \anchor initialize() \par initialize()
	/// The user's initialize() member function is called by the merge
  /// function once so that application-specific data structures (if any)
  /// can be initialized.
  ///
  /// <tt>err initialize(arity_t arity, const T * const *in, merge_flag *taken_flags,
  /// int &taken_index); </tt>
	/// 
	/// where \p arity is the number of input streams in the merge,
	/// \p in is a pointer to an array of pointers to
	/// input objects, each of which is the first objects appearing in
	/// one of the input streams,
	/// \p taken_flags an array of flags indicating which
	/// of the inputs are present (i.e.  which of the input streams is
	/// not empty), and a pointer to an output object.
	///
	/// The typical behavior of initialize() is to place all
  /// the input objects into a data structure and then return
  /// \ref MERGE_READ_MULTIPLE to indicate that it used (and
  /// is now finished with) all of the inputs which were indicated to be
  /// valid by \p taken_flags.  initialize need not
  /// process all inputs; it can turn off any flags in
  /// \p taken_flags corresponding to inputs that should be
  /// presented to operate().  Alternatively, it can set
  /// \p taken_index to the index of a single input it
  /// processed and return \ref MERGE_CONTINUE.
  ///    
  /// \anchor operate() \par operate() 
  /// When performing a merge, TPIE
  /// relies on the application programmer to provide code to determine
  /// the order of any two application data elements, and certain other
  /// application-specific processing. By convention, TPIE expects these
  /// decisions to be made by the operate() function:
  /// 
  /// <tt>err operate(const T * const *in, merge_flag *taken_flags, int &taken_index, T *out);</tt>
  ///
  /// The operate() member function is called repeatedly to
  /// process input objects. Typically, operate() will choose a
  /// single input object to process, and set \p taken_index to the
  /// index of the pointer to that object in the input array. This object
  /// is then typically added to a dynamic data structure maintained by the
  /// merge management object. If output is generated, for example by
  /// removing an object from the dynamic data structure,
  /// operate() should return \ref MERGE_OUTPUT,
  /// otherwise, it returns either \ref MERGE_CONTINUE to
  /// indicate that more input should be presented, or
  /// \ref MERGE_DONE to indicate that the merge has completed.
  /// 
  /// Alternatively, operate() can clear the elements of
  /// \p taken_flags that correspond to inputs it does not
  /// currently wish to process, and then return
  /// \ref  MERGE_READ_MULTIPLE. This is generally undesirable
  /// because, if only one input is taken, it is far slower than using
  /// \p taken_index to indicate which input was taken. The merge
  /// management object must clear all other flags, and then TPIE must test
  /// all the flags to see which inputs were or were not processed.
  ///////////////////////////////////////////////////////////////////////////
	template<class T>
	class merge_base {
	public:

#if VIRTUAL_BASE
	    virtual err initialize(arity_t arity,
				   CONST T * CONST * in,
				   merge_flag *taken_flags,
				   int &taken_index) = 0;
	    virtual err operate(CONST T * CONST *in,
				merge_flag *taken_flags,
				int &taken_index,
				T *out) = 0;
	    virtual err main_mem_operate(T* mm_stream, TPIE_OS_SIZE_T len) = 0;
	    virtual TPIE_OS_SIZE_T space_usage_overhead(void) = 0;
	    virtual TPIE_OS_SIZE_T space_usage_per_stream(void) = 0;
#endif // VIRTUAL_BASE

	};




	template<class T, class M>
	err 
	merge(stream<T> **instreams, arity_t arity,
	      stream<T> *outstream, M *m_obj) {

	    TPIE_OS_SIZE_T sz_avail;
	    TPIE_OS_OFFSET sz_stream, sz_needed = 0;
  
	    // How much main memory is available?
	    sz_avail = consecutive_memory_available ();

	    // Iterate through the streams, finding out how much additional
	    // memory each stream will need in the worst case (the streams are
	    // in memory, but their memory usage could be smaller then the
	    // maximum one; one scenario is when the streams have been loaded
	    // from disk with no subsequent read_item/write_item operation, in
	    // which case their current memory usage is just the header block);
	    // count also the output stream
	    for (unsigned int ii = 0; ii < arity + 1; ii++) {
			instreams[ii]->main_memory_usage(&sz_stream, STREAM_USAGE_MAXIMUM);
			sz_needed += sz_stream;
			instreams[ii]->main_memory_usage(&sz_stream, STREAM_USAGE_CURRENT);
			sz_needed -= sz_stream;
	    }                              
  
	    //count the space used by the merge_management object (include
	    //overhead added to a stream)
	    sz_needed += m_obj->space_usage_overhead() + 
		arity * m_obj->space_usage_per_stream();
               
	    //streams and m_obj must fit in memory!
	    if (sz_needed >= static_cast<TPIE_OS_OFFSET>(sz_avail)) {
		TP_LOG_WARNING("Insufficient main memory to perform a merge.\n");
		return INSUFFICIENT_MAIN_MEMORY;
	    }
	    assert(sz_needed < sz_avail);
  
	    //merge streams in memory
	    return single_merge(instreams, arity, outstream, m_obj);
	};





	template<class T, class M>
	err 
	single_merge(stream<T> **instreams, arity_t arity,
		     stream<T> *outstream, M *m_obj) {

	    TPIE_OS_SIZE_T ii;
	    err ami_err;
  
	    // Create an array of pointers for the input.
	    T* *in_objects = tpie_new_array<T*>(arity);
  
	    // Create an array of flags the merge object can use to ask for more
	    // input from specific streams.
	    merge_flag* taken_flags  = tpie_new_array<merge_flag>(arity);
  
	    // An index to speed things up when the merge object takes only from
	    // one index.
	    int taken_index;
  
	    //Output of the merge object.
	    T merge_out;
  
#if DEBUG_PERFECT_MERGE
	    unsigned int input_count = 0, output_count = 0;
#endif    
  
	    // Rewind and read the first item from every stream; count the
	    // number of non-null items read
	    for (ii = arity; ii--; ) {
		if ((ami_err = instreams[ii]->seek(0)) != NO_ERROR) {
		    tpie_delete_array(in_objects, arity);
			tpie_delete_array(taken_flags, arity);
		    return ami_err;
		}
		if ((ami_err = instreams[ii]->read_item(&(in_objects[ii]))) !=
		    NO_ERROR) {
		    //error on read
		    if (ami_err == END_OF_STREAM) {
			in_objects[ii] = NULL;
		    } else {
				tpie_delete_array(in_objects, arity);
				tpie_delete_array(taken_flags, arity);
			return ami_err;
		    }
		    // Set the taken flags to 0 before we call intialize()
		    taken_flags[ii] = 0;
		} else {
		    //item read succesfully
#if DEBUG_PERFECT_MERGE
		    input_count++;
#endif                
		}
	    }
  
	    // Initialize the merge object.
	    if (((ami_err = m_obj->initialize(arity, in_objects, taken_flags, 
					      taken_index)) != NO_ERROR) &&
		(ami_err != MERGE_READ_MULTIPLE)) {
		return OBJECT_INITIALIZATION;
	    }      
  
  
	    // Now simply call the merge object repeatedly until it claims to
	    // be done or generates an error.
	    while (1) {
		if (ami_err == MERGE_READ_MULTIPLE) {
		    for (ii = arity; ii--; ) {
			if (taken_flags[ii]) {
			    ami_err = instreams[ii]->read_item(&(in_objects[ii]));
			    if (ami_err != NO_ERROR) {
				if (ami_err == END_OF_STREAM) {
				    in_objects[ii] = NULL;
				} else {
					tpie_delete_array(in_objects, arity);
					tpie_delete_array(taken_flags, arity);
				    return ami_err;
				}
			    } else {
#if DEBUG_PERFECT_MERGE                    
				input_count++;
#endif
			    }
			}
			// Clear all flags before operate is called.
			taken_flags[ii] = 0;
		    }
		} else {
		    // The last call took at most one item.
		    if (taken_index >= 0) {
			ami_err = instreams[taken_index]->
			    read_item(&(in_objects[taken_index]));
			if (ami_err != NO_ERROR) {
			    if (ami_err == END_OF_STREAM) {
				in_objects[taken_index] = NULL;
			    } else {
					tpie_delete_array(in_objects, arity);
					tpie_delete_array(taken_flags, arity);
				return ami_err;
			    }
			} else {
#if DEBUG_PERFECT_MERGE                    
			    input_count++;
#endif
			}
			taken_flags[taken_index] = 0;
		    }
		}
		ami_err = m_obj->operate(in_objects, taken_flags, taken_index,
					 &merge_out);
		if (ami_err == MERGE_DONE) {
		    break;
		} else if (ami_err == MERGE_OUTPUT) {
#if DEBUG_PERFECT_MERGE
		    output_count++;
#endif                    
		    if ((ami_err = outstream->write_item(merge_out)) !=
			NO_ERROR) {
				tpie_delete_array(in_objects, arity);
				tpie_delete_array(taken_flags, arity);
			return ami_err;
		    }            
		} else if ((ami_err != MERGE_CONTINUE) &&
			   (ami_err != MERGE_READ_MULTIPLE)) {
			tpie_delete_array(in_objects, arity);
			tpie_delete_array(taken_flags, arity);
		    return ami_err;
		}
	    }
  
#if DEBUG_PERFECT_MERGE
	    tp_assert(input_count == output_count,
		      "Merge done, input_count = " << input_count <<
		      ", output_count = " << output_count << '.');
#endif

		tpie_delete_array(in_objects, arity);
		tpie_delete_array(taken_flags, arity);

	    return NO_ERROR;
	};





	template<class T, class M>
	err main_mem_merge(stream<T> *instream,
			   stream<T> *outstream, M *m_obj)  {

	    err ae;
	    TPIE_OS_OFFSET len;
	    TPIE_OS_SIZE_T sz_avail;
  
	    // How much memory is available?
	    sz_avail = consecutive_memory_available ();

	    len = instream->stream_len();
	    if ((len * static_cast<TPIE_OS_OFFSET>(sizeof(T))) <= static_cast<TPIE_OS_OFFSET>(sz_avail)) {
    
		// If the whole input can fit in main memory just call
		// m_obj->main_mem_operate
    
		ae = instream->seek(0);
		assert(ae == NO_ERROR);
    
		// This code is sloppy and has to be rewritten correctly for
		// parallel buffer allocation.  It will not work with anything
		// other than a registration based memory manager.
		T *mm_stream;
		TPIE_OS_OFFSET len1;
		//allocate and read input stream in memory we know it fits, so we may cast.
		mm_stream = tpie_new_array<T>(static_cast<TPIE_OS_SIZE_T>(len));

		len1 = len;
		if ((ae = instream->read_array(mm_stream, &len1)) !=
		    NO_ERROR) {
		    return ae;
		}
		tp_assert(len1 == len, "Did not read the right amount; "
			  "Allocated space for " << len << ", read " << len1 << '.');
    
		//just call m_obj->main_mem_operate. We know that len items fit into
		//main memory, so we may cast to TPIE_OS_SIZE_T
		if ((ae = m_obj->main_mem_operate(mm_stream, 
						  static_cast<TPIE_OS_SIZE_T>(len))) !=
		    NO_ERROR) {
		    TP_LOG_WARNING_ID("main_mem_operate failed");
		    return ae;
		}

		//write array back to stream
		if ((ae = outstream->write_array(mm_stream, 
						 static_cast<TPIE_OS_SIZE_T>(len))) !=
		    NO_ERROR) {
		    TP_LOG_WARNING_ID("write array failed");
		    return ae;
		}

		tpie_delete_array(mm_stream, static_cast<TPIE_OS_SIZE_T>(len));
		return NO_ERROR;
    
	    } else {
    
		// Something went wrong.  We should not have called this
		// function, since we don't have enough main memory.
		TP_LOG_WARNING_ID("out of memory");
		return INSUFFICIENT_MAIN_MEMORY;
	    }
	};


	template<class T, class M>
	err partition_and_merge(stream<T> *instream,
				stream<T> *outstream, M *m_obj) {

	    err ae;
	    TPIE_OS_OFFSET len;
	    TPIE_OS_SIZE_T sz_avail, sz_stream;
	    unsigned int ii;
	    int jj;
  
	    //How much memory is available?
	    sz_avail = consecutive_memory_available ();

	    // If the whole input can fit in main memory then just call
	    // main_mem_merge() to deal with it by loading it once and
	    // processing it.
	    len = instream->stream_len();
	    if ((len * static_cast<TPIE_OS_OFFSET>(sizeof(T))) <= static_cast<TPIE_OS_OFFSET>(sz_avail)) {
		return main_mem_merge(instream, outstream, m_obj);
	    } 
	    //else {

  
 
	    // The number of substreams that can be merged together at once; i
	    // this many substreams (at most) we are dividing the input stream
	    arity_t merge_arity;
  
	    //nb of substreams the original input stream will be split into
	    arity_t nb_orig_substr;
  
	    // length (nb obj of type T) of the original substreams of the input
	    // stream.  The last one may be shorter than this.
	    TPIE_OS_OFFSET sz_orig_substr;
  
	    // The initial temporary stream, to which substreams of the
	    // original input stream are written.
	    stream<T> *initial_tmp_stream;

	    // A pointer to the buffer in main memory to read a memory load into.
	    T *mm_stream;
  
  
	    // Loop variables:
  
	    // The stream being read at the current level.
	    stream<T> *current_input;

	    // The output stream for the current level if it is not outstream.
	    stream<T> *intermediate_tmp_stream;
        
	    // The size of substreams of *current_input that are being
	    // merged.  The last one may be smaller.  This value should be
	    // sz_orig_substr * (merge_arity ** k) where k is the
	    // number of iterations the loop has gone through.
	    TPIE_OS_OFFSET current_substream_len;

	    // The exponenent used to verify that current_substream_len is
	    // correct.
	    unsigned int k;
  
	    TPIE_OS_OFFSET sub_start, sub_end;
  
  
  
	    // How many substreams will there be?  The main memory
	    // available to us is the total amount available, minus what
	    // is needed for the input stream and the temporary stream.
	    if ((ae = instream->main_memory_usage(&sz_stream, STREAM_USAGE_MAXIMUM)) 
		!= NO_ERROR) {
		return ae;
	    }                                     
	    if (sz_avail <= 2 * sz_stream + sizeof(T)) {
		return INSUFFICIENT_MAIN_MEMORY;
	    }
	    sz_avail -= 2 * sz_stream;

	
	    // number of elements that will fit in memory (M) -R
	    sz_orig_substr = sz_avail / sizeof(T);

	    // Round the original substream length off to an integral number of
	    // chunks.  This is for systems like HP-UX that cannot map in
	    // overlapping regions.  It is also required for BTE's that are
	    // capable of freeing chunks as they are read.
	    {
		TPIE_OS_OFFSET sz_chunk_size = instream->chunk_size();
    
		sz_orig_substr = sz_chunk_size *
		    ((sz_orig_substr + sz_chunk_size - 1) /sz_chunk_size);
		// WARNING sz_orig_substr now may not fit in memory!!! -R
	    }

	    // number of memoryloads in input ceil(N/M) -R
	    nb_orig_substr = static_cast<arity_t>((len + sz_orig_substr - 1) / 
						  sz_orig_substr);
  
	    // Account for the space that a merge object will use.
	    {
		TPIE_OS_SIZE_T sz_avail_during_merge = sz_avail - m_obj->space_usage_overhead();
		TPIE_OS_SIZE_T sz_stream_during_merge = sz_stream +m_obj->space_usage_per_stream();
    
		merge_arity = static_cast<arity_t>((sz_avail_during_merge +
						    sz_stream_during_merge - 1) / 
						   sz_stream_during_merge);
	    }

	    // Make sure that the AMI is willing to provide us with the number
	    // of substreams we want.  It may not be able to due to operating
	    // system restrictions, such as on the number of regions that can be
	    // mmap()ed in.
	    {
		int ami_available_streams = instream->available_streams();
    
		if (ami_available_streams != -1) {
		    if (ami_available_streams <= 4) {
			return INSUFFICIENT_AVAILABLE_STREAMS;
		    }
		    //  safe to cast, since ami_available_streams > 4
		    if (merge_arity > static_cast<arity_t>(ami_available_streams) - 2) {
			merge_arity = ami_available_streams - 2;
			TP_LOG_DEBUG_ID("Reduced merge arity due to AMI restrictions.");
		    }
		}
	    }
	    TP_LOG_DEBUG_ID("partition_and_merge(): merge arity = "
				<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(merge_arity));
	    if (merge_arity < 2) {
		return INSUFFICIENT_MAIN_MEMORY;
	    }
  
  
	    //#define MINIMIZE_INITIAL_SUBSTREAM_LENGTH
#ifdef MINIMIZE_INITIAL_SUBSTREAM_LENGTH
  
	    // Make the substreams as small as possible without increasing the
	    // height of the merge tree.
	    {
		// The tree height is the ceiling of the log base merge_arity
		// of the number of original substreams.
    
		double tree_height = log((double)nb_orig_substr)/ log((double)merge_arity);
		tp_assert(tree_height > 0, "Negative or zero tree height!");
    
		tree_height = ceil(tree_height);
    
		// See how many substreams we could possibly fit in the tree
		// without increasing the height.
		double max_original_substreams = pow((double)merge_arity, tree_height);
		tp_assert(max_original_substreams >= nb_orig_substr,
			  "Number of permitted substreams was reduced.");

		// How big will such substreams be?
		double new_sz_original_substream = ceil((double)len /
							max_original_substreams);
		tp_assert(new_sz_original_substream <= sz_orig_substr,
			  "Size of original streams increased.");
    
		sz_orig_substr = (size_t)new_sz_original_substream;
		TP_LOG_DEBUG_ID("Memory constraints set original substreams = " << nb_orig_substr);
    
		nb_orig_substr = (len + sz_orig_substr - 1) / sz_orig_substr;
		TP_LOG_DEBUG_ID("Tree height constraints set original substreams = " << nb_orig_substr);
	    }                
#endif // MINIMIZE_INITIAL_SUBSTREAM_LENGTH

    
	    // Create a temporary stream, then iterate through the substreams,
	    // processing each one and writing it to the corresponding substream
	    // of the temporary stream.
	    initial_tmp_stream = tpie_new<stream<T> >();
	    mm_stream = tpie_new_array<T>(static_cast<TPIE_OS_SIZE_T>(sz_orig_substr));
  
	    instream->seek(0);
	    assert(ae == NO_ERROR);

	    tp_assert(static_cast<TPIE_OS_OFFSET>(nb_orig_substr * sz_orig_substr - len) < sz_orig_substr,
		      "Total substream length too long or too many.");
	    tp_assert(len - static_cast<TPIE_OS_OFFSET>(nb_orig_substr - 1) * sz_orig_substr <= sz_orig_substr,
		      "Total substream length too short or too few.");        
  
	    for (ii = 0; ii++ < nb_orig_substr; ) {
    
		TPIE_OS_OFFSET mm_len;
		if (ii == nb_orig_substr) {
		    mm_len = len % sz_orig_substr;
		    // If it is an exact multiple, then the mod will come out 0,
		    // which is wrong.
		    if (!mm_len) {
			mm_len = sz_orig_substr;
		    }
		} else {
		    mm_len = sz_orig_substr;
		}
#ifndef TPIE_NDEBUG
		TPIE_OS_OFFSET mm_len_bak = mm_len;
#endif
    
		// Read a memory load out of the input stream.
		ae = instream->read_array(mm_stream, &mm_len);
		if (ae != NO_ERROR) {
		    return ae;
		}
		tp_assert(mm_len == mm_len_bak,
			  "Did not read the requested number of objects." <<
			  "\n\tmm_len = " << mm_len <<
			  "\n\tmm_len_bak = " << mm_len_bak << '.');
    
		// Solve in main memory. We know it fits, so cast to TPIE_OS_SIZE_T
		m_obj->main_mem_operate(mm_stream, static_cast<TPIE_OS_SIZE_T>(mm_len));
    
		// Write the result out to the temporary stream.
		ae = initial_tmp_stream->write_array(mm_stream, static_cast<TPIE_OS_SIZE_T>(mm_len));
		if (ae != NO_ERROR) {
		    return ae;
		}            
	    } //for
		tpie_delete_array(mm_stream, static_cast<TPIE_OS_SIZE_T>(sz_orig_substr));
  
	    // Make sure the total length of the temporary stream is the same as
	    // the total length of the original input stream.
	    tp_assert(instream->stream_len() == initial_tmp_stream->stream_len(),
		      "Stream lengths do not match:" <<
		      "\n\tinstream->stream_len() = " << instream->stream_len() <<
		      "\n\tinitial_tmp_stream->stream_len() = " <<
		      initial_tmp_stream->stream_len() << ".\n");
  
	    // Set up the loop invariants for the first iteration of hte main
	    // loop.
	    current_input = initial_tmp_stream;
	    current_substream_len = sz_orig_substr;
  
	    // Pointers to the substreams that will be merged.
	    stream<T>* *the_substreams = tpie_new_array<stream<T>* >(merge_arity);
  
	    //Monitoring prints.
	    TP_LOG_DEBUG_ID("Number of runs from run formation is "
			    << static_cast<TPIE_OS_OUTPUT_SIZE_T>(nb_orig_substr));
	    TP_LOG_DEBUG_ID("Merge arity is " 
				<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(merge_arity));
  
  
	    k = 0;
	    // The main loop.  At the outermost level we are looping over levels
	    // of the merge tree.  Typically this will be very small, e.g. 1-3.
	    // CAVEAT: is the cast o.k.??
	    for( ; current_substream_len < len;
		 current_substream_len *= merge_arity) {
    
		// The number of substreams to be processed at this level.
		arity_t substream_count;
    
		// Set up to process a given level.
		tp_assert(len == current_input->stream_len(),
			  "Current level stream not same length as input." <<
			  "\n\tlen = " << len <<
			  "\n\tcurrent_input->stream_len() = " <<
			  current_input->stream_len() << ".\n");
    
		// Do we have enough main memory to merge all the substreams on
		// the current level into the output stream?  If so, then we will
		// do so, if not then we need an additional level of iteration to
		// process the substreams in groups.
		substream_count = static_cast<arity_t>((len + current_substream_len - 1) /
						       current_substream_len);
    
		if (substream_count <= merge_arity) {
      
		    TP_LOG_DEBUG_ID("Merging substreams directly to the output stream.");
      
		    // Create all the substreams
		    for (sub_start = 0, ii = 0 ;
			 ii < substream_count;
			 sub_start += current_substream_len, ii++) {
	
			sub_end = sub_start + current_substream_len - 1;
			if (sub_end >= len) {
			    sub_end = len - 1;
			}
			current_input->new_substream(READ_STREAM, sub_start, sub_end, the_substreams + ii);
			// The substreams are read-once.
			the_substreams[ii]->persist(PERSIST_READ_ONCE);
		    }               
      
		    tp_assert((sub_start >= len) &&
			      (sub_start < len + current_substream_len),
			      "Loop ended in wrong location.");
      
		    // Fool the OS into unmapping the current block of the input
		    // stream so that blocks of the substreams can be mapped in
		    // without overlapping it.  This is needed for correct execution
		    // on HP-UX.
		    //this needs to be cleaned up..Laura
		    current_input->seek(0);
		    assert(ae == NO_ERROR);

		    // Merge them into the output stream.
		    ae = single_merge(the_substreams, substream_count, outstream, m_obj);
		    if (ae != NO_ERROR) {
			return ae;
		    }
		    // Delete the substreams.
		    for (ii = 0; ii < substream_count; ii++) {
				tpie_delete(the_substreams[ii]);
		    }
		    // And the current input, which is an intermediate stream of
		    // some kind.
			tpie_delete(current_input);
		} else {
      
		    //substream_count  is >  merge_arity
		    TP_LOG_DEBUG_ID("Merging substreams to an intermediate stream.");
      
		    // Create the next intermediate stream.
		    intermediate_tmp_stream = new stream<T>;
      
		    // Fool the OS into unmapping the current block of the input
		    // stream so that blocks of the substreams can be mapped in
		    // without overlapping it.  This is needed for correct execution
		    // on HU-UX.
		    //this needs to be cleaned up..Laura
		    current_input->seek(0);
		    assert(ae == NO_ERROR);

		    // Loop through the substreams of the current stream, merging as
		    // many as we can at a time until all are done with.
		    for (sub_start = 0, ii = 0, jj = 0;
			 ii < substream_count;
			 sub_start += current_substream_len, ii++, jj++) {
	
			sub_end = sub_start + current_substream_len - 1;
			if (sub_end >= len) {
			    sub_end = len - 1;
			}
			current_input->new_substream(READ_STREAM, sub_start, sub_end, the_substreams + jj); 
			// The substreams are read-once.
			the_substreams[jj]->persist(PERSIST_READ_ONCE);
                    
			// If we've got all we can handle or we've seen them all, then
			// merge them.
			if ((jj >= static_cast<int>(merge_arity) - 1) || 
			    (ii == substream_count - 1)) {
	  
			    tp_assert(jj <= static_cast<int>(merge_arity) - 1,
				      "Index got too large.");
#if DEBUG_ASSERTIONS
			    // Check the lengths before the merge.
			    TPIE_OS_OFFSET sz_output, sz_output_after_merge;
			    TPIE_OS_OFFSET sz_substream_total;
	  
			    {
				unsigned int kk;
	    
				sz_output = intermediate_tmp_stream->stream_len();
				sz_substream_total = 0;
	    
				for (kk = jj+1; kk--; ) {
				    sz_substream_total += the_substreams[kk]->stream_len();
				}                          
	    
			    }
#endif 
	  
			    // This should append to the stream, since
			    // single_merge() does not rewind the output before
			    // merging.
			    ae = single_merge(the_substreams, jj+1,
					      intermediate_tmp_stream, m_obj);
			    if (ae != NO_ERROR) {
				return ae;
			    }
	  
#if DEBUG_ASSERTIONS
			    // Verify the total lengths after the merge.
			    sz_output_after_merge = intermediate_tmp_stream->stream_len();
			    tp_assert(sz_output_after_merge - sz_output ==
				      sz_substream_total,
				      "Stream lengths do not add up: " <<
				      sz_output_after_merge - sz_output <<
				      " written when " <<
				      sz_substream_total <<
				      " were to have been read.");
                                  
#endif 
	  
			    // Delete the substreams.  jj is currently the index of the
			    // largest, so we want to bump it up before the idiomatic
			    // loop.
			    for (jj++; jj--; )
					tpie_delete(the_substreams[jj]);
	  
			    // Now jj should be -1 so that it gets bumped back up to 0
			    // before the next iteration of the outer loop.
			    tp_assert((jj == -1), "Index not reduced to -1.");
	  
			} // if                
		    } //for
      
		    // Get rid of the current input stream and use the next one.
		    tpie_delete(current_input);
		    current_input = intermediate_tmp_stream;
		}
    
		k++;
    
	    }

	    //Monitoring prints.
	    TP_LOG_DEBUG_ID("Number of passes incl run formation is " << k+1);
  
		tpie_delete_array(the_substreams, merge_arity);
		return NO_ERROR;

	}

    }  //  ami namespace

}  //  tpie namespace



#endif
