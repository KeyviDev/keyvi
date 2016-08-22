// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, 2013, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \brief pipelining/factory_base.h  Base class of pipelining factories
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_FACTORY_BASE_H__
#define __TPIE_PIPELINING_FACTORY_BASE_H__
#include <tpie/pipelining/node_set.h>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/tokens.h>

namespace tpie {

namespace pipelining {

class factory_init_hook {
public:
	virtual void init_node(node & r) = 0;
	virtual ~factory_init_hook() {
	}
};

struct destination_kind {
	enum type {
		none,
		push,
		pull
	};
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Base class of all pipelining factories.
///
/// The subclass must define an inner template struct named \c constructed,
/// that takes one template parameter \c dest_t and defines an inner typedef
/// named \c type that is the actual type of node constructed.
///
/// If the factory \c foo_factory constructs \c foo objects, then the type
/// expression <tt>foo_factory::constructed&lt;bar&lt;baz&gt; &gt;::type</tt>
/// should be equal to <tt>foo&lt;bar&lt;baz&gt; &gt;</tt>.
///
/// The subclass must also define a template const method named \c construct,
/// that takes one template parameter \c dest_t, has the return type
/// <tt>constructed&lt;dest_t&gt;::type</tt> and takes just one parameter
/// \c dest of type \c dest_t.
///
/// If the factory constructs just one \c node descendent, then the factory
/// should call the \c factory_base::init_node method with this \c node as
/// parameter. For example see tpie/pipelining/factory_helpers.h.
///
/// If the factory constructs multiple \c node descendents, then the factory
/// should call the \c factory_base::init_sub_node method for each of the
/// \c nodes. For example see \c sort_factory_base in tpie/pipelining/sort.h.
///////////////////////////////////////////////////////////////////////////////
class factory_base {
public:
	factory_base() : m_amount(0), m_set(false), m_destinationKind(destination_kind::none) {
	}

