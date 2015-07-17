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

#ifndef _AMI_SORT_H
#define _AMI_SORT_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie/sort.h Sorting algorithms.
/// \anchor sortingspace_in_tpie \par In-place Variants for Sorting in TPIE:
/// TPIE can sort given an input stream and output stream,
/// or just an input stream. When just an input stream is specified, the
/// original input elements are deleted the input stream is rewritten with the
/// sorted output elements. If both the input stream and output stream are
/// specified, the original input elements are saved. During sorting, a
/// temporary copy of each element is stored on disk as part of intermediate
/// sorting results. If N is the size on disk of the original input stream,
/// the polymorphs of sorting with both input and output streams use 3N
/// space, whereas if just an input stream is specified, 2N space is used.
/// If the original unsorted input stream is not needed after sorting, it is
/// recommended that users use the sort() polymorph with with just
/// an input stream, to save space and avoid having to maintain both an input
/// and output stream. 
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// The typename that actually does the sorting
#include <tpie/sort_manager.h>
#include <tpie/mergeheap.h>
#include <tpie/internal_sort.h>

#include <tpie/progress_indicator_base.h>
#include <tpie/progress_indicator_null.h>
#include <tpie/fractional_progress.h>

#include <tpie/pipelining/merge_sorter.h>
#include <tpie/file_stream.h>
#include <tpie/uncompressed_stream.h>

namespace tpie {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream in-place using the given STL-style
/// comparator object.
///////////////////////////////////////////////////////////////////////////////
template<typename Stream, typename T, typename Compare>
void generic_sort(Stream & instream, Compare comp,
				  progress_indicator_base * indicator) {

	stream_size_type sz = instream.size();

	fractional_progress fp(indicator);
	fractional_subindicator push(fp, "sort", TPIE_FSI, sz, "Write sorted runs");
	fractional_subindicator merge(fp, "sort", TPIE_FSI, sz, "Perform merge heap");
	fractional_subindicator output(fp, "sort", TPIE_FSI, sz, "Write sorted output");
	fp.init(sz);

	instream.seek(0);

	merge_sorter<T, true, Compare> s(comp);
	s.set_available_memory(get_memory_manager().available());
	s.begin();
	push.init(sz);
	while (instream.can_read()) s.push(instream.read()), push.step();
	push.done();
	s.end();

	instream.truncate(0);
	s.calc(merge);

	output.init(sz);
	while (s.can_pull()) instream.write(s.pull()), output.step();
	output.done();
	fp.done();
	instream.seek(0);
}

template<typename Stream, typename T, typename Compare>
void generic_sort(Stream & instream, Stream & outstream, Compare comp,
				  progress_indicator_base *indicator) {

	if (&instream == &outstream) {
		generic_sort<Stream, T, Compare>(instream, comp, indicator);
		return;
	}

	stream_size_type sz = instream.size();

	fractional_progress fp(indicator);
	fractional_subindicator push(fp, "sort", TPIE_FSI, sz, "Write sorted runs");
	fractional_subindicator merge(fp, "sort", TPIE_FSI, sz, "Perform merge heap");
	fractional_subindicator output(fp, "sort", TPIE_FSI, sz, "Write sorted output");
	fp.init(sz);

	instream.seek(0);

	merge_sorter<T, true, Compare> s(comp);
	s.set_available_memory(get_memory_manager().available());
	s.begin();
	push.init(sz);
	while (instream.can_read()) s.push(instream.read()), push.step();
	push.done();
	s.end();

	s.calc(merge);

	outstream.truncate(0);
	output.init(sz);
	while (s.can_pull()) outstream.write(s.pull()), output.step();
	output.done();
	fp.done();
	outstream.seek(0);
}

}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream using the given STL-style comparator
/// object.
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename Compare>
void sort(uncompressed_stream<T> &instream, uncompressed_stream<T> &outstream,
		  Compare comp, progress_indicator_base & indicator) {
	bits::generic_sort<uncompressed_stream<T>, T, Compare>(instream, outstream, &comp, &indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream using the less-than operator.
///////////////////////////////////////////////////////////////////////////////
template<typename T>
void sort(uncompressed_stream<T> &instream, uncompressed_stream<T> &outstream,
		  tpie::progress_indicator_base* indicator=NULL) {
	std::less<T> comp;
	sort(instream, outstream, comp, indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream using the less-than operator.
///////////////////////////////////////////////////////////////////////////////
template<typename T>
void sort(file_stream<T> &instream, file_stream<T> &outstream,
		  tpie::progress_indicator_base* indicator=NULL) {
	std::less<T> comp;
	bits::generic_sort<file_stream<T>, T>(instream, outstream, comp, indicator);
}


// ********************************************************************
// *                                                                  *
// * Duplicates of the above versions that only use 2x space and      *
// * overwrite the original input stream                              *
// *                                                                  *
// ********************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream in-place using the given STL-style
/// comparator object.
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename Compare>
void sort(uncompressed_stream<T> &instream, Compare comp,
		  progress_indicator_base & indicator) {
	sort(instream, instream, comp, &indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream in-place using the given STL-style
/// comparator object.
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename Compare>
void sort(file_stream<T> &instream, Compare comp,
		  progress_indicator_base & indicator) {
	bits::generic_sort<file_stream<T>, T>(instream, comp, &indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream in-place using the less-than operator.
///////////////////////////////////////////////////////////////////////////////
template<typename T>
void sort(uncompressed_stream<T> &instream, 
		  progress_indicator_base &indicator) {
	sort(instream, instream, &indicator);
}

template<typename T>
void sort(file_stream<T> &instream,
		  progress_indicator_base &indicator) {
	std::less<T> comp;
	sort(instream, comp, indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream in-place using the less-than operator and
/// no progress indicator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
void sort(uncompressed_stream<T> & instream) {
	sort(instream, instream);
}

}  //  tpie namespace

#include <tpie/sort_deprecated.h>

#endif // _AMI_SORT_H 
