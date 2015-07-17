// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008-2012, The TPIE development team
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

#ifndef __TPIE_SORT_DEPRECATED_H__
#define __TPIE_SORT_DEPRECATED_H__

///////////////////////////////////////////////////////////////////////////
/// \file sort_deprecated.h
///  Contains deprecated sorting algorithms.
/// \anchor sorting_in_tpie \par Sorting in TPIE:
/// TPIE offers three
/// merge sorting variants. The user must decide which variant is most
/// appropriate for their circumstances. All accomplish the same goal,
/// but the performance can vary depending on the situation. They differ
/// mainly in the way they perform the merge phase of merge sort,
/// specifically how they maintain their heap data structure used in the
/// merge phase. The three variants are the following: 
/// sort(), ptr_sort(), key_sort().
///
///
/// \par sort()
/// keeps the (entire) first record of each
/// sorted run (each is a stream) in a heap. This approach is most
/// suitable when the record consists entirely of the record key.
/// 
/// \par ptr_sort() 
/// keeps a pointer to the first record of
/// each stream in the heap. This approach works best when records are
/// very long and the key field(s) take up a large percentage of the
/// record.
///
/// \par key_sort()
/// keeps the key field(s) and a pointer
/// to the first record of each stream in the heap. This approach works
/// best when the key field(s) are small in comparison to the record
/// size.
/// 
/// Any of these variants will accomplish the task of sorting an input
/// stream in an I/O efficient way, but there can be noticeable
/// differences in processing time between the variants. As an example,
/// key_sort() appears to be more cache-efficient than the
/// others in many cases, and therefore often uses less processor time,
/// despite extra data movement relative to ptr_sort().
/// 
/// In addition to the three variants discussed above, there are multiple
/// choices within each variant regarding how the actual comparison
/// operations are to be performed. These choices are described in some detail
/// for sort().
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// The class that actually does the sorting
#include <tpie/sort_manager.h>
#include <tpie/mergeheap.h>
#include <tpie/internal_sort.h>

#include <tpie/progress_indicator_base.h>
#include <tpie/progress_indicator_null.h>

#include <tpie/sort.h>

#include <tpie/comparator.h>

namespace tpie {

	namespace ami {
		inline err exception_kind(const exception & e) {
			if (0 != dynamic_cast<const end_of_stream_exception *>(&e)) return END_OF_STREAM;
			return BTE_ERROR;
		}
	}
  //////////////////////////////////////////////////////////////////////////
  /// A version of sort that takes an input stream of elements of type
  /// T, and an output stream, and and uses the < operator to sort,
  /// see also \ref sorting_in_tpie "Sorting in TPIE".
  /// 
  /// \anchor comp_sorting \par Comparing within Sorting:
  /// TPIE's sort() has two polymorphs, namely the comparison
  /// operator and comparison class polymorphs. The comparison operator
  /// version tends to be the fastest and most straightforward to use. The
  /// comparison class version is comparable in speed (maybe slightly
  /// slower), but somewhat more flexible, as it can support multiple,
  /// different sorts on the same keys.
  /// 
  /// \par Comparison operator version:
  /// This version works on streams of
  /// objects for which the operator "<" is defined.
  /// 
  /// \par Comparison class version:
  /// This version of sort() uses a method of a user-defined
  /// comparison object to determine the order of two input objects. The
  /// object must have a public member function named compare(),
  /// having the following prototype:
  ///
  /// <tt>inline int compare (const KEY & k1, const KEY & k2);</tt>
  /// 
  /// The user-written compare() function computes the order of
  /// the two user-defined keys \p k1  and \p k2, and
  /// returns -1, 0, or +1 to indicate that <tt>k1<k2</tt>, <tt>k1==k2</tt>, or
  /// <tt>k1>k2</tt> respectively.
  ///
  /// \internal \deprecated \par  Deprecated Comparison Function Sorting:
  /// Earlier TPIE versions allowed a sort that used a C-style
  /// comparison function to sort. However, comparison functions cannot be
  /// inlined, so each comparison requires one function call. Given that the
  /// comparison operator < and comparison object classes can be inlined and
  /// have better performance while providing the exact same functionality,
  /// comparison functions have been removed from TPIE. If you can provide us
  /// with a compelling argument on why they should be in here, we may consider
  /// adding them again, but you must demonstrate that comparision functions
  /// can outperform other methods in at least some cases or give an example
  /// were it is impossible to use a comparison operator or comparison object.
  //////////////////////////////////////////////////////////////////////////

