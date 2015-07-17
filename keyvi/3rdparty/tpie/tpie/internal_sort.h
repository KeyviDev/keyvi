// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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

#ifndef _INTERNAL_SORT_H
#define _INTERNAL_SORT_H

///////////////////////////////////////////////////////////////////////////////
/// \file internal_sort.h  Internal sorter objects.
/// Provides base class Internal_Sorter_Base for internal sorter objects and
/// subclass implementation Internal_Sorter_Obj.
/// Relies on quicksort variant quick_sort_obj().
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/parallel_sort.h>
#include <tpie/fractional_progress.h>
#include <tpie/memory.h>
#include <tpie/tpie_assert.h>
#include <tpie/file_stream.h>

namespace tpie {
namespace ami {

///////////////////////////////////////////////////////////////////////////////
/// \brief A simple class that facilitates doing key sorting followed by
/// in-memory permuting to sort items in-memory. This is particularly useful
/// when key size is much smaller than item size. Note that using this requires
/// that the class Key have the comparison operators defined appropriately.
///////////////////////////////////////////////////////////////////////////////
template<class Key>
class qsort_item {
public:
	Key keyval;
	TPIE_OS_SIZE_T source;
	
	friend int operator==(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval ==  y.keyval);}
	
	friend int operator!=(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval !=  y.keyval);}    
	
	friend int operator<=(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval <=  y.keyval);}
	
	friend int operator>=(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval >=  y.keyval);}
	
	friend int operator<(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval <  y.keyval);}
	
	friend int operator>(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval >  y.keyval);}
};


///////////////////////////////////////////////////////////////////////////////
/// \brief The base class for internal sorters. 
/// This class does not have a sort() function, so it cannot be used directly.
/// \tparam T The type of elements to sort.
///////////////////////////////////////////////////////////////////////////////
template<class T>
class Internal_Sorter_Base {
protected:
	/** Array that holds items to be sorted. */
	array<T> ItemArray;        
	/** length of ItemArray */
	TPIE_OS_SIZE_T len;  

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	Internal_Sorter_Base(void): len(0) {
		//  No code in this constructor.
	};
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Allocate ItemArray as array that can hold \p nItems.
	///////////////////////////////////////////////////////////////////////////
	void allocate(TPIE_OS_SIZE_T nItems);
	    
	///////////////////////////////////////////////////////////////////////////
	/// \brief Clean up internal array ItemArray.
	///////////////////////////////////////////////////////////////////////////
	void deallocate(void); 
	    
	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns maximum number of items that can be sorted using
	/// \p memSize bytes.
	///////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T MaxItemCount(TPIE_OS_SIZE_T memSize);
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns memory usage in bytes per sort item.
	///////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T space_per_item();
	    
	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns fixed memory usage overhead in bytes per class
	/// instantiation.
	///////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T space_overhead();
	
private:
	// Prohibit these
	Internal_Sorter_Base(const Internal_Sorter_Base<T>& other);
	Internal_Sorter_Base<T> operator=(const Internal_Sorter_Base<T>& other);
};
	
template<class T>
inline void Internal_Sorter_Base<T>::allocate(TPIE_OS_SIZE_T nitems) {
	len=nitems;
	ItemArray.resize(len);
}

template<class T>
inline void Internal_Sorter_Base<T>::deallocate(void) {
	ItemArray.resize(0);
	len=0;
}

template<class T>
inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::MaxItemCount(TPIE_OS_SIZE_T memSize) {
	//Space available for items
	TPIE_OS_SIZE_T memAvail=memSize-space_overhead();
	
	if (memAvail < space_per_item()) return 0; 
	return memAvail/space_per_item(); 
}

	
template<class T>
inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::space_overhead(void) {
	// Space usage independent of space_per_item
	// accounts MM_manager space overhead on "new" call
	return 0;
}

template<class T>
inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::space_per_item(void) {
	return sizeof(T);
}

///////////////////////////////////////////////////////////////////////////
/// Comparision object based Internal_Sorter_base subclass implementation; uses 
/// quick_sort_obj().
///////////////////////////////////////////////////////////////////////////
template<class T, class Compare>
class Internal_Sorter_Obj: public Internal_Sorter_Base<T>{
protected:
	using Internal_Sorter_Base<T>::ItemArray;
	using Internal_Sorter_Base<T>::len;
	/** Comparison object used for sorting */
	Compare cmp_o;
	
public:
	///////////////////////////////////////////////////////////////////////////
	/// Empty constructor.
	///////////////////////////////////////////////////////////////////////////
	Internal_Sorter_Obj(Compare cmp) :cmp_o(cmp) {};
	
	///////////////////////////////////////////////////////////////////////////
	/// Empty destructor.
	///////////////////////////////////////////////////////////////////////////
	~Internal_Sorter_Obj(){};
	
	using Internal_Sorter_Base<T>::space_overhead;
	
	//Sort nItems from input stream and write to output stream
	void sort(file_stream<T>* InStr, file_stream<T>* OutStr, TPIE_OS_SIZE_T nItems, 
			  progress_indicator_base * pi=0);
	
private:
	// Prohibit these
	Internal_Sorter_Obj(const Internal_Sorter_Obj<T,Compare>& other);
	Internal_Sorter_Obj<T,Compare> operator=(const Internal_Sorter_Obj<T,Compare>& other);
};

///////////////////////////////////////////////////////////////////////////////
/// Reads nItems sequentially from InStr, starting at the current file
/// position; writes the sorted output to OutStr, starting from the current
/// file position.
///////////////////////////////////////////////////////////////////////////
template<class T, class Compare>
void Internal_Sorter_Obj<T, Compare>::sort(file_stream<T>* InStr, 
										   file_stream<T>* OutStr, 
										   TPIE_OS_SIZE_T nItems,
										   progress_indicator_base * pi) {
	tp_assert ( nItems <= len, "Internal buffer overfull (nItems > len)");

	
	TPIE_OS_SIZE_T i = 0;
	
	//make sure we called allocate earlier
	if (ItemArray.size() == 0) throw stream_exception("NULL_POINTER");
	    
	tp_assert ( nItems <= len, "Internal buffer overfull (nItems > len)");

	fractional_progress fp(pi);
	fp.id() << __FILE__ << __FUNCTION__ << typeid(T) << typeid(Compare);
	fractional_subindicator read_progress(fp, "read", TPIE_FSI, nItems, "Reading");
	fractional_subindicator sort_progress(fp, "sort", TPIE_FSI, nItems, "Sorting");
	fractional_subindicator write_progress(fp, "write", TPIE_FSI, nItems, "Writing");
	fp.init();

	read_progress.init(nItems);
	// Read a memory load out of the input stream one item at a time,
	for (i = 0; i < nItems; i++) {
		ItemArray[i] = InStr->read();
		read_progress.step();
	}
	read_progress.done();

	//Sort the array.
	tpie::parallel_sort<true>(ItemArray.begin(), ItemArray.begin()+nItems, sort_progress, cmp_o);
	if (InStr==OutStr) { //Do the right thing if we are doing 2x sort
		//Internal sort objects should probably be re-written so that
		//the interface is cleaner and they don't have to worry about I/O
		InStr->truncate(0); //delete original items
		InStr->seek(0); //rewind
	}

	write_progress.init(nItems);
	//Write sorted array to OutStr
	for (i = 0; i < nItems; i++) {
		OutStr->write(ItemArray[i]);
		write_progress.step();
	}
	write_progress.done();
	fp.done();
}

}  //  ami namespace
}  //  tpie namespace
#endif // _INTERNAL_SORT_H 














































