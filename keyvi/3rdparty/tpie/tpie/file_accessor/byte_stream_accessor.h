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

#ifndef TPIE_FILE_ACCESSOR_BYTE_STREAM_ACCESSOR_H
#define TPIE_FILE_ACCESSOR_BYTE_STREAM_ACCESSOR_H

#include <tpie/file_accessor/stream_accessor_base.h>
#include <tpie/tpie_log.h>

namespace tpie {
namespace file_accessor {

template <typename file_accessor_t>
class byte_stream_accessor : public stream_accessor_base<file_accessor_t> {
	typedef stream_accessor_base<file_accessor_t> p_t;
public:
	virtual memory_size_type read_block(void * /*data*/,
										stream_size_type /*blockNumber*/,
										memory_size_type /*itemCount*/) override
	{
		throw exception("Block operations not supported");
	}

	virtual void write_block(const void * /*data*/,
							 stream_size_type /*blockNumber*/,
							 memory_size_type /*itemCount*/) override
	{
		throw exception("Block operations not supported");
	}

	void set_size(stream_size_type amt) {
		p_t::set_size(amt);
	}

	bool empty() {
		return this->size() == 0;
	}

	stream_size_type file_size() {
		return std::max(this->m_fileAccessor.file_size_i(),
						static_cast<stream_size_type>(this->header_size()))
			- this->header_size();
	}

	void truncate_bytes(stream_size_type size) {
		this->m_fileAccessor.truncate_i(this->header_size() + size);
	}

	void write(const stream_size_type byteOffset, const void * data, const memory_size_type size) {
		stream_size_type position = byteOffset + this->header_size();
		this->m_fileAccessor.seek_i(position);
		this->m_fileAccessor.write_i(data, size);
	}

	void append(const void * data, memory_size_type size) {
		stream_size_type position = this->m_fileAccessor.file_size_i();

		if (position < this->header_size())
			position = this->header_size();

		this->m_fileAccessor.seek_i(position);
		this->m_fileAccessor.write_i(data, size);
	}

	memory_size_type read(const stream_size_type byteOffset, void * data, memory_size_type size) {
		stream_size_type sz
			= std::max(static_cast<stream_size_type>(this->header_size()),
					   this->m_fileAccessor.file_size_i())
			- this->header_size();

		if (byteOffset + size > sz)
			size = sz - byteOffset;

		stream_size_type position = this->header_size() + byteOffset;

		this->m_fileAccessor.seek_i(position);
		this->m_fileAccessor.read_i(data, size);
		return size;
	}

	memory_size_type block_items() const {
		return p_t::block_items();
	}

	memory_size_type block_size() const {
		return p_t::block_size();
	}
};

} // namespace file_accessor
} // namespace tpie

#endif // TPIE_FILE_ACCESSOR_BYTE_STREAM_ACCESSOR_H
