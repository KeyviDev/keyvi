// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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

#include <tpie/file_base_crtp.h>
#include <tpie/file_accessor/file_accessor.h>

namespace tpie {

template <typename child_t>
file_base_crtp<child_t>::file_base_crtp(
	memory_size_type itemSize, double blockFactor,
	file_accessor::file_accessor * fileAccessor) {
	m_size = 0;
	m_itemSize = itemSize;
	m_canRead = false;
	m_canWrite = false;
	m_open = false;
	if (fileAccessor == 0)
		fileAccessor = new default_file_accessor();
	m_fileAccessor = fileAccessor;

	m_blockSize = block_size(blockFactor);
	m_blockItems = m_blockSize/m_itemSize;
	m_tempFile = 0;
}

template <typename child_t>
template <typename BT>
void file_base_crtp<child_t>::read_block(BT & b, stream_size_type block) {
	b.dirty = false;
	b.number = block;

	// calculate buffer size
	b.size = m_blockItems;
	if (static_cast<stream_size_type>(b.size) + b.number * static_cast<stream_size_type>(m_blockItems) > self().size())
		b.size = static_cast<memory_size_type>(self().size() - block * m_blockItems);

	// populate buffer data
	if (b.size > 0 &&
		m_fileAccessor->read_block(b.data, b.number, b.size) != b.size) {
		throw io_exception("Incorrect number of items read");
	}
}

template <typename child_t>
void file_base_crtp<child_t>::get_block_check(stream_size_type block) {
	// If the file contains n full blocks (numbered 0 through n-1), we may
	// request any block in {0, 1, ... n}

	// If the file contains n-1 full blocks and a single non-full block, we may
	// request any block in {0, 1, ... n-1}

	// We capture this restraint with the assertion:
	if (block * static_cast<stream_size_type>(m_blockItems) > self().size()) {
		throw end_of_stream_exception();
	}
}

} // namespace tpie
