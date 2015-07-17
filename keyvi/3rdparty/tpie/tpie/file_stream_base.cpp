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

#include <tpie/file_stream_base.h>
#include <tpie/file_base_crtp.inl>
#include <tpie/stream_crtp.inl>

namespace tpie {

file_stream_base::file_stream_base(memory_size_type itemSize,
								   double blockFactor,
								   file_accessor::file_accessor * fileAccessor):
	file_base_crtp<file_stream_base>(itemSize, blockFactor, fileAccessor)
{
	m_blockStartIndex = 0;
	m_nextBlock = std::numeric_limits<stream_size_type>::max();
	m_nextIndex = std::numeric_limits<memory_size_type>::max();
	m_index = std::numeric_limits<memory_size_type>::max();
	m_block.data = 0;
	m_block.dirty = false;
}

void file_stream_base::get_block(stream_size_type block) {
	get_block_check(block);
	read_block(m_block, block);
}

void file_stream_base::update_block_core() {
	flush_block();
	get_block(m_nextBlock);
}

template class stream_crtp<file_stream_base>;
template class file_base_crtp<file_stream_base>;

} // namespace tpie
