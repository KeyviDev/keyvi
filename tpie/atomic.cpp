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

#ifdef _WIN32
#include <windows.h>
#undef NO_ERROR
#endif

#include <tpie/atomic.h>

namespace tpie {

atomic_int::atomic_int()
	: i(0)
{
}

size_t atomic_int::fetch() const {
	return i;
}

void atomic_int::add(size_t inc) {
	fetch_and_add(inc);
}

void atomic_int::sub(size_t inc) {
	fetch_and_sub(inc);
}

atomic_stream_size_type::atomic_stream_size_type()
	: i(0)
{
}

stream_size_type atomic_stream_size_type::fetch() const {
	return i;
}

void atomic_stream_size_type::add(stream_size_type inc) {
	fetch_and_add(inc);
}

void atomic_stream_size_type::sub(stream_size_type inc) {
	fetch_and_sub(inc);
}

#ifdef _WIN32

size_t atomic_int::add_and_fetch(size_t inc) {
	return fetch_and_add(inc) + inc;
}

size_t atomic_int::sub_and_fetch(size_t inc) {
	return fetch_and_sub(inc) - inc;
}

size_t atomic_int::fetch_and_add(size_t inc) {
	return InterlockedExchangeAdd(&i, inc);
}

size_t atomic_int::fetch_and_sub(size_t inc) {
	return InterlockedExchangeSubtract(&i, inc);
}

stream_size_type atomic_stream_size_type::add_and_fetch(stream_size_type inc) {
	return fetch_and_add(inc) + inc;
}

stream_size_type atomic_stream_size_type::sub_and_fetch(stream_size_type inc) {
	return fetch_and_sub(inc) - inc;
}

stream_size_type atomic_stream_size_type::fetch_and_add(stream_size_type inc) {
	return InterlockedExchangeAdd64(reinterpret_cast<volatile long long *>(&i), inc);
}

stream_size_type atomic_stream_size_type::fetch_and_sub(stream_size_type inc) {
	return fetch_and_add(-inc);
}

#else // _WIN32

// Linux

size_t atomic_int::add_and_fetch(size_t inc) {
	return __sync_add_and_fetch(&i, inc);
}

size_t atomic_int::sub_and_fetch(size_t inc) {
	return __sync_sub_and_fetch(&i, inc);
}

size_t atomic_int::fetch_and_add(size_t inc) {
	return __sync_fetch_and_add(&i, inc);
}

size_t atomic_int::fetch_and_sub(size_t inc) {
	return __sync_fetch_and_sub(&i, inc);
}

stream_size_type atomic_stream_size_type::add_and_fetch(stream_size_type inc) {
	return __sync_add_and_fetch(&i, inc);
}

stream_size_type atomic_stream_size_type::sub_and_fetch(stream_size_type inc) {
	return __sync_sub_and_fetch(&i, inc);
}

stream_size_type atomic_stream_size_type::fetch_and_add(stream_size_type inc) {
	return __sync_fetch_and_add(&i, inc);
}

stream_size_type atomic_stream_size_type::fetch_and_sub(stream_size_type inc) {
	return __sync_fetch_and_sub(&i, inc);
}

#endif // !_WIN32

} // namespace tpie
