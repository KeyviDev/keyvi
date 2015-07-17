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

#ifndef __TPIE_PIPELINING_NODE_MAP_DUMP_H__
#define __TPIE_PIPELINING_NODE_MAP_DUMP_H__

#include <tpie/pipelining/tokens.h>
#include <tpie/pipelining/node.h>

namespace tpie {

namespace pipelining {

namespace bits {

inline void node_map::dump(std::ostream & os) const {
	os << this << " node_map\n";
	if (m_authority)
		os << "Non-authoritative" << std::endl;
	else
		os << "Authoritative" << std::endl;
	for (mapit i = m_tokens.begin(); i != m_tokens.end(); ++i) {
		os << i->first << " -> " << i->second << std::endl;
	}
	for (relmapit i = m_relations.begin(); i != m_relations.end(); ++i) {
		os << i->first << " -> " << i->second.first << " edge type " << i->second.second << std::endl;
	}
}

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_NODE_MAP_DUMP_H__
