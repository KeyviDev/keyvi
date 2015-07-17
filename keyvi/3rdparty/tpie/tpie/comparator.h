// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
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

#ifndef _COMPARATOR_H
#define _COMPARATOR_H

///////////////////////////////////////////////////////////////////////////
/// \file comparator.h
/// \internal
/// \brief Conversion between STL and TPIE comparators.
///
/// Used in \ref internal_sort.h.
///
/// We do not implement conversion of less-than-operator to STL-style
/// comparator, as this is already implemented as std::less in &lt;functional&gt;.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/config.h>
#include <tpie/portability.h>

namespace tpie {

// In is unlikely that users will need to directly need these classes
// except to maybe upgrade old code with minimal changes. In the future it
// may be better to make TPIE's compare() method match the syntax of the
// STL operator ()

///////////////////////////////////////////////////////////////////////////////
/// Convert STL comparison object with operator() to a TPIE comparison
/// object with a compare() function.
///////////////////////////////////////////////////////////////////////////////
template<class T, class STLCMP>
class STL2TPIE_cmp{
public:
	STL2TPIE_cmp(STLCMP* cmp) : m_isLess(cmp) {
		// Does nothing.
	}

	///////////////////////////////////////////////////////////////////////////
	/// Do not use with applications that test if compare returns +1
	/// Because it never does so.
	///////////////////////////////////////////////////////////////////////////
	inline int compare(const T& left, const T& right){
		if( (*m_isLess)(left, right) ){ return -1; }
		else { return 0; }
	}

private:
	/** Class with STL comparison operator(). */
	STLCMP *m_isLess;
};

///////////////////////////////////////////////////////////////////////////////
/// Convert a TPIE comparison object with a compare() function to STL
/// comparison object with operator().
///////////////////////////////////////////////////////////////////////////////
template<class T, class TPCMP>
class TPIE2STL_cmp{
public:
	TPIE2STL_cmp(TPCMP* cmp) : m_cmpobj(cmp) {
		// Does nothing.
	}

	inline bool operator()(const T& left, const T& right) const{
		return (m_cmpobj->compare(left, right) < 0);
	}

private:
	/** Class with TPIE comparison compare(). */
	TPCMP* m_cmpobj;
};

///////////////////////////////////////////////////////////////////////////////
/// Convert a class with a comparison operator &lt; to a TPIE comparison object
/// with a compare() function.
///////////////////////////////////////////////////////////////////////////////
template<class T>
class op2TPIE_cmp{
public:
	op2TPIE_cmp() {
		// Does nothing.
	}

	///////////////////////////////////////////////////////////////////////////
	/// Do not use with applications that test if compare returns +1
	/// Because it never does so.
	///////////////////////////////////////////////////////////////////////////
	inline int compare(const T& left, const T& right){
		if( left < right ){ return -1; }
		else { return 0; }
	}
};

}  //  tpie namespace

#endif // _COMPARATOR_H 
