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
#include "fractional_progress.h"
#include <tpie/backtrace.h>
#include <tpie/prime.h>
#include <tpie/tempname.h>
#include <tpie/util.h>
#include <map>
#include <fstream>
#include <sstream>
#include <locale>
#include <boost/filesystem.hpp>
#include <regex>
#include <boost/lexical_cast.hpp>

namespace {
using namespace tpie;

class fraction_db_impl {
public:
	typedef std::map<std::string, std::pair<float, stream_size_type> > db_t;
	typedef db_t::iterator i_t;
	db_t db;
	bool dirty;
	const bool capture;

	inline fraction_db_impl(bool capture) : capture(capture) {
	}

	void update(std::string name, float frac, stream_size_type n) {
		i_t i =db.find(name);
		if (i != db.end() && i->second.second > n) return;
		db[name] = std::make_pair(frac, n);
		dirty=true;
	}

	void load(const std::string & path) {
		std::locale::global(std::locale::classic());
		dirty=false;
		std::fstream f;
		f.open(path.c_str(), std::fstream::in | std::fstream::binary);
		if (f.is_open()) {
			static const std::string head = "tpie::update_fractions(\"";
			static const std::string s1 = "\", ";
			static const std::string s2 = ", ";
			static const std::string tail = ");";
			std::string line;
			while (f) {
				std::getline(f, line);
				if (line.substr(0, head.size()) != head) continue;
				const size_t p1 = line.rfind(s1);
				const size_t p2 = p1 + s1.size();
				const size_t p3 = line.find(s2, p2);
				const size_t p4 = p3 + s2.size();
				const size_t p5 = line.find(tail, p4);
				if (p1 == std::string::npos ||
					p3 == std::string::npos ||
					p5 == std::string::npos) continue;
				
				update(line.substr(head.size(), p1 - head.size()),
					   boost::lexical_cast<float>(line.substr(p2, p3-p2)),
					   boost::lexical_cast<stream_size_type>(line.substr(p4, p5-p4)));

			}
		}
		dirty=false;
		f.close();
	}

	void save(const std::string & path, bool force = false) {
		if (!dirty && !force) return;
		std::string tmp = path+'~';
		{
			std::locale::global(std::locale::classic());
			std::fstream f;
			f.open(tmp.c_str(), std::fstream::out | std::fstream::trunc | std::fstream::binary);

			if (!f.is_open()) return;

			for (i_t i=db.begin(); i != db.end(); ++i)
				f << "tpie::update_fractions(\"" << i->first << "\", " << i->second.first << ", " << i->second.second << ");\n";
		}
		atomic_rename(tmp, path);
	}

	inline double getFraction(const std::string & name) {
		i_t i = db.find(name);
		if (i == db.end()) {
			return 1.0;
		}
		return i->second.first;
	}
};

static fraction_db_impl * fdb = 0;

} //annonymous namespace

