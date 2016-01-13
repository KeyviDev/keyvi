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
#include <tpie/portability.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <tpie/util.h>
#include <tpie/prime.h>
#ifndef __TPIE_EXECUTION_TIME_PREDICTOR_H__
#define __TPIE_EXECUTION_TIME_PREDICTOR_H__

///////////////////////////////////////////////////////////////////////////////
/// \file execution_time_predictor.h Execution time predictor used by
/// fractional progress.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

typedef uint64_t time_type;

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the execution time
/// database.
///////////////////////////////////////////////////////////////////////////////
void init_execution_time_db();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the execution time
/// database.
///////////////////////////////////////////////////////////////////////////////
void finish_execution_time_db();

class unique_id_type {
public:
    inline unique_id_type & operator << (const std::type_info & type) {
		ss << type.name() << ";"; return *this;
    }
    
    template <typename T>
    inline unique_id_type & operator<<(const T & x) {
		ss << typeid(T).name() << ":" << x << ";"; return *this;
    }
	
	inline std::string operator()() {return ss.str();}
private:
	std::stringstream ss;
};


class execution_time_predictor {
public:
	execution_time_predictor(const std::string & id=std::string());
	~execution_time_predictor();
	///////////////////////////////////////////////////////////////////////////
	/// Estimate execution time.
	/// \param n Input size
	/// \param confidence (output) Confidence (between 0.0 and 1.0)
	///////////////////////////////////////////////////////////////////////////
	time_type estimate_execution_time(stream_size_type n, double & confidence);
	void start_execution(stream_size_type n);
	time_type end_execution();
	std::string estimate_remaining_time(double progress);

	static void start_pause();
	static void end_pause();
	static void disable_time_storing();

	//Used by fractional_time_perdictor
	//TPIE_OS_OFFSET m_aux1;
	//double m_aux2;
private:
	hash_type m_id;
	boost::posix_time::ptime m_start_time;
	time_type m_estimate;
	double m_confidence;

	/** Input size */
	stream_size_type m_n;

	time_type m_pause_time_at_start;

#ifndef TPIE_NDEBUG
	std::string m_name;
#endif

	static time_type s_pause_time;
	static boost::posix_time::ptime s_start_pause_time;
	static bool s_store_times;
};

} //namespace tpie

#endif //__TPIE_EXECUTION_TIME_PREDICTOR_H__

