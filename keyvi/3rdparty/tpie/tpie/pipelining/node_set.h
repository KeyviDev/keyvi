// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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

#ifndef __TPIE_PIPELINING_NODE_SET_H__
#define __TPIE_PIPELINING_NODE_SET_H__
#include <vector>
#include <boost/intrusive_ptr.hpp>
#include <tpie/pipelining/tokens.h>

namespace tpie {
namespace pipelining {

class node;
  
namespace bits {

struct node_set_content {
	size_t m_refCnt;
	node_map::ptr m_map;
	std::vector<node_map::id_t> m_nodes;
	std::vector<std::pair<node_map::id_t, node_relation> > m_relations;
	
	friend void intrusive_ptr_add_ref(node_set_content * s) {
		s->m_refCnt++;
	}
	
	friend void intrusive_ptr_release(node_set_content * s) {
		s->m_refCnt--;
		if (s->m_refCnt == 0) delete s;
	}
};

} // namespace bits

typedef boost::intrusive_ptr<bits::node_set_content> node_set;

inline node_set make_node_set() {
	return new bits::node_set_content();
}

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_NODE_SET_H__