    namespace ami {

	template<class T>
	err sort(stream<T> *instream_ami, stream<T> *outstream_ami,
		 tpie::progress_indicator_base* indicator=NULL)	{
		try {
			tpie::sort(instream_ami->underlying_stream(), outstream_ami->underlying_stream(), indicator);
		} catch (const exception & e) {
			TP_LOG_FATAL_ID(e.what());
			return exception_kind(e);
		}
		return NO_ERROR;
	}

	}

  ///////////////////////////////////////////////////////////////////////////
  /// A version of sort that takes an input stream of elements of
  /// type T, an output stream, and a user-specified comparison
  /// object. The comparison object "cmp", of (user-defined) class
  /// represented by CMPR, must have a member function called "compare"
  /// which is used for sorting the input stream; see also 
  /// \ref comp_sorting "Comparing within Sorting".
  ///////////////////////////////////////////////////////////////////////////

  namespace ami {
  template<class T, class CMPR>
  err sort(stream<T> *instream_ami, stream<T> *outstream_ami,
		 CMPR *cmp, progress_indicator_base* indicator=NULL) {
		try {
			TPIE2STL_cmp<T, CMPR> comp(cmp);
			if (indicator) {
				tpie::sort(instream_ami->underlying_stream(), outstream_ami->underlying_stream(), comp, *indicator);
			} else {
				progress_indicator_null dummy(1);
				tpie::sort(instream_ami->underlying_stream(), outstream_ami->underlying_stream(), comp, dummy);
			}
		} catch (const exception & e) {
			TP_LOG_FATAL_ID(e.what());
			return exception_kind(e);
		}
		return NO_ERROR;
	}
  }

// --------------------------------------------------------------------
// -                                                                  -
// -  These versions used to build a heap on pointers to objects      -
// -  but have been deprecated, since they have been broken for       -
// -  many years and caused unnecessary code duplication.             -
// -                                                                  -
// --------------------------------------------------------------------

	namespace ami {
	template<class T>
	err ptr_sort(stream<T> *instream_ami, stream<T> *outstream_ami,
			   progress_indicator_base* indicator=NULL) {

		TP_LOG_WARNING_ID("tpie::ami::ptr_sort is deprecated");
		return sort(instream_ami, outstream_ami, indicator);
	}
	}
	
	namespace ami {
	template<class T, class CMPR>
	err ptr_sort(stream<T> *instream_ami, stream<T> *outstream_ami,
		     CMPR *cmp, progress_indicator_base* indicator=NULL) {
		TP_LOG_WARNING_ID("tpie::ami::ptr_sort is deprecated");
		return sort(instream_ami, outstream_ami, cmp, indicator);
	}
	}

// ********************************************************************
// *                                                                  *
// *  This version used to keep a heap of keys and pointers, but has  *
// *  been deprecated since nobody uses it.                           *
// *                                                                  *
// ********************************************************************

	namespace ami {
	template<class T, class KEY, class CMPR>
	err  key_sort(stream<T> *instream_ami, stream<T> *outstream_ami,
		      KEY, CMPR *cmp, progress_indicator_base* indicator=NULL)	{
		TP_LOG_WARNING_ID("tpie::ami::key_sort is deprecated");
		return sort(instream_ami, outstream_ami, cmp, indicator);
	}
	}

// ********************************************************************
// *                                                                  *
// * Duplicates of the above versions that only use 2x space and      *
// * overwrite the original input stream                              *
// *                                                                  *
// ********************************************************************/

