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

#include "progress_indicator_subindicator.h"
#include <tpie/backtrace.h>
#include <sstream>

namespace tpie {

void progress_indicator_subindicator::push_breadcrumb(const char * crumb, description_importance importance) {
	if (m_parent) m_parent->push_breadcrumb(crumb, std::min(importance, m_importance));
}


void progress_indicator_subindicator::pop_breadcrumb() {
	if (m_parent) m_parent->pop_breadcrumb();
}

/**
 * \param parent The parent indecator of this indicator
 * \param outerRange The range this indicator ocupices of its outer indicator
 * \param crumb The bread crumb of this indicator
 * \param importance The maximal importance to assign to the crumbs of child indicators
 */
progress_indicator_subindicator::progress_indicator_subindicator(progress_indicator_base * parent,
																 stream_size_type outerRange,
																 const char * crumb,
																 description_importance importance,
																 log_group_mode::type logGroupMode):
	progress_indicator_base(0) {
	setup(parent, outerRange, crumb, importance, logGroupMode);
}


progress_indicator_subindicator::progress_indicator_subindicator():
	progress_indicator_base(0) {}

void progress_indicator_subindicator::setup(progress_indicator_base * parent,
											stream_size_type outerRange,
											const char * crumb,
											description_importance importance,
											log_group_mode::type logGroupMode) {
	m_parent = parent;
	m_outerRange = outerRange;
	m_importance = importance;
	m_oldValue = 0;
	m_logGroupMode = logGroupMode;
#ifndef TPIE_NDEBUG
	m_init_called = false;
	m_done_called = false;
#endif
	if (crumb == 0)
		m_crumb = "";
	else
		m_crumb = crumb;
}


#ifndef TPIE_NDEBUG
progress_indicator_subindicator::~progress_indicator_subindicator() {
	if (m_init_called && !m_done_called && !std::uncaught_exception()) {
		std::stringstream s;
		s << "A progress_indicator_subindicator was destructed without done being called." << std::endl;
		TP_LOG_FATAL(s.str());
		s.str("");
		tpie::backtrace(s, 5);
		TP_LOG_DEBUG(s.str());
		TP_LOG_FLUSH_LOG;
	}
}
#endif

void progress_indicator_subindicator::refresh() {
	stream_size_type val = get_current();
	if (val > get_range()) val = get_range();
	if (get_range() == 0) return;
	stream_size_type value= val* m_outerRange / get_range();
	if (m_parent) {
		m_parent->raw_step(value - m_oldValue);
		m_oldValue = value;
	}
}

void progress_indicator_subindicator::init(stream_size_type range) {
#ifndef TPIE_NDEBUG
	softassert(!m_init_called && "Init called twice");
	m_init_called=true;
#endif
	if (!m_crumb.empty() && m_parent) m_parent->push_breadcrumb(m_crumb.c_str(), IMPORTANCE_MAJOR);
	progress_indicator_base::init(range);

	if(m_logGroupMode == log_group_mode::enabled)
		begin_log_group(m_crumb);
}

void progress_indicator_subindicator::done() {
#ifndef TPIE_NDEBUG
	softassert(m_init_called);
	softassert(!m_done_called);
	m_done_called=true;

	if(m_logGroupMode == log_group_mode::enabled)
		end_log_group();
#endif
	if (!m_crumb.empty() && m_parent) m_parent->pop_breadcrumb();
	progress_indicator_base::done();
	m_current = m_range; 
	refresh();
}

} //namespace tpie
