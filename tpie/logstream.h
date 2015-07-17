// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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

#ifndef _TPIE_LOGSTREAM_H
#define _TPIE_LOGSTREAM_H
///////////////////////////////////////////////////////////////////////////
/// \file logstream.h
/// logstream class used by definitions in \ref tpie_log.h.
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_LOGSTREAM_H__
#define __TPIE_LOGSTREAM_H__

#include <tpie/config.h>
#include <tpie/loglevel.h>
#include <streambuf>
#include <ostream>

namespace tpie {

namespace log_bits {

extern bool logging_disabled;

}

struct log_target {
	virtual void log(log_level level, const char * message, size_t message_size) = 0;
	virtual ~log_target() { }
	virtual void begin_group(const std::string &) {};
	virtual void end_group() {};
};

void add_log_target(log_target * t);
void remove_log_target(log_target * t);

void begin_log_group(const std::string & name);
void end_log_group();

class log_stream_buf: public std::basic_streambuf<char, std::char_traits<char> >  {
private:
	const static size_t buff_size = 2048;
	const static size_t max_targets = 8;

	char m_buff[buff_size];
	log_level m_level;

public:
	log_stream_buf(log_level level);
	virtual ~log_stream_buf();
	void flush();
	virtual int overflow(int c = traits_type::eof()) override;
	virtual int sync() override;

	// Deprecated:
	void add_target(log_target * t) {add_log_target(t);}
	void remove_target(log_target * t) {remove_log_target(t);}
};

///////////////////////////////////////////////////////////////////////////////
/// A log is like a regular output stream, but it also supports messages
/// at different priorities, see \ref log_level.
///
/// Do not instantiate this class directly. Instead, use \ref get_log() as well
/// as helper methods \ref log_fatal(), \ref log_error(), \ref log_info(),
/// \ref log_warning(), \ref log_app_debug(), \ref log_debug() and
/// \ref log_mem_debug().
///////////////////////////////////////////////////////////////////////////////
class logstream: public std::ostream {
private:
	log_stream_buf m_buff;
public:
	///////////////////////////////////////////////////////////////////////////
	/// Constructor.
	///////////////////////////////////////////////////////////////////////////
	inline logstream(log_level level=LOG_INFORMATIONAL): std::ostream(&m_buff), m_buff(level) {}

	///////////////////////////////////////////////////////////////////////////
	/// \deprecated
	/// Add a target for the log messages
	///////////////////////////////////////////////////////////////////////////
	void add_target(log_target * t) {add_log_target(t);}

	///////////////////////////////////////////////////////////////////////////
	/// \deprecated
	/// Remove a target for the log messages
	///////////////////////////////////////////////////////////////////////////
	void remove_target(log_target * t) {remove_log_target(t);}
};

class log_level_manip {
private:
	log_level level;

public:
	log_level_manip(log_level p) : level(p) {}

	log_level get_level() const { return level; }
};

inline log_level_manip setlevel(log_level p) { return log_level_manip(p); }

///////////////////////////////////////////////////////////////////////////////
/// \brief RAII-style management for log groups
///////////////////////////////////////////////////////////////////////////////
class log_group {
public:
	log_group(const std::string & name);
	~log_group();
};

}  //  tpie namespace


#endif //__TPIE_LOGSTREAM_H__

#endif // _LOGSTREAM_H
