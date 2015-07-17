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
#ifndef TPIE_LOGPRIORITY_H
#define TPIE_LOGPRIORITY_H

///////////////////////////////////////////////////////////////////////////////
/// \file loglevel.h
/// \brief Logging levels.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/config.h>
namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief TPIE logging levels, from higest priority to lowest.
///////////////////////////////////////////////////////////////////////////////
enum log_level {
	/** LOG_FATAL is the highest error level and is used for all kinds of errors
	 *  that would normally impair subsequent computations; LOG_FATAL errors are
	 *  always logged */
	LOG_FATAL = 0,	
	
	/** LOG_ERROR is used for none fatal errors. */
	LOG_ERROR,
	
	/** LOG_WARNING  is used for warnings. */
	LOG_WARNING,	
	
	/** LOG_INFORMATIONAL is used for informational messagse. */
	LOG_INFORMATIONAL,
	
	/** LOG_APP_DEBUG can be used by applications built on top of TPIE, for 
	 * logging debugging information. */ 
	LOG_APP_DEBUG,     
	
	/** LOG_DEBUG is the lowest level and is used by the TPIE library for 
	 * logging debugging information. */ 
	LOG_DEBUG,
	
	/** Logging level for warnings concerning memory allocation and deallocation. */
	LOG_MEM_DEBUG,
	
	/** Logging levels to be further defined by user applications. */
	LOG_USER1,
	LOG_USER2,
	LOG_USER3
};

} //namespace tpie

#endif //TPIE_LOGPRIORITY_H
