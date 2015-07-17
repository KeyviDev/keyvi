// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#ifndef TPIE_ATOMIC_H
#define TPIE_ATOMIC_H

#include <tpie/types.h>

///////////////////////////////////////////////////////////////////////////////
/// \file  atomic.h  Atomic integer operations.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

class atomic_int {
	// volatile is necessary on Win32, but it might not be on Linux.
	volatile size_t i;
public:
	atomic_int();
	size_t add_and_fetch(size_t inc);
	size_t sub_and_fetch(size_t inc);
	size_t fetch_and_add(size_t inc);
	size_t fetch_and_sub(size_t inc);
	size_t fetch() const;
	void add(size_t inc);
	void sub(size_t inc);
};

class atomic_stream_size_type {
	volatile stream_size_type i;

public:
	atomic_stream_size_type();
	stream_size_type add_and_fetch(stream_size_type inc);
	stream_size_type sub_and_fetch(stream_size_type inc);
	stream_size_type fetch_and_add(stream_size_type inc);
	stream_size_type fetch_and_sub(stream_size_type inc);
	stream_size_type fetch() const;
	void add(stream_size_type inc);
	void sub(stream_size_type inc);
};

} // namespace tpie

#endif // TPIE_ATOMIC_H
