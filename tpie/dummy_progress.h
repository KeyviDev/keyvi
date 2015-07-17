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
#ifndef __TPIE_DUMMY_PROGRESS__
#define __TPIE_DUMMY_PROGRESS__

#include <tpie/fractional_progress.h>

///////////////////////////////////////////////////////////////////////////////
/// \file dummy_progress.h
///
/// Progress indicator concept in an efficient
/// non-inheritance way.  For a null object child class of
/// \ref tpie::progress_indicator_base, see \ref tpie::progress_indicator_null.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

struct dummy_progress_indicator;

///////////////////////////////////////////////////////////////////////////////
/// \brief  A fractional progress indicator that is conceptually compatible
/// with tpie::fractional_progress.
///////////////////////////////////////////////////////////////////////////////

struct dummy_fraction_progress {
	///////////////////////////////////////////////////////////////////////////
	/// Dummy constructor that does nothing.
	///////////////////////////////////////////////////////////////////////////
	inline dummy_fraction_progress(tpie::progress_indicator_base *) {
		// Do nothing.
	}

	///////////////////////////////////////////////////////////////////////////
	/// Dummy constructor that does nothing.
	///////////////////////////////////////////////////////////////////////////
	inline dummy_fraction_progress(dummy_progress_indicator *) {
		// Do nothing.
	}

	/// \copydoc tpie::fractional_progress::id()
	inline dummy_fraction_progress & id() {
		return *this;
	}

	template <typename T>
	inline dummy_fraction_progress & operator << (const T &) {
		return *this;
	}

	/// \copybrief tpie::fractional_progress::init()
	/// \copydetails tpie::fractional_progress::init()
	inline void init(stream_size_type range = 0) {
		unused(range);
	}

	/// \copybrief tpie::fractional_progress::done()
	/// \copydetails tpie::fractional_progress::done()
	inline void done() {
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  A progress indicator that is conceptually compatible with
/// tpie::progress_indicator_base and tpie::fractional_subindicator.
///////////////////////////////////////////////////////////////////////////////

struct dummy_progress_indicator {
	inline dummy_progress_indicator() {
		// Do nothing.
	}

	// fractional_subindicator style constructor
	inline dummy_progress_indicator(dummy_fraction_progress & /*fp*/,
									const char * /*id*/, const char * /*file*/, const char * /*function*/,
									stream_size_type /*n*/, const char *crumb=0,
									bool importance=true, bool enabled=true) {
		unused(crumb);
		unused(importance);
		unused(enabled);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc progress_indicator_base::init(stream_size_type)
	///////////////////////////////////////////////////////////////////////////
	inline void init(stream_size_type range = 0) {
		unused(range);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc progress_indicator_base::step(stream_size_type)
	///////////////////////////////////////////////////////////////////////////
	inline void step(stream_size_type) { }

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc progress_indicator_base::step(stream_size_type)
	///////////////////////////////////////////////////////////////////////////
	inline void step() { }

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc progress_indicator_base::done()
	///////////////////////////////////////////////////////////////////////////
	inline void done() { }
};

///////////////////////////////////////////////////////////////////////////////
/// For applications where you wish to disable progress indicators via a
/// template parameter, refer to progress_types<use_progress> members names
/// sub, fp and base.
///////////////////////////////////////////////////////////////////////////////
template <bool use_progress>
struct progress_types {
	typedef fractional_subindicator sub;
	typedef fractional_progress fp;
	typedef progress_indicator_base base;
};

template <>
struct progress_types<false> {
	typedef dummy_progress_indicator sub;
	typedef dummy_fraction_progress fp;
	typedef dummy_progress_indicator base;
};

}
#endif //__TPIE_DUMMY_PROGRESS__
