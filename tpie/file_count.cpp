// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, 2012, The TPIE development team
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
#include <tpie/file_count.h>
#include <tpie/portability.h>
#ifndef _WIN32
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace {

inline int get_maximum_open_files() {
#ifdef _WIN32
	return 512;
#else
	return getdtablesize();
#endif
}

}

namespace tpie {

memory_size_type available_files() {
#ifdef _WIN32
	return get_maximum_open_files();
#else
	// skip to the first unused file descriptor
	int nextfd = dup(0);
	if (nextfd == -1) {
		// all files must be in use
		return 0;
	}
	close(nextfd);

	int count = 1;
	++nextfd; // `nextfd' is a fd that is not in use

	int maxfd = get_maximum_open_files();
	while (nextfd < maxfd) {
		if (-1 == fcntl(nextfd, F_GETFD)) ++count;
		++nextfd;
	}
	return count;
#endif
}

memory_size_type open_file_count() {
	return get_maximum_open_files()-available_files();
}

}
