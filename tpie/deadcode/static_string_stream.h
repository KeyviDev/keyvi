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

#ifndef _TPIE_STATIC_STRING_STREAM_H
#define _TPIE_STATIC_STRING_STREAM_H

#include <streambuf>
#include <ostream>
#include <tpie/util.h>

namespace tpie {

class static_string_stream_buff: public std::basic_streambuf<char, std::char_traits<char> > {
private:
	const static size_t buff_size = 20480;
	char m_buff[buff_size];
public:
	inline void clear() {setp(m_buff, m_buff+buff_size-2);}
	inline static_string_stream_buff() {clear();}
	
	inline const char * c_str() {
		*pptr() = 0;
		return m_buff;
	}
	virtual int overflow(int c = traits_type::eof()) {unused(c); return 0;}
	virtual int sync() {return 0;}
};

class static_string_stream: public std::ostream {
private:
	static_string_stream_buff m_buff;
public:
	inline static_string_stream() : std::ostream(&m_buff) {}
	inline  const char * c_str() {return m_buff.c_str();}
	inline void clear() {m_buff.clear();}
};

} //namespace tpie
#endif //_TPIE_STATIC_STRING_STREAM_H
