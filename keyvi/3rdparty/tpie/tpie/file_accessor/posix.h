// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2010, The TPIE development team
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
/// \file posix.h  POSIX-style file accessor
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_FILE_ACCESSOR_POSIX_H
#define _TPIE_FILE_ACCESSOR_POSIX_H

#include <tpie/file_accessor/stream_accessor_base.h>
namespace tpie {
namespace file_accessor {

///////////////////////////////////////////////////////////////////////////////
/// \brief POSIX-style file accessor.
///////////////////////////////////////////////////////////////////////////////

class posix {
private:
	int m_fd;
	cache_hint m_cacheHint;

public:
	inline posix();
	inline ~posix() {close_i();}

	inline void open_ro(const std::string & path);
	inline void open_wo(const std::string & path);
	inline bool try_open_rw(const std::string & path);
	inline void open_rw_new(const std::string & path);

	inline void read_i(void * data, memory_size_type size);
	inline void write_i(const void * data, memory_size_type size);
	inline void seek_i(stream_size_type offset);
	inline stream_size_type file_size_i();
	inline void close_i();
	inline void truncate_i(stream_size_type bytes);
	inline bool is_open() const;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check the global errno variable and throw an exception that
	/// matches its value.
	///////////////////////////////////////////////////////////////////////////
	static inline void throw_errno();

	inline void set_cache_hint(cache_hint cacheHint);

private:
	inline void give_advice();
};

}
}

#include <tpie/file_accessor/posix.inl>

#endif //_TPIE_FILE_ACCESSOR_POSIX_H
