// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2011, The TPIE development team
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

#ifndef _TPIE_LOG_H
#define _TPIE_LOG_H
///////////////////////////////////////////////////////////////////////////
/// \file tpie_log.h
/// Logging functionality and log_level codes for different priorities of log messages.
///////////////////////////////////////////////////////////////////////////

#include <vector>
#include <stack>
#include <memory>
#include <tpie/config.h>
#include <tpie/logstream.h>
#include <fstream>

namespace tpie {

/** A simple logger that writes messages to a tpie temporary file */
class file_log_target: public log_target {
private:
	std::stack<std::string> groups;
public:
	std::ofstream m_out;
	std::string m_path;
	log_level m_threshold;
	
    /** Construct a new file logger
	 * \param threshold record messages at or above this severity threshold
	 * */
	file_log_target(log_level threshold);

	/** Implement \ref log_target virtual method to record message
	 * \param level severity of message
	 * \param message content of message
	 * */
	void log(log_level level, const char * message, size_t);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Creates a new logging group. All console output that occurs after 
	/// this will appear in the same visual group.
	///////////////////////////////////////////////////////////////////////////////
	void begin_group(const std::string & name);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Closes the most recently created logging group.
	///////////////////////////////////////////////////////////////////////////////
	void end_group();
private:
	std::string build_prefix(size_t length);
};

/** A simple logger that writes messages to stderr */
class stderr_log_target: public log_target {
private:
	std::stack<std::string> groups;
public:
	log_level m_threshold;

    /** Construct a new stderr logger
	 * \param threshold record messages at or above this severity threshold
	 * */
	stderr_log_target(log_level threshold);
	
