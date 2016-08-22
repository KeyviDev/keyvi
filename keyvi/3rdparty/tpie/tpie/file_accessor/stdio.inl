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
#include <tpie/config.h>
//#include <tpie/stream/stdio_bte.h>
//#include <tpie/stream/header.h>
#include <string.h>
#include <tpie/exception.h>
#include <tpie/file_manager.h>
#include <tpie/file_accessor/stdio.h>
#include <cstdio>

#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

namespace tpie {
namespace file_accessor {

#ifdef _WIN32
#define fseeko _fseeki64
#endif

stdio::stdio():
	m_fd(0) {
	invalidateLocation();
}

inline void stdio::read_i(void * data, memory_size_type size) {
	if (::fread(data, 1, size, m_fd) != size) throw_errno();
}

inline void stdio::write_i(const void * data, memory_size_type size) {
	if (::fwrite(data, 1, size, m_fd) != size) throw_errno();
}

inline void stdio::seek_i(stream_size_type offset) {
	if (::fseeko(m_fd, offset, SEEK_SET) != 0) throw_errno();
}
	
void stdio::open(const std::string & path,
				 bool read,
				 bool write,
				 memory_size_type itemSize,
				 memory_size_type blockSize,
				 memory_size_type userDataSize) {
	close();
	invalidateLocation();
	m_write = write;
	m_path = path;
	m_itemSize=itemSize;
	m_blockSize=blockSize;
	m_blockItems=blockSize/itemSize;
	m_userDataSize=userDataSize;
	if (!write && !read)
		throw invalid_argument_exception("Either read or write must be specified");
	if (write && !read) {
		m_fd = ::fopen(path.c_str(), "wb");
		if (m_fd == 0) throw_errno();
		m_size = 0;
		write_header(false);
		char * buf = new char[userDataSize];
		write_user_data(buf);
		delete[] buf;
	} else if (!write && read) {
		m_fd = ::fopen(path.c_str(), "rb");
		if (m_fd == 0) throw_errno();
		read_header();
	} else {
		m_fd = ::fopen(path.c_str(), "r+b");
		if (m_fd == 0) {
			if (errno != ENOENT) throw_errno();
			m_fd = ::fopen(path.c_str(), "w+b");
			if (m_fd == 0) throw_errno();
			m_size=0;
			write_header(false);
			char * buf = new char[userDataSize];
			write_user_data(buf);
			delete[] buf;
		} else {
			read_header();
			write_header(false);
		}
	}
	get_file_manager().increment_open_file_count();
	setvbuf(m_fd, NULL, _IONBF, 0);
}

void stdio::close() {
	if (m_fd && m_write) write_header(true);
	if (m_fd != 0) {
		::fclose(m_fd);
		get_file_manager().decrement_open_file_count();
	}
	m_fd=0;
}

void stdio::truncate(stream_size_type size) {
#ifndef WIN32
	if (::truncate(m_path.c_str(), sizeof(stream_header_t) + m_userDataSize + size*m_itemSize) == -1) throw_errno();
#else
	//Since there is no reliable way of trunacing a file, we will just fake it
	if (size > m_size) {
		char * buff = new char[m_blockSize];
		while (size > m_size) {
			write_block(buff, m_size/m_blockItems, std::min(m_blockItems, static_cast<memory_size_type>(size-m_size)));
		}
		delete [] buff;
	}
#endif
	invalidateLocation();
	m_size = size;
}

}
}