namespace tpie {

void update_fractions(const char * name, float frac, stream_size_type n) {
	fdb->update(name, frac, n);
}
void load_fractions(const std::string & path) {
	fdb->load(path);
}
void save_fractions(const std::string & path, bool force) {
	fdb->save(path, force);
}

void init_fraction_db(bool capture_progress) {
	if (fdb) return;
	fdb = tpie_new<fraction_db_impl>(capture_progress);
}

void finish_fraction_db() {
	tpie_delete(fdb);
	fdb=NULL;
}

inline std::string fname(const char * file, const char * function, const char * name) {
	const char * y=file;
	for(const char * z=file; *z; ++z)
		if (*z=='\\' || *z == '/') y=(z+1);

	char f[256];
	{
		const char * z=function+strlen(function)-1;
		//Skip nested <> template function arguments emitted by vs
		if (*z == '>') {	
			--z;
			int c=1;
			while(c != 0) {
				if (*z == '<') --c;
				else if (*z == '>') ++c;
				--z;
			}
		}
		++z;
		//Skip anything before the last : since vs likes to include such jazz
		const char * x=z;
		while (x != function && *x != ':') --x;
		if (*x == ':') ++x;
		//Copy result to f removing any whitespace (vs likes to write "operater ()")
		char * k=f;
		for(const char * i=x; i != z; ++i) {
			if (*i == ' ') continue;
			*k = *i;
			++k;
		}
		*k = 0;
	}	
	std::string x;
	x += y;
	x += ":";
	x += f;
	x += ":";
	x += name;
	return x;
}


fractional_subindicator::fractional_subindicator(
	fractional_progress & fp): m_fp(fp) {}

///////////////////////////////////////////////////////////////////////////////
/// Construct a fractional_subindicator.
/// \param fp The owning fractional_progress
/// \param id ID of this subindicator
/// \param file The file that constructed this (see \ref TPIE_FSI)
/// \param function The function that constructed this (see \ref TPIE_FSI)
/// \param n Input size
/// \param crumb Short text describing this job
///////////////////////////////////////////////////////////////////////////////
fractional_subindicator::fractional_subindicator(
	fractional_progress & fp,
	const char * id,
	const char * file,
	const char * function,
	stream_size_type n,
	const char * crumb,
	description_importance importance,
	bool enabled): m_fp(fp) {
	setup(id, file, function, n, crumb, importance, enabled);
}

void fractional_subindicator::setup(
	const char * id,
	const char * file,
	const char * function,
	stream_size_type n,
	const char * crumb,
	description_importance importance,
	bool enabled) {
	progress_indicator_subindicator::setup(m_fp.m_pi, 42, crumb, importance);
#ifndef TPIE_NDEBUG
	m_init_called = false;
	m_done_called = false;
#endif
	m_fraction = enabled?fdb->getFraction(fname(file, function, id)):0.0;
	m_estimate = -1;
	m_n = enabled?n:0;
	m_predict = m_fp.m_id() + ";" + id;
	if (fdb->capture)
		m_stat = fname(file, function, id);

	if (enabled)
		m_estimate = m_predict.estimate_execution_time(n, m_confidence);
	else {
		m_estimate = 0;
		m_confidence = 1;
	}
	m_fp.add_sub_indicator(*this);
};

void fractional_subindicator::init(stream_size_type range) {
	softassert(m_fp.m_init_called);
	m_predict.start_execution(m_n);
	if (m_parent) {
		double f = m_fp.get_fraction(*this);
		double t = static_cast<double>(m_parent->get_range());
		m_outerRange = static_cast<stream_size_type>(t * f);
	}
#ifndef TPIE_NDEBUG
	m_init_called=true;
#endif
	progress_indicator_subindicator::init(range);	
}

void fractional_subindicator::done() {
	if (fdb->capture) {
		time_type r = m_predict.end_execution();
		if(m_n > 0) 
			m_fp.stat(m_stat, r, m_n);
	} else {
		m_predict.end_execution();
	}
	progress_indicator_subindicator::done();
}

fractional_subindicator::~fractional_subindicator() {
#ifndef TPIE_NDEBUG
	if (!m_init_called && m_fraction > 0.00001 && !std::uncaught_exception()) {
		std::stringstream s;
		if (!m_stat.empty()) {
			s << "A fractional_subindicator for ``" << m_stat << "'' was assigned a non-zero fraction but never initialized." << std::endl;
		} else {
			s << "A fractional_subindicator was assigned a non-zero fraction but never initialized." << std::endl;
		}
		TP_LOG_FATAL(s.str());
		s.str("");
		tpie::backtrace(s, 5);
		TP_LOG_DEBUG(s.str());
		TP_LOG_FLUSH_LOG;
	}
#endif
}

fractional_progress::fractional_progress(progress_indicator_base * pi):
	m_pi(pi), m_add_state(true),
#ifndef TPIE_NDEBUG
	m_init_called(false),
	m_done_called(false),
#endif
	m_confidence(1.0), m_total_sum(0), m_time_sum(0) {}
	
void fractional_progress::init(stream_size_type range) {
	unused(range);
#ifndef TPIE_NDEBUG
	if (m_init_called) {
		std::stringstream s;
		s << "init() was called on a fractional_progress for which init had already been called.  Subindicators were:" << std::endl;
		s << sub_indicators_ss();
		TP_LOG_FATAL(s.str());
		s.str("");
		tpie::backtrace(s, 5);
		TP_LOG_DEBUG(s.str());
		TP_LOG_FLUSH_LOG;
	}
	m_init_called=true;
#endif
	if (m_pi) m_pi->init(23000);
}

void fractional_progress::done() {
#ifndef TPIE_NDEBUG
	if (m_done_called || !m_init_called) {
		std::stringstream s;
		if (m_done_called) {
			s << "done() was called on fractional_progress, but done has already been called.  Subindicators were:" << std::endl;
		} else {
			s << "done() was called on fractional_progress, but init has not been called.  Subindicators were:" << std::endl;
		}
		s << sub_indicators_ss();
		TP_LOG_FATAL(s.str());
		s.str("");
		tpie::backtrace(s, 5);
		TP_LOG_DEBUG(s.str());
		TP_LOG_FLUSH_LOG;
	}
	m_done_called=true;
#endif
	if (m_pi) m_pi->done();
}

fractional_progress::~fractional_progress() {
#ifndef TPIE_NDEBUG
	if (m_init_called && !m_done_called && !std::uncaught_exception()) {
		std::stringstream s;
		s << "A fractional_progress was destructed without done being called." << std::endl;
		TP_LOG_FATAL(s.str());
		s.str("Subindicators were:\n");
		s << sub_indicators_ss();
		tpie::backtrace(s, 5);
		TP_LOG_DEBUG(s.str());
		TP_LOG_FLUSH_LOG;
	}
#endif
	if (fdb->capture) {
		time_type time_sum=0;
		for (size_t i=0; i < m_stat.size(); ++i)
			time_sum += m_stat[i].second.first;

		if (time_sum > 0) {
			for (size_t i=0; i < m_stat.size(); ++i) {
				std::pair< std::string, std::pair<time_type, stream_size_type> > & x = m_stat[i];
				float f= (float)x.second.first / (float)time_sum;
				fdb->update(x.first.c_str(), f, x.second.second);
			}
		}
	}
}

unique_id_type & fractional_progress::id() {return m_id;}

void fractional_progress::add_sub_indicator(fractional_subindicator & sub) {
	softassert(m_add_state==true);
	if (sub.m_fraction < 0.000000001 && sub.m_confidence > 0.5) return;
	m_total_sum += sub.m_fraction;
	m_confidence = std::min(sub.m_confidence, m_confidence);
	m_time_sum += sub.m_estimate;
}

double fractional_progress::get_fraction(fractional_subindicator & sub) {
	m_add_state=false;

	if (sub.m_fraction < 0.000000001 && sub.m_confidence > 0.5) return 0.0;
	
	double f1 = (m_total_sum > 0.00001)?sub.m_fraction / m_total_sum: 0.0;
	double f2 = (m_time_sum >= 1)?((double)sub.m_estimate / (double)m_time_sum):0.0;
	
	double f = f1 * (1.0 - m_confidence) + f2*m_confidence;
	return f;
}

void fractional_progress::stat(std::string name, time_type time, stream_size_type n) {
	m_stat.push_back(std::make_pair(name , std::make_pair(time, n)));
}

std::string fractional_progress::sub_indicators_ss() {
	std::stringstream s;
	if (m_stat.size() > 0) {
		for (size_t i = 0; i < m_stat.size(); ++i) {
			s << "- " << m_stat[i].first << std::endl;
		}
	} else {
		s << "(None.)" << std::endl;
	}
	return s.str();
}
	
} //namespace tpie
