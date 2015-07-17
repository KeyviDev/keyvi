// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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
/// \file fractional_progress.h
/// \brief Fractional progress reporting.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_FRACTIONAL_PROGRESS__
#define __TPIE_FRACTIONAL_PROGRESS__

#include <tpie/portability.h>
#include <tpie/util.h>
#include <tpie/progress_indicator_subindicator.h>

///////////////////////////////////////////////////////////////////////////////
/// \def TPIE_FSI
/// For use when constructing a fractional subindicator. Returns the caller's
/// file and function name.
///////////////////////////////////////////////////////////////////////////////
#define TPIE_FSI __FILE__,__FUNCTION__

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the fraction database.
///////////////////////////////////////////////////////////////////////////////
void init_fraction_db(bool capture_progress = false);

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the fraction database.
///////////////////////////////////////////////////////////////////////////////
void finish_fraction_db();

class fractional_progress;

///////////////////////////////////////////////////////////////////////////////
/// \brief Subindicator for fractional progress reporting.
///////////////////////////////////////////////////////////////////////////////
class fractional_subindicator: public progress_indicator_subindicator {
public:
	fractional_subindicator(fractional_progress & fp);
	
	fractional_subindicator(fractional_progress & fp,
							const char * id,
							const char * file,
							const char * function,
							stream_size_type n,
							const char * crumb=0,
							description_importance importance=IMPORTANCE_MAJOR,
							bool enabled=true);

	void setup(const char * id,
			   const char * file,
			   const char * function,
			   stream_size_type n,
			   const char * crumb=0,
			   description_importance importance=IMPORTANCE_MAJOR,
			   bool enabled=true);

	~fractional_subindicator();

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc progress_indicator_subindicator::init
	///////////////////////////////////////////////////////////////////////////
	virtual void init(stream_size_type range);

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc progress_indicator_subindicator::done
	///////////////////////////////////////////////////////////////////////////
	virtual void done();
private:
#ifndef TPIE_NDEBUG
	bool m_init_called;
	bool m_done_called;
#endif
	double m_fraction;
	stream_size_type m_estimate;
	double m_confidence;
	stream_size_type m_n;
	fractional_progress & m_fp;
	execution_time_predictor m_predict;

	std::string m_stat;
	friend class fractional_progress;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Fractional progress reporter.
///////////////////////////////////////////////////////////////////////////////
class fractional_progress {
public:
	fractional_progress(progress_indicator_base * pi);
	~fractional_progress();

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc progress_indicator_base::done
	///////////////////////////////////////////////////////////////////////////
	void done();

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc progress_indicator_base::init
	///////////////////////////////////////////////////////////////////////////
	void init(stream_size_type range=0);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return this progress indicator's unique id.
	///////////////////////////////////////////////////////////////////////////
	unique_id_type & id();

	void push_breadcrumb(const char * crumb, description_importance importance) {
		if (m_pi) m_pi->push_breadcrumb(crumb, importance);
	}

	void pop_breadcrumb() {
		if (m_pi) m_pi->pop_breadcrumb();
	}

private:
	double get_fraction(fractional_subindicator & sub);

	void add_sub_indicator(fractional_subindicator & sub);
	progress_indicator_base * m_pi;
	bool m_add_state;
#ifndef TPIE_NDEBUG
	bool m_init_called;
	bool m_done_called;
#endif
	double m_confidence;
	
	unique_id_type m_id;
	double m_total_sum;
	stream_size_type m_time_sum;

	void stat(std::string, time_type, stream_size_type);
	std::vector< std::pair<std::string, std::pair<time_type, stream_size_type> > > m_stat;

	std::string sub_indicators_ss();
	
	friend class fractional_subindicator;
};

void update_fractions(const char * name, float frac, stream_size_type n);
void load_fractions(const std::string & path);
void save_fractions(const std::string & path, bool force = false);

}
#endif //__TPIE_FRACTIONAL_PROGRESS__

