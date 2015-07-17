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

#include <queue>
#include <set>
#include <tpie/pipelining/tokens.h>
#include <tpie/pipelining/node.h>

namespace tpie {

namespace pipelining {

namespace bits {

node_map::id_t node_map::nextId = 0;

void node_map::link(node_map::ptr target) {
	if (target.get() == this) {
		// self link attempted
		// we must never have some_map->m_authority point to some_map,
		// since it would create a reference cycle
		return;
	}
	// union by rank
	if (target->m_rank > m_rank)
		return target->link(this);

	for (mapit i = target->begin(); i != target->end(); ++i) {
		set_token(i->first, i->second);
	}
	for (relmapit i = target->m_relations.begin(); i != target->m_relations.end(); ++i) {
		m_relations.insert(*i);
	}
	for (relmapit i = target->m_relationsInv.begin(); i != target->m_relationsInv.end(); ++i) {
		m_relationsInv.insert(*i);
	}
	target->m_tokens.clear();
	target->m_authority = this;

	// union by rank
	if (target->m_rank == m_rank)
		++m_rank;
}

node_map::ptr node_map::find_authority() {
	if (!m_authority)
		return this;

	node_map * i = m_authority.get();
	while (i->m_authority) {
		i = i->m_authority.get();
	}
	ptr result(i);

	// path compression
	node_map * j = m_authority.get();
	while (j->m_authority) {
		node_map * k = j->m_authority.get();
		j->m_authority = result;
		j = k;
	}

	return result;
}

void node_map::add_relation(id_t from, id_t to, node_relation rel) {
	// Check that the edge is not already in the "set"
	std::pair<relmapit, relmapit> is = m_relations.equal_range(from);
	for (relmapit i = is.first; i != is.second; ++i) {
		if (i->second.first == to && i->second.second == rel) return;
	}

	// Insert edge
	m_relations.insert(std::make_pair(from, std::make_pair(to, rel)));
	m_relationsInv.insert(std::make_pair(to, std::make_pair(from, rel)));
}

size_t node_map::out_degree(const relmap_t & map, id_t from, node_relation rel) const {
	size_t res = 0;
	std::pair<relmapit, relmapit> is = map.equal_range(from);
	for (relmapit i = is.first; i != is.second; ++i) {
		if (i->second.second == rel) ++res;
	}
	return res;
}

size_t node_map::out_degree(const relmap_t & map, id_t from) const {
	std::pair<relmapit, relmapit> is = map.equal_range(from);
	return std::distance(is.first, is.second);
}

void node_map::get_successors(id_t from, std::vector<id_t> & successors, bool forward_only) {
	std::queue<id_t> q;
	std::set<id_t> seen;
	q.push(from);
	while (!q.empty()) {
		id_t v = q.front();
		q.pop();
		if (seen.count(v)) continue;
		seen.insert(v);
		successors.push_back(v);
		{
			std::pair<relmapit, relmapit> is = m_relations.equal_range(v);
			for (relmapit i = is.first; i != is.second; ++i) {
				switch (i->second.second) {
					case pushes:
						q.push(i->second.first);
						break;
					case pulls:
					case depends:
					case no_forward_depends:
						break;
				}
			}
		}
		{
			std::pair<relmapit, relmapit> is = m_relationsInv.equal_range(v);
			for (relmapit i = is.first; i != is.second; ++i) {
				switch (i->second.second) {
					case pushes:
						break;
					case pulls:
					case depends:
						q.push(i->second.first);
						break;
					case no_forward_depends:
						if (!forward_only)
							q.push(i->second.first);
						break;
				}
			}
		}
	}
}

} // namespace bits

} // namespace pipelining

} // namespace tpie
