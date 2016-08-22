// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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

#include "progress_indicator_base.h"
#include <tpie/tpie_assert.h>

namespace {

class ptime {
public:
	ptime()
		: m_ptime(boost::posix_time::not_a_date_time)
	{
	}

	static ptime now() {
		return ptime(boost::posix_time::microsec_clock::universal_time());
	}

	static double seconds(const ptime & t1, const ptime & t2) {
		if (t1.m_ptime.is_special() || t2.m_ptime.is_special()) {
			return 0.0;
		}
		return (t2.m_ptime - t1.m_ptime).total_microseconds() / 1000000.0;
	}

private:
	boost::posix_time::ptime m_ptime;

	ptime(boost::posix_time::ptime ptime)
		: m_ptime(ptime)
	{
	}
};

} // unnamed namespace

namespace tpie {

struct progress_indicator_base::refresh_impl {
	/**  The approximate frequency of calls to refresh in hz */
	static const unsigned int FREQUENCY = 5;
	ptime m_firstSample;
};

progress_indicator_base::progress_indicator_base(stream_size_type range)
	: m_range(range)
	, m_current(0)
	, m_predictor(0)
{
	m_refreshImpl = new progress_indicator_base::refresh_impl;
}

/*virtual*/ progress_indicator_base::~progress_indicator_base() {
	delete m_refreshImpl;
	m_refreshImpl = NULL;
};

void progress_indicator_base::call_refresh() {
	refresh_impl * const impl = this->m_refreshImpl;
	if (m_current == 0) {
		impl->m_firstSample = ptime::now();
		this->m_remainingSteps = 1;
		this->refresh();
		return;
	}

	// Time since beginning
	const double t = std::max(0.000001, ptime::seconds(impl->m_firstSample, ptime::now()));

	// From t0 (target time between calls to refresh)
	// and f (measured step frequency),
	// compute k' (estimated steps until next refresh) as such:
	//     k := m_current (steps since beginning)
	//    t0 := 1 / FREQUENCY  (seconds),
	//     f := k / t  (steps per second),
	//    k' := t0 * f
	//        = k / (t * m_frequency).
	// However, we limit k' to be at most 2*k.
	const stream_size_type k_new = static_cast<stream_size_type>(
		m_current / (t * refresh_impl::FREQUENCY));
	const stream_size_type a = 1;
	const stream_size_type b = 2 * m_current;
	this->m_remainingSteps = std::max(a, std::min(k_new, b));

	this->refresh();
}

} // namespace tpie
