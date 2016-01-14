// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2015, The TPIE development team
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

#include <tpie/blocks/block_collection_cache.h>
#include <tpie/btree/external_store_base.h>

namespace tpie {
namespace bbits {

external_store_base::external_store_base(const std::string & path)
: m_root()
, m_path(path)
, m_height(0)
, m_size(0)
{
	tpie::file_accessor::raw_file_accessor m_accessor;
	if(m_accessor.try_open_rw(path)) {
		if(m_accessor.file_size_i() > 0) {
			stream_size_type size = sizeof(size_t) * 2 + sizeof(blocks::block_handle);
			tp_assert(m_accessor.file_size_i() >= size, "file is not empty but does not contain size btree_external_store information");
			stream_size_type pos = m_accessor.file_size_i() - size;
			m_accessor.seek_i(pos);
			m_accessor.read_i((void*) &m_height, sizeof(size_t));
			m_accessor.seek_i(pos + sizeof(size_t));
			m_accessor.read_i((void*) &m_size, sizeof(size_t));
			m_accessor.seek_i(pos + 2 * sizeof(size_t));
			m_accessor.read_i((void*) &m_root, sizeof(blocks::block_handle));
			m_accessor.truncate_i(pos);
		}
	}
}

external_store_base::~external_store_base() {
	tpie::file_accessor::raw_file_accessor m_accessor;
	m_accessor.try_open_rw(m_path);
	stream_size_type size = sizeof(size_t) * 2 + sizeof(blocks::block_handle);
	stream_size_type pos = m_accessor.file_size_i();
	m_accessor.truncate_i(pos + size);
	m_accessor.seek_i(pos);
	m_accessor.write_i((void*) &m_height, sizeof(size_t));
	m_accessor.seek_i(pos + sizeof(size_t));
	m_accessor.write_i((void*) &m_size, sizeof(size_t));
	m_accessor.seek_i(pos + 2 * sizeof(size_t));
	m_accessor.write_i((void*) &m_root, sizeof(blocks::block_handle));
}

} // namespace bits
} // namespace tpie
