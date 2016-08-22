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
#include <tpie/portability.h>

#include <string.h>
#include <tpie/exception.h>
#include <tpie/file_manager.h>
#include <tpie/file_accessor/win32.h>
#include <tpie/util.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <winerror.h>
#include <sstream>

namespace tpie {
namespace file_accessor {

using tpie::throw_getlasterror;

win32::win32()
	: m_fd(INVALID_HANDLE_VALUE)
	, m_creationFlag(0)
{
}

inline void win32::read_i(void * data, memory_size_type size) {
	DWORD bytesRead = 0;
	if (!ReadFile(m_fd, data, (DWORD)size, &bytesRead, 0)) throw_getlasterror();
	if (bytesRead != size) {
		std::stringstream ss;
		ss << "Wrong number of bytes read: Expected " << size << " but got " << bytesRead;
		throw io_exception(ss.str());
	}
	increment_bytes_read(size);
}

inline void win32::write_i(const void * data, memory_size_type size) {
	DWORD bytesWritten = 0;
	if (!WriteFile(m_fd, data, (DWORD)size, &bytesWritten, 0) || bytesWritten != size ) throw_getlasterror();
	increment_bytes_written(size);
}

inline void win32::seek_i(stream_size_type size) {
	LARGE_INTEGER i;
	i.QuadPart = size;
	if (!SetFilePointerEx(m_fd, i, NULL, 0)) throw_getlasterror();
}

inline stream_size_type win32::file_size_i() {
	LARGE_INTEGER i;
	if (!GetFileSizeEx(m_fd, &i)) throw_getlasterror();
	// i.QuadPart is a signed long long
	return static_cast<stream_size_type>(i.QuadPart);
}

static const DWORD shared_flags = FILE_SHARE_READ | FILE_SHARE_WRITE;

void win32::set_cache_hint(cache_hint cacheHint) {
	switch (cacheHint) {
		case access_normal:
			m_creationFlag = 0;
			break;
		case access_sequential:
			m_creationFlag = FILE_FLAG_SEQUENTIAL_SCAN;
			break;
		case access_random:
			m_creationFlag = FILE_FLAG_RANDOM_ACCESS;
			break;
	}
}

void win32::_open(const std::string & path, DWORD access, DWORD create_mode) {
	m_fd = CreateFile(path.c_str(), access, shared_flags, 0, create_mode, m_creationFlag, 0);
	if (m_fd == INVALID_HANDLE_VALUE) return;

	get_file_manager().increment_open_file_count();
}

void win32::open_wo(const std::string & path) {
	_open(path, GENERIC_WRITE, CREATE_ALWAYS);
	if (m_fd == INVALID_HANDLE_VALUE) throw_getlasterror();
}

void win32::open_ro(const std::string & path) {
	_open(path, GENERIC_READ, OPEN_EXISTING);
	if (m_fd == INVALID_HANDLE_VALUE) throw_getlasterror();
}

bool win32::try_open_rw(const std::string & path) {
	_open(path, GENERIC_READ | GENERIC_WRITE, OPEN_EXISTING);
	if (m_fd == INVALID_HANDLE_VALUE) {
		if (GetLastError() != ERROR_FILE_NOT_FOUND) throw_getlasterror();
		return false;
	}
	return true;
}

void win32::open_rw_new(const std::string & path) {
	_open(path, GENERIC_READ | GENERIC_WRITE, CREATE_NEW);
	if (m_fd == INVALID_HANDLE_VALUE) throw_getlasterror();
}

bool win32::is_open() const {
	return m_fd != INVALID_HANDLE_VALUE;
}

void win32::close_i() {
	if (m_fd != INVALID_HANDLE_VALUE) {
		if(CloseHandle(m_fd)) {
			get_file_manager().decrement_open_file_count();
		}
	}
	m_fd=INVALID_HANDLE_VALUE;
}

void win32::truncate_i(stream_size_type size) {
	LARGE_INTEGER i;
	i.QuadPart = size;
	SetFilePointerEx(m_fd, i, NULL, 0);
	if (!SetEndOfFile(m_fd)) throw_getlasterror();
}

}
}
