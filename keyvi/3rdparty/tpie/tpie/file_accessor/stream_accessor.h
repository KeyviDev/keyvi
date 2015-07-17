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

#ifndef TPIE_FILE_ACCESSOR_STREAM_ACCESSOR_H
#define TPIE_FILE_ACCESSOR_STREAM_ACCESSOR_H

#include <tpie/file_accessor/stream_accessor_base.h>

namespace tpie {
namespace file_accessor {

template <typename file_accessor_t>
class stream_accessor : public stream_accessor_base<file_accessor_t> {
public:
	virtual memory_size_type read_block(void * data,
										stream_size_type blockNumber,
										memory_size_type itemCount) override
	{
		stream_size_type loc = this->header_size() + blockNumber*this->block_size();
		this->m_fileAccessor.seek_i(loc);
		stream_size_type offset = blockNumber*this->block_items();
		if (offset + itemCount > this->size()) itemCount = static_cast<memory_size_type>(this->size() - offset);
		memory_size_type z=itemCount*this->item_size();
		this->m_fileAccessor.read_i(data, z);
		return itemCount;
	}

	virtual void write_block(const void * data,
							 stream_size_type blockNumber,
							 memory_size_type itemCount) override
	{
		stream_size_type loc = this->header_size() + blockNumber*this->block_size();
		// Here, we may seek beyond the file size.
		// However, lseek(2) specifies that the file will be padded with zeroes in this case,
		// and on Windows, the file is padded with arbitrary garbage (which is ok).
		this->m_fileAccessor.seek_i(loc);
		stream_size_type offset = blockNumber*this->block_items();
		memory_size_type z=itemCount*this->item_size();
		this->m_fileAccessor.write_i(data, z);
		if (offset+itemCount > this->size()) this->set_size(offset+itemCount);
	}
};

} // namespace tpie
} // namespace file_accessor

#endif // TPIE_FILE_ACCESSOR_STREAM_ACCESSOR_H
