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
/// \file tpie/exception.h
///
/// Exception classes.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_EXCEPTION_H__
#define __TPIE_EXCEPTION_H__
#include <stdexcept>
#include <string>

namespace tpie {

struct exception: public std::runtime_error {
	exception(const std::string & s): std::runtime_error(s) {};
};

struct maybe_exception : public exception {
	maybe_exception(const std::string & s): exception(s) {};
};

struct invalid_argument_exception: public exception {
	invalid_argument_exception(const std::string & s): exception(s) {};
};

struct stream_exception : public exception {
	stream_exception(): exception("") {};
	stream_exception(const std::string & s): exception(s) {};
};

struct io_exception: public stream_exception {
	io_exception(const std::string & s): stream_exception(s) {};
};

struct out_of_space_exception: public io_exception {
	out_of_space_exception(const std::string & s): io_exception(s) {};
};

struct invalid_file_exception: public stream_exception {
	invalid_file_exception(const std::string & s): stream_exception(s) {};
};

struct end_of_stream_exception: public stream_exception {
	end_of_stream_exception(): stream_exception("") {};
};

struct job_manager_exception: public exception {
	job_manager_exception(): exception("") {};
};

}
#endif //__TPIE_EXCEPTION_H__
