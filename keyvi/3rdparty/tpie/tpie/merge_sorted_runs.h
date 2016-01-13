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

#ifndef _MERGE_SORTED_RUNS_H
#define _MERGE_SORTED_RUNS_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie/merge_sorted_runs.h
/// merge_sorted_runs as used in several of TPIE's merge variants
/// \sa sorted_stream_merging
/// \internal \todo Document better
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// Includes needed from TPIE
#include <tpie/mergeheap.h>     //For templated heaps

#include <tpie/progress_indicator_base.h>

#include <tpie/file_stream.h>


namespace tpie {

    namespace ami {

		/** Intended to signal the number of input streams in a merge */ 
		typedef TPIE_OS_SIZE_T arity_t;
	
		//////////////////////////////////////////////////////////////////////////
		/// This is a common merge routine for all of the AMI_merge_sorted,
		/// AMI_ptr_merge_sorted and AMI_key_merge_sorted entry points. 
		///
		/// It is also used by the sort entry points AMI_sort, AMI_ptr_sort and
		/// AMI_key_sort and by the routine      
		/// AMI_partition_and_merge.  Differences are encapsulated within the      
		/// merge heap object MergeHeap. It is assumed that MergeHeap#allocate()   
		/// was called before entering ami_merge_sorted_runs.                          
		/// ami_merge_sorted_runs takes both the max number of elements to read from any 
		/// stream and also a boolean flag for showing a progress indicator.
		///
		/// \anchor sorted_stream_merging \par Sorted Stream Merging:
		/// TPIE provides several merge entry points for merging sorted streams to
		/// produce a single, interleaved output stream.
		/// merge_sorted_runs has three polymorphs, namely the
		/// comparison operator, comparison class and the key-based
		/// polymorphs. 
		/// The comparison operator version tends to be the fastest and most straightforward to
		/// use. The comparison class version is comparable in speed (maybe
		/// slightly slower), but somewhat more flexible, as it can support
		/// multiple, different merges on the same keys.
		/// \internal \todo Doc: LA: Syntax of comparator for polymorphs? key-based?
		////////////////////////////////////////////////////////////////////////////
		template <class T, class M>
		void merge_sorted_runs(typename tpie::array<tpie::unique_ptr<file_stream<T> > >::iterator start,
							   typename tpie::array<tpie::unique_ptr<file_stream<T> > >::iterator end,
							   file_stream<T> *outStream,  M* MergeHeap,
							   TPIE_OS_OFFSET cutoff=-1, 
							   progress_indicator_base* indicator = NULL) {
	    
			size_t i;
			size_t arity = end-start;

			//Pointers to current leading elements of streams
			tpie::array<const T *> in_objects(arity);
			tpie::array<TPIE_OS_OFFSET> nread(arity);
			
			// **************************************************************
			// * Read first element from stream. Do not rewind! We may read *
			// * more elements from the same stream later.                  *
			// **************************************************************

			for (i = 0; i < arity; i++) {
				file_stream<T> * stream = (start+i)->get();
				if (stream->can_read()) {
					in_objects[i] = &stream->read();
					MergeHeap->insert( in_objects[i], i );
				} else 
					in_objects[i] = NULL;
				nread[i] = 1;
				if (indicator) indicator->step();
			}

			// *********************************************************
			// * Build a heap from the smallest items of each stream   *
			// *********************************************************
	    
			MergeHeap->initialize ( );

			// *********************************************************
			// * Perform the merge until the inputs are exhausted.     *
			// *********************************************************
			while (MergeHeap->sizeofheap() > 0) {
				i = MergeHeap->get_min_run_id ();
				outStream->write(*in_objects[i]);

				bool eof = false;
		
				//Check if we read as many elements as we are allowed to
				if ( (cutoff != -1) && (nread[i]>=cutoff))
					eof = true;
				else {
					file_stream<T> * stream = (start+i)->get();
					if (stream->can_read()) in_objects[i] = &stream->read();
					else eof = true;
		    
					if (indicator) indicator->step();
				} 
		
				if (eof)
					MergeHeap->delete_min_and_insert(NULL);
				else { 
					nread[i]++;
					MergeHeap->delete_min_and_insert (in_objects[i]);
				}
			}  //  while
		}
	
	
		///////////////////////////////////////////////////////////////////////////
		/// Merging with a heap that contains the records to be merged. CMPR is
		/// the class of the comparison object, and must contain the method
		/// compare() which is called from within the merge.
		///
		/// This is one of the merge entry points for merging without the   
		/// \ref merge_management_object used by TPIE's merge.
		/// These routines perform the special case of merging when the
		/// the required output is the original records interleaved
		/// according to a comparison operator or function.
		/// \internal \todo add comparison operator version
		///////////////////////////////////////////////////////////////////////////
		template <class T, class CMPR>
		void merge_sorted(typename tpie::array<std::unique_ptr<file_stream<T> > >::iterator start,
						 typename tpie::array<std::unique_ptr<file_stream<T> > >::iterator end,
						 file_stream<T> *outStream, CMPR *cmp) {
	    
			// make a merge heap which uses the user's comparison object
			// and initialize it
			merge_heap_obj<T,CMPR> mrgheap (cmp);
			mrgheap.allocate(end-start);
	    
			//Rewind all the input streams
			for (typename tpie::array<std::unique_ptr<file_stream<T> > >::iterator i=start; 
				 i != end; ++i)
				i->seek(0); 
	    
			merge_sorted_runs(start, end, outStream, mrgheap);
		}

	
		///////////////////////////////////////////////////////////////////////////
		/// Merging with a heap that keeps a pointer to the records rather than
		/// the records themselves: \p CMPR is the class of the comparison object,
		/// and must contain the method compare()  which is called from within
		/// the merge.
		///
		/// This is one of the merge entry points for merging without the   
		/// \ref merge_management_object used by TPIE's merge.
		/// These routines perform the special case of merging when the
		/// the required output is the original records interleaved
		/// according to a comparison operator or function.
		/// \internal \todo Check that memory management is done right
		///////////////////////////////////////////////////////////////////////////
		template <class T, class CMPR>
		void ptr_merge_sorted(typename tpie::array<tpie::unique_ptr<file_stream<T> > >::iterator start,
							 typename tpie::array<tpie::unique_ptr<file_stream<T> > >::iterator end,
							 file_stream<T> *outStream, 
							 CMPR *cmp) {

			// make a merge heap of pointers which uses the user's comparison
			// object and initialize it
			merge_heap_ptr_obj<T,CMPR> mrgheap (cmp);
			mrgheap.allocate(end-start);
	    
			// Rewind all the input streams
			for (typename tpie::array<std::unique_ptr<file_stream<T> > >::iteratorarity_t i=start; 
				 i != end; ++i)
				i->seek(0); 
	    
			merge_sorted_runs(start, end, outStream, mrgheap);
		}
    }  //  ami namespace
    
}  //  tpie namespace

#endif //_MERGE_SORTED_RUNS_H