	factory_base(const factory_base & other) = delete;
	factory_base(factory_base &&) = default;
	factory_base & operator=(const factory_base & other) = delete;
	factory_base & operator=(factory_base &&) = default;

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief bits::pipe_base::memory(double)
	/// \copydetails bits::pipe_base::memory(double)
	///
	/// \sa bits::pipe_base::memory(double)
	///////////////////////////////////////////////////////////////////////////
	inline void memory(double amount) {
		m_amount = amount;
		m_set = true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief bits::pipe_base::memory()
	/// \copydetails bits::pipe_base::memory()
	///
	/// \sa memory(double)
	/// \sa bits::pipe_base::memory()
	///////////////////////////////////////////////////////////////////////////
	inline double memory() const {
		return m_amount;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Add a node initialization hook.
	///
	/// When a node is instantiated in construct(), the given hook will get a
	/// chance to do some additional initialization.
	///////////////////////////////////////////////////////////////////////////
	void hook_initialization(factory_init_hook * hook) {
		m_hooks.push_back(hook);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Copy the hooks that have been added to this factory to another.
	///////////////////////////////////////////////////////////////////////////
	void copy_hooks_to(factory_base & other) const {
		for (size_t i = 0; i < m_hooks.size(); ++i) {
			other.m_hooks.push_back(m_hooks[i]);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Initialize node constructed in a subclass.
	///
	/// This lets the user define a name or memory fraction for this certain
	/// node in the pipeline phase, and it lets initialization hooks do their
	/// thing.
	///
	/// If more than one node is constructed in the subclass in \c construct(),
	/// the implementation should use \c init_sub_node instead.
	///////////////////////////////////////////////////////////////////////////
	inline void init_node(node & r) {
		if (!m_name.empty()) {
			r.set_name(m_name, m_namePriority);
		}
		if (!m_phaseName.empty()) {
			r.set_phase_name(m_phaseName, m_phaseNamePriority);
		}
		if (!m_breadcrumbs.empty()) {
			r.set_breadcrumb(m_breadcrumbs);
		}
		init_common(r);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Initialize node constructed in a subclass.
	///
	/// This lets the user define a name or memory fraction for this certain
	/// node in the pipeline phase, and it lets initialization hooks do their
	/// thing.
	///
	/// If just one node is constructed in the subclass in \c construct(),
	/// the implementation should use \c init_node instead.
	///////////////////////////////////////////////////////////////////////////
	void init_sub_node(node & r) {
		if (m_breadcrumbs.empty()) {
			if (m_name.empty()) {
				// no op
			} else {
				r.set_breadcrumb(m_name);
			}
		} else {
			if (m_name.empty()) {
				r.set_breadcrumb(m_breadcrumbs);
			} else {
				r.set_breadcrumb(m_breadcrumbs + " | " + m_name);
			}
		}
		init_common(r);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used by pipe_base classes to set a default actor edge
	/// for ordinary push/pull nodes.
	///////////////////////////////////////////////////////////////////////////
	void add_default_edge(node & r, const node & dest) const {
		add_default_edge(r, dest.get_token());
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used by pipe_base classes to set a default actor edge
	/// for ordinary push/pull nodes.
	///////////////////////////////////////////////////////////////////////////
	void add_default_edge(node & r, const node_token & dest) const {
		if (r.get_node_map()->find_authority()->out_degree(r.get_id()) > 0) return;
		switch (m_destinationKind) {
		case destination_kind::none:
			break;
		case destination_kind::push:
			r.add_push_destination(dest);
			break;
		case destination_kind::pull:
			r.add_pull_source(dest);
			break;
		}
	}

	void add_node_set_edges(node & r) const {
		bits::node_map::ptr m=r.get_node_map();
		for (size_t i=0; i < m_add_to_set.size(); ++i) {
			bits::node_map::ptr m2=m_add_to_set[i]->m_map;
			if (m2 && m2 != m)
				m2->union_set(m);
		}
		for (size_t i=0; i < m_add_relations.size(); ++i) {
			bits::node_map::ptr m2=m_add_relations[i].first->m_map;
			if (m2 && m2 != m)
				m2->union_set(m);
		}
		m = m->find_authority();
		for (size_t i=0; i < m_add_to_set.size(); ++i)
			m_add_to_set[i]->m_map = m;
		for (size_t i=0; i < m_add_relations.size(); ++i)
			m_add_relations[i].first->m_map = m;

		for (size_t i=0; i < m_add_to_set.size(); ++i) {
			node_set s=m_add_to_set[i];
			for (size_t j=0; j < s->m_relations.size(); ++j)
				m->add_relation(s->m_relations[j].first, r.get_id(), s->m_relations[j].second);
			s->m_nodes.push_back(r.get_id());
		}

		for (size_t i=0; i < m_add_relations.size(); ++i) {
			node_set s=m_add_relations[i].first;
			bits::node_relation relation = m_add_relations[i].second;
			for (size_t j=0; j < s->m_nodes.size(); ++j)
				m->add_relation(r.get_id(), s->m_nodes[j], relation);
			s->m_relations.push_back(std::make_pair(r.get_id(), relation));
		}
	}

	
	///////////////////////////////////////////////////////////////////////////
	/// \copybrief bits::pipe_base::name
	/// \copydetails bits::pipe_base::name
	///
	/// \sa bits::pipe_base::name
	///////////////////////////////////////////////////////////////////////////
	inline void name(const std::string & n, priority_type p) {
		m_name = n;
		m_namePriority = p;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief bits::pipe_base::phase_name
	/// \copydetails bits::pipe_base::phase_name
	///
	/// \sa bits::pipe_base::phase_name
	///////////////////////////////////////////////////////////////////////////
	inline void phase_name(const std::string & n, priority_type p) {
		m_phaseName = n;
		m_phaseNamePriority = p;
	}

	
	///////////////////////////////////////////////////////////////////////////
	/// \copybrief bits::pipe_base::breadcrumb
	/// \copydetails bits::pipe_base::breadcrumb
	///
	/// \sa bits::pipe_base::breadcrumb
	///////////////////////////////////////////////////////////////////////////
	inline void push_breadcrumb(const std::string & n) {
		if (m_breadcrumbs.empty()) m_breadcrumbs = n;
		else m_breadcrumbs = n + " | " + m_breadcrumbs;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used by pipe_base classes to indicate that the default
	/// actor edge is a push edge.
	///////////////////////////////////////////////////////////////////////////
	void set_destination_kind_push() {
		m_destinationKind = destination_kind::push;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used by pipe_base classes to indicate that the default
	/// actor edge is a pull edge.
	///////////////////////////////////////////////////////////////////////////
	void set_destination_kind_pull() {
		m_destinationKind = destination_kind::pull;
	}

	void add_to_set(node_set s) {
		if (s)
			m_add_to_set.push_back(s);
	}

	void add_dependencies(node_set s) {
		if (s)
			m_add_relations.push_back(std::make_pair(s,bits::no_forward_depends));
	}

	void add_forwarding_dependencies(node_set s) {
		if (s)
			m_add_relations.push_back(std::make_pair(s,bits::depends));
	}

	void forward(const std::string & key, any_noncopyable value) {
		m_forwards.push_back({key, std::move(value)});
	}

private:
	void init_common(node & r) {
		if (m_set) r.set_memory_fraction(memory());

		for (size_t i = 0; i < m_hooks.size(); ++i) {
			m_hooks[i]->init_node(r);
		}

		auto nodeMap = r.get_node_map()->find_authority();

		for (auto &p : m_forwards) {
			nodeMap->forward_from_pipe_base(r.get_id(), p.first, std::move(p.second));
		}
	}
	
	double m_amount;
	bool m_set;
	destination_kind::type m_destinationKind;
	std::string m_name, m_phaseName;
	std::string m_breadcrumbs;
	priority_type m_namePriority, m_phaseNamePriority;
	std::vector<factory_init_hook *> m_hooks;
	std::vector<node_set> m_add_to_set;
	std::vector<std::pair<node_set, bits::node_relation> > m_add_relations;
	std::vector<std::pair<std::string, any_noncopyable> > m_forwards;
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_FACTORY_BASE_H__
