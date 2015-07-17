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
#include <string.h>
#include <tpie/exception.h>
#include <tpie/file_count.h>
#include <tpie/file_accessor/stream_accessor_base.h>
#include <tpie/stats.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits>
#include <tpie/compressed/scheme.h>

namespace tpie {
namespace file_accessor {

template <typename file_accessor_t>
void stream_accessor_base<file_accessor_t>::read_header() {
	stream_header_t header;
	m_fileAccessor.seek_i(0);
	m_fileAccessor.read_i(&header, sizeof(header));
	validate_header(header);
	m_size = header.size;
	m_userDataSize = (size_t)header.userDataSize;
	m_maxUserDataSize = (size_t)header.maxUserDataSize;
	m_lastBlockReadOffset = header.lastBlockReadOffset;
	m_useCompression = header.get_compressed();
}

template <typename file_accessor_t>
void stream_accessor_base<file_accessor_t>::write_header(bool clean) {
	stream_header_t header;
	fill_header(header, clean);
	m_fileAccessor.seek_i(0);
	m_fileAccessor.write_i(&header, sizeof(header));
}

template <typename file_accessor_t>
memory_size_type stream_accessor_base<file_accessor_t>::read_user_data(void * data, memory_size_type count) {
	if (count > m_userDataSize) count = m_userDataSize;
	if (count) {
		m_fileAccessor.seek_i(sizeof(stream_header_t));
		m_fileAccessor.read_i(data, count);
	}
	return count;
}

template <typename file_accessor_t>
void stream_accessor_base<file_accessor_t>::write_user_data(const void * data, memory_size_type count) {
	if (count > m_maxUserDataSize)
		throw stream_exception("Tried to write more user data than stream allows");
	if (count) {
		m_fileAccessor.seek_i(sizeof(stream_header_t));
		m_fileAccessor.write_i(data, count);
	}
	m_userDataSize = count;
}

template <typename file_accessor_t>
void stream_accessor_base<file_accessor_t>::validate_header(const stream_header_t & header) {
	if (header.magic != stream_header_t::magicConst)
		throw invalid_file_exception("Invalid file, header magic wrong");

	if (header.version != stream_header_t::versionConst)
		throw invalid_file_exception("Invalid file, header version wrong");

	if (header.itemSize != m_itemSize)
		throw invalid_file_exception("Invalid file, item size is wrong");

	if (header.blockSize != m_blockSize)
		throw invalid_file_exception("Invalid file, block size is wrong");

	if (header.userDataSize > header.maxUserDataSize)
		throw invalid_file_exception("Invalid file, user data size is greater than max user data size");

	if (header.get_clean_close() == false)
		throw invalid_file_exception("Invalid file, the file was not closed properly");
}

template <typename file_accessor_t>
void stream_accessor_base<file_accessor_t>::fill_header(stream_header_t & header, bool clean) {
	header.magic = stream_header_t::magicConst;
	header.version = stream_header_t::versionConst;
	header.itemSize = m_itemSize;
	header.blockSize = m_blockSize;
	header.set_clean_close(clean);
	header.userDataSize = m_userDataSize;
	header.maxUserDataSize = m_maxUserDataSize;
	header.size = m_size;
	header.lastBlockReadOffset = m_lastBlockReadOffset;
	header.set_compressed(m_useCompression);
}

template <typename file_accessor_t>
void stream_accessor_base<file_accessor_t>::open(const std::string & path,
											bool read,
											bool write,
											memory_size_type itemSize,
											memory_size_type blockSize,
											memory_size_type maxUserDataSize,
											cache_hint cacheHint,
											int compressionFlags) {
	close();
	m_write = write;
	m_path = path;
	m_itemSize=itemSize;
	m_blockSize=blockSize;
	m_blockItems=blockSize/itemSize;
	m_userDataSize=0;
	m_maxUserDataSize=maxUserDataSize;
	m_size=0;
	m_fileAccessor.set_cache_hint(cacheHint);
	m_compressionFlags = compressionFlags;
	m_useCompression = compressionFlags != compression_scheme::none;
	m_lastBlockReadOffset = std::numeric_limits<stream_size_type>::max();
	if (!write && !read)
		throw invalid_argument_exception("Either read or write must be specified");
	if (write && !read) {
		m_fileAccessor.open_wo(path);
		write_header(false);
		write_user_data(0, 0);
	} else if (!write && read) {
		m_fileAccessor.open_ro(path);
		read_header();
	} else {
		if (!m_fileAccessor.try_open_rw(path)) {
			m_fileAccessor.open_rw_new(path);
			write_header(false);
			write_user_data(0, 0);
		} else {
			read_header();
			write_header(false);
		}
	}
	increment_open_file_count();
	m_open = true;
	if (write && m_maxUserDataSize < maxUserDataSize) {
		close();
		throw invalid_file_exception("Invalid file, max user data size not large enough");
	}
}

template <typename file_accessor_t>
void stream_accessor_base<file_accessor_t>::close() {
	if (!m_open)
		return;
	if (m_write)
		write_header(true);
	m_fileAccessor.close_i();
	decrement_open_file_count();
	m_open = false;
}

template <typename file_accessor_t>
void stream_accessor_base<file_accessor_t>::truncate(stream_size_type items) {
	if (m_useCompression && items != 0)
		throw exception("stream_accessor_base cannot truncate compressed stream");
	stream_size_type blocks = items/m_blockItems;
	stream_size_type blockIndex = items%m_blockItems;
	stream_size_type bytes = header_size() + blocks*m_blockSize + blockIndex*m_itemSize;
	m_fileAccessor.truncate_i(bytes);
	m_size = items;
}

}
}
