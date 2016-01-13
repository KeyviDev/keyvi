// -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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
/// \file util.cpp  Miscellaneous utility functions - implementation
///////////////////////////////////////////////////////////////////////////////

#include <tpie/exception.h>
#include <tpie/util.h>
#include <stdexcept>
#include <cstdio>
#ifndef WIN32
#include <tpie/tpie_log.h>
#include <tpie/file_accessor/posix.h>
#else
#include <windows.h>
#endif

namespace tpie {

void atomic_rename(const std::string & src, const std::string & dst) {
	//Note according to posix rename is atomic..
	//On windows it is probably not
#ifndef _WIN32
	if (rename(src.c_str(), dst.c_str()) != 0) {
		log_debug() << "Atomic rename failed from ``" << src << "'' to ``" << dst << "''." << std::endl;
		file_accessor::posix::throw_errno();
	}
#else
	//TODO use MoveFileTransacted on vista or newer
	if (!MoveFileEx(src.c_str(), dst.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		throw std::runtime_error("Atomic rename failed");
#endif
}

#ifdef _WIN32
void throw_getlasterror() {
	char buffer[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buffer, 1023, 0);
	switch (GetLastError()) {
		case ERROR_HANDLE_DISK_FULL:
		case ERROR_DISK_FULL:
		case ERROR_DISK_TOO_FRAGMENTED:
		case ERROR_DISK_QUOTA_EXCEEDED:
		case ERROR_VOLMGR_DISK_NOT_ENOUGH_SPACE:
			throw out_of_space_exception(buffer);
		default:
			throw io_exception(buffer);
	}
}
#endif

} // namespace tpie
