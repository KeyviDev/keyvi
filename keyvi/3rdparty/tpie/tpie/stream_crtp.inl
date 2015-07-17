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

#include <tpie/stream_crtp.h>

namespace tpie {

template <typename child_t>
void stream_crtp<child_t>::update_block() {
	if (m_nextBlock == std::numeric_limits<stream_size_type>::max()) {
		m_nextBlock = self().get_block().number+1;
		m_nextIndex = 0;
	}
	self().update_block_core();
	m_blockStartIndex = m_nextBlock*static_cast<stream_size_type>(self().get_file().block_items());
	m_index = m_nextIndex;
	m_nextBlock = std::numeric_limits<stream_size_type>::max();
	m_nextIndex = std::numeric_limits<memory_size_type>::max();
}

} // namespace tpie