	/** Implement \ref log_target virtual method to record message
	 * \param level severity of message
	 * \param message content of message
	 * \param size lenght of message array
	 * */
	void log(log_level level, const char * message, size_t size);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Creates a new logging group. All console output that occurs after 
	/// this will appear in the same visual group.
	///////////////////////////////////////////////////////////////////////////////
	void begin_group(const std::string & name);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Closes the most recently created logging group.
	///////////////////////////////////////////////////////////////////////////////
	void end_group();
private:
	std::string build_prefix(size_t length);	
};




///////////////////////////////////////////////////////////////////////////
/// \brief Returns the file name of the log stream.
/// This assumes that init_default_log has been called.
///////////////////////////////////////////////////////////////////////////
const std::string& log_name();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the log subsystem.
///////////////////////////////////////////////////////////////////////////////
void init_default_log();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the log subsystem.
///////////////////////////////////////////////////////////////////////////////
void finish_default_log();

namespace log_bits {

extern std::vector<std::shared_ptr<logstream> > log_instances;

void initiate_log_level(log_level level);

void flush_logs();

}

inline logstream & get_log_by_level(log_level level) {
	using namespace log_bits;
	if (log_instances.size() <= level || log_instances[level].get() == 0)
		initiate_log_level(level);
	return *log_instances[level];
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing fatal log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_fatal() {return get_log_by_level(LOG_FATAL);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing error log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_error() {return get_log_by_level(LOG_ERROR);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing info log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_info() {return get_log_by_level(LOG_INFORMATIONAL);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing warning log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_warning() {return get_log_by_level(LOG_WARNING);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing app_debug log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_app_debug() {return get_log_by_level(LOG_APP_DEBUG);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing debug log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_debug() {return get_log_by_level(LOG_DEBUG);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing mem_debug log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_mem_debug() {return get_log_by_level(LOG_MEM_DEBUG);}

class scoped_log_enabler {
private:
	bool m_orig;
public:
	inline bool get_orig() {return m_orig;}
	inline scoped_log_enabler(bool e) {
		m_orig = log_bits::logging_disabled;
		log_bits::logging_disabled = !e;
	}
	inline ~scoped_log_enabler() {
		log_bits::logging_disabled = m_orig;
	}
};

namespace log_bits {

class log_selector {
private:
	static bool s_init;
	static log_level s_level;

	logstream & get_log() {
		if (!s_init) {
			s_init = true;
			s_level = LOG_INFORMATIONAL;
		}
		switch (s_level) {
		case LOG_FATAL:
			return log_fatal();
		case LOG_ERROR:
			return log_error();
		case LOG_INFORMATIONAL:
			return log_info();
		case LOG_WARNING:
			return log_warning();
		case LOG_APP_DEBUG:
			return log_app_debug();
		case LOG_DEBUG:
			return log_debug();
		case LOG_MEM_DEBUG:
			return log_mem_debug();
		case LOG_USER1:
		case LOG_USER2:
		case LOG_USER3:
			break;
		}
		return log_info();
	}

public:
	log_selector & operator<<(log_level_manip mi) {
		set_level(mi.get_level());
		return *this;
	}

	template <typename T>
	logstream & operator<<(const T & x) {
		logstream & res = get_log();
		res << x;
		return res;
	}

	void flush() {
		get_log().flush();
	}

	void set_level(log_level level) {
		s_init = true;
		s_level = level;
	}

	void add_target(log_target * t) { add_log_target(t); }

	void remove_target(log_target * t) { remove_log_target(t); }
};

} // namespace log_bits

///////////////////////////////////////////////////////////////////////////
/// \brief Returns the only logstream object. 
///////////////////////////////////////////////////////////////////////////
inline log_bits::log_selector get_log() {return log_bits::log_selector();}

#if TPL_LOGGING		
/// \def TP_LOG_FLUSH_LOG  \deprecated Use \ref get_log().flush() instead.
#define TP_LOG_FLUSH_LOG tpie::get_log().flush()
    
/// \def TP_LOG_FATAL \deprecated Use \ref log_fatal() instead.
#define TP_LOG_FATAL(msg) tpie::log_fatal() << msg
/// \def TP_LOG_WARNING \deprecated Use \ref log_warning() instead.
#define TP_LOG_WARNING(msg)	tpie::log_warning() << msg
/// \def TP_LOG_APP_DEBUG \deprecated Use \ref log_app_debug() instead.
#define TP_LOG_APP_DEBUG(msg) tpie::log_app_debug() << msg
/// \def TP_LOG_DEBUG \deprecated Use \ref log_debug() instead.
#define TP_LOG_DEBUG(msg) tpie::log_debug() << msg
/// \def TP_LOG_MEM_DEBUG \deprecated Use \ref log_mem_debug() instead.
#define TP_LOG_MEM_DEBUG(msg) tpie::log_mem_debug() << msg

#define TP_LOG_ID_MSG __FILE__ << " line " << __LINE__ << ": "

/** \def TP_LOG_FATAL_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_FATAL_ID(msg) TP_LOG_FATAL(TP_LOG_ID_MSG << msg << std::endl)

/** \def TP_LOG_WARNING_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_WARNING_ID(msg) TP_LOG_WARNING(TP_LOG_ID_MSG << msg << std::endl)

/** \def TP_LOG_APP_DEBUG_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_APP_DEBUG_ID(msg) TP_LOG_APP_DEBUG(TP_LOG_ID_MSG << msg << std::endl)

/** \def TP_LOG_DEBUG_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_DEBUG_ID(msg) TP_LOG_DEBUG(TP_LOG_ID_MSG << msg << std::endl)

/** \def TP_LOG_MEM_DEBUG_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_MEM_DEBUG_ID(msg) TP_LOG_MEM_DEBUG(TP_LOG_ID_MSG << msg << std::endl)
    
#else // !TPL_LOGGING
    
// We are not compiling logging.
#define TP_LOG_FATAL(msg) 
#define TP_LOG_WARNING(msg) 
#define TP_LOG_APP_DEBUG(msg)
#define TP_LOG_DEBUG(msg) 
#define TP_LOG_MEM_DEBUG(msg)
    
#define TP_LOG_FATAL_ID(msg)
#define TP_LOG_WARNING_ID(msg)
#define TP_LOG_APP_DEBUG_ID(msg)
#define TP_LOG_DEBUG_ID(msg)
#define TP_LOG_MEM_DEBUG_ID(msg)
    
#define TP_LOG_FLUSH_LOG {}
    
#endif // TPL_LOGGING

}  //  tpie namespace

#endif // _TPIE_LOG_H 