  ///////////////////////////////////////////////////////////////////////////
  /// In-place sorting variant of \ref sort(stream<T> *instream_ami, stream<T> *outstream_ami, tpie::progress_indicator_base* indicator=NULL),
  /// see also \ref sortingspace_in_tpie "In-place Variants for Sorting in TPIE".
  ///////////////////////////////////////////////////////////////////////////
	namespace ami {
	template<class T>
	err sort(stream<T> *instream_ami, 
		 progress_indicator_base* indicator=0) {
		try {
			if (indicator) {
				tpie::sort(instream_ami->underlying_stream(), *indicator);
			} else {
				progress_indicator_null dummy(1);
				tpie::sort(instream_ami->underlying_stream(), dummy);
			}
		} catch (const exception & e) {
			TP_LOG_FATAL_ID(e.what());
			return exception_kind(e);
		}
		return NO_ERROR;
	}
	}
	

	///////////////////////////////////////////////////////////////////////////
	/// In-place sorting variant of \ref  sort(stream<T> *instream_ami, stream<T> *outstream_ami, CMPR *cmp, progress_indicator_base* indicator=NULL), 
	/// see also \ref sortingspace_in_tpie "In-place Variants for Sorting in TPIE".
	///////////////////////////////////////////////////////////////////////////
	namespace ami {
	template<class T, class CMPR>
	err sort(stream<T> *instream_ami, 
		 CMPR *cmp, progress_indicator_base* indicator=NULL) {
		try {
			TPIE2STL_cmp<T, CMPR> comp(cmp);
			if (indicator) {
				tpie::sort(instream_ami->underlying_stream(), comp, *indicator);
			} else {
				progress_indicator_null dummy(1);
				tpie::sort(instream_ami->underlying_stream(), comp, dummy);
			}
		} catch (const exception & e) {
			TP_LOG_FATAL_ID(e.what());
			return exception_kind(e);
		}
		return NO_ERROR;
	}

	}

  ///////////////////////////////////////////////////////////////////////////
  /// In-place sorting variant of \ref ptr_sort(stream<T> *instream_ami, stream<T> *outstream_ami, progress_indicator_base* indicator=NULL),
  /// see also \ref sortingspace_in_tpie "In-place Variants for Sorting in TPIE".
  ///////////////////////////////////////////////////////////////////////////
	namespace ami {
	template<class T>
	err ptr_sort(stream<T> *instream_ami, 
		     progress_indicator_base* indicator=NULL) {
		TP_LOG_WARNING_ID("tpie::ami::ptr_sort is deprecated");
		return sort(instream_ami, indicator);
	}
	}

  ///////////////////////////////////////////////////////////////////////////
  /// In-place sorting variant of \ref ptr_sort(stream<T> *instream_ami, stream<T> *outstream_ami, CMPR *cmp, progress_indicator_base* indicator=NULL),
  /// see also \ref sortingspace_in_tpie "In-place Variants for Sorting in TPIE".
  ///////////////////////////////////////////////////////////////////////////
	namespace ami {
	template<class T, class CMPR>
	err ptr_sort(stream<T> *instream_ami, 
		     CMPR *cmp, progress_indicator_base* indicator=NULL) {
		TP_LOG_WARNING_ID("tpie::ami::ptr_sort is deprecated");
		return sort(instream_ami, cmp, indicator);
	}
	}

  ///////////////////////////////////////////////////////////////////////////
  /// In-place sorting variant of \ref key_sort(stream<T> *instream_ami, stream<T> *outstream_ami, KEY dummykey, CMPR *cmp, progress_indicator_base* indicator=NULL),
  /// see also \ref sortingspace_in_tpie "In-place Variants for Sorting in TPIE".
  ///////////////////////////////////////////////////////////////////////////
	namespace ami {
	template<class T, class KEY, class CMPR>
	err  key_sort(stream<T> *instream_ami, 
		      KEY /*dummykey*/, CMPR *cmp, progress_indicator_base* indicator=NULL)	{
		TP_LOG_WARNING_ID("tpie::ami::key_sort is deprecated");
		return sort(instream_ami, cmp, indicator);
	}
    }

}  //  tpie namespace


#endif // __TPIE_SORT_DEPRECATED_H__
