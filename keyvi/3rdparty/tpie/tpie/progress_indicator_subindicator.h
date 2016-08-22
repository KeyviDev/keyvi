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

///////////////////////////////////////////////////////////////////////////////
/// \file progress_indicator_subindicator.h
/// Indicate progress of a part of a computation.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PROGRESS_INDICATOR_SUBINDICATOR_H__
#define __TPIE_PROGRESS_INDICATOR_SUBINDICATOR_H__
#include <tpie/portability.h>
#include <tpie/util.h>
#include <tpie/progress_indicator_base.h>

namespace tpie {

struct log_group_mode {
	enum type {
		enabled,
		disabled
	};
};

class progress_indicator_subindicator: public progress_indicator_base {
public:
	void refresh();
	virtual void push_breadcrumb(const char * crumb, description_importance importance);
	virtual void pop_breadcrumb();
	virtual void init(stream_size_type range);
	virtual void done();

	void set_crumb(const std::string & c) {m_crumb = c;}
	
	void setup(progress_indicator_base * parent,
			   stream_size_type outerRange,
			   const char * crumb=0,
			   description_importance importance=IMPORTANCE_MAJOR,
			   log_group_mode::type logGroupMode=log_group_mode::enabled);
	
	progress_indicator_subindicator();
	
	progress_indicator_subindicator(progress_indicator_base * parent,
									stream_size_type outerRange,
									const char * crumb=0,
									description_importance importance=IMPORTANCE_MAJOR,
									log_group_mode::type logGroupMode=log_group_mode::enabled);
#ifndef TPIE_NDEBUG
	~progress_indicator_subindicator();
#endif
protected:
	progress_indicator_base * m_parent;
	stream_size_type m_outerRange;
	stream_size_type m_oldValue;
	std::string m_crumb;
	description_importance m_importance;
	log_group_mode::type m_logGroupMode;
#ifndef TPIE_NDEBUG
	bool m_init_called;
	bool m_done_called;
#endif
};
	
} //namespace tpie
#endif //__TPIE_PROGRESS_INDICATOR_SUBINDICATOR_H__
