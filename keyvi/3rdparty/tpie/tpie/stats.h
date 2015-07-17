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

///////////////////////////////////////////////////////////////////////////////
/// \file stats.h  I/O statistics
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_STATS_H
#define _TPIE_STATS_H
#include <tpie/types.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace tpie {

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the number of bytes currently being used by temporary files.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type get_temp_file_usage();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Increment (possibly by a negative amount) the number of bytes being
    /// used by temporary files
	///////////////////////////////////////////////////////////////////////////
	void increment_temp_file_usage(stream_offset_type delta);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the number of bytes read from disk since program start.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type get_bytes_read();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the number of bytes written to disk since program start.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type get_bytes_written();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Inform that stats module that an additional delta bytes have
	/// been read from disk.
	///////////////////////////////////////////////////////////////////////////
	void increment_bytes_read(stream_size_type delta);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Inform the stats module that an additional delta bytes have
	/// been written to disk.
	///////////////////////////////////////////////////////////////////////////
	void increment_bytes_written(stream_size_type delta);

	stream_size_type get_user(size_t i);
	void increment_user(size_t i, stream_size_type delta);

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

class stat_timer {
public:
	stat_timer(size_t i)
		: i(i)
		, t1(ptime::now())
	{
	}

	~stat_timer() {
		ptime t2 = ptime::now();
		increment_user(i, ptime::seconds(t1, t2)*1000000);
	}

private:
	size_t i;
	ptime t1;
};

}  //  tpie namespace
#endif //_TPIE_STATS_H
