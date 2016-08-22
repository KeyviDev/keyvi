// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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

#include <tpie/pipelining/pipeline.h>
#include <tpie/pipelining/subpipeline.h>
#include <tpie/pipelining/node.h>
#include <unordered_map>
#include <iostream>
#include <tpie/pipelining/runtime.h>

namespace {
	typedef tpie::pipelining::bits::node_map S;

	class name {
	public:
		inline name(S::ptr nodeMap, S::id_t id) : nodeMap(nodeMap), id(id) {}
		S::ptr nodeMap;
		S::id_t id;
	};

	inline std::ostream & operator<<(std::ostream & out, const name & n) {
		S::val_t p = n.nodeMap->get(n.id);
		std::string name = p->get_name();
		if (name.size())
			return out << name << " (" << n.id << ')';
		else
			return out << typeid(*p).name() << " (" << n.id << ')';
	}
} // default namespace

namespace tpie {

namespace pipelining {

namespace bits {

typedef std::unordered_map<const node *, size_t> nodes_t;

void pipeline_base_base::plot_impl(std::ostream & out, bool full) {
	typedef tpie::pipelining::bits::node_map::id_t id_t;

	node_map::ptr nodeMap = m_nodeMap->find_authority();
	const node_map::relmap_t & relations = nodeMap->get_relations();
	
	std::unordered_map<id_t, id_t> repr;
	if (!full) {
	 	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			id_t s = i->first;
			id_t t = i->second.first;
			if (i->second.second != pushes) std::swap(s,t);
			if (nodeMap->get(s)->get_plot_options() & node::PLOT_SIMPLIFIED_HIDE)
				repr[s] = t;
		}
	}
	
	out << "digraph {\n";
	for (node_map::mapit i = nodeMap->begin(); i != nodeMap->end(); ++i) {
		if (repr.count(i->first)) continue;
		if (!full && (nodeMap->get(i->first)->get_plot_options() & node::PLOT_BUFFERED))
			out << '"' << name(nodeMap, i->first) << "\" [shape=box];\n";
		
		if (!full && (nodeMap->get(i->first)->get_plot_options() & node::PLOT_PARALLEL))
			out << '"' << name(nodeMap, i->first) << "\" [shape=polygon];\n";
		else
			out << '"' << name(nodeMap, i->first) << "\";\n";
	}

	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		id_t s = i->first;
		id_t t = i->second.first;
		if (i->second.second != pushes) std::swap(s,t);
		if (repr.count(s)) continue;
		while (repr.count(t)) t=repr[t];
		switch (i->second.second) {
			case pushes:
				out << '"' << name(nodeMap, s) << "\" -> \"" << name(nodeMap, t) << "\";\n";
				break;
			case pulls:
				out << '"' << name(nodeMap, s) << "\" -> \"" << name(nodeMap, t) << "\" [arrowhead=none,arrowtail=normal,dir=both];\n";
				break;
			case depends:
				out << '"' << name(nodeMap, s) << "\" -> \"" << name(nodeMap, t) << "\" [arrowhead=none,arrowtail=normal,dir=both,style=dashed];\n";
				break;
			case no_forward_depends:
				out << '"' << name(nodeMap, s) << "\" -> \"" << name(nodeMap, t) << "\" [arrowhead=none,arrowtail=normal,dir=both,style=dotted];\n";
				break;
			case memory_share_depends:
				out << '"' << name(nodeMap, s) << "\" -> \"" << name(nodeMap, t) << "\" [arrowhead=none,arrowtail=normal,dir=both,style=tapered];\n";
				break;

		}
	}
	out << '}' << std::endl;
}

void pipeline_base::operator()(stream_size_type items, progress_indicator_base & pi,
							   const memory_size_type initialFiles,
							   const memory_size_type initialMemory,
							   const char * file, const char * function) {
	node_map::ptr map = m_nodeMap->find_authority();
	runtime rt(map);
	rt.go(items, pi, initialFiles, initialMemory, file, function);

	/*
	typedef std::vector<phase> phases_t;
	typedef phases_t::const_iterator it;

	graph_traits g(*map);
	const phases_t & phases = g.phases();
	if (initialMemory == 0) log_warning() << "No memory for pipelining" << std::endl;

	memory_size_type mem = initialMemory;
	mem -= graph_traits::memory_usage(phases.size());

	if (mem > initialMemory) { // overflow
		log_warning() << "Not enough memory for pipelining framework overhead" << std::endl;
		mem = 0;
	}

	log_debug() << "Assigning " << mem << " b memory to each pipelining phase." << std::endl;
	for (it i = phases.begin(); i != phases.end(); ++i) {
		i->assign_memory(mem);
#ifndef TPIE_NDEBUG
		i->print_memory(log_debug());
#endif // TPIE_NDEBUG
	}
	g.go_all(items, pi);
	*/
}

void pipeline_base_base::forward_any(std::string key, any_noncopyable value) {
	get_node_map()->find_authority()->forward(key, std::move(value));
}

bool pipeline_base_base::can_fetch(std::string key) {
	node_map::ptr map = m_nodeMap->find_authority();
	runtime rt(map);
	std::vector<node *> sinks;
	rt.get_item_sinks(sinks);
	for (size_t j = 0; j < sinks.size(); ++j) {
		if (sinks[j]->can_fetch(key)) return true;
	}
	return false;
}

any_noncopyable & pipeline_base_base::fetch_any(std::string key) {
	node_map::ptr map = m_nodeMap->find_authority();
	runtime rt(map);
	std::vector<node *> sinks;
	rt.get_item_sinks(sinks);
	for (size_t j = 0; j < sinks.size(); ++j) {
		if (sinks[j]->can_fetch(key)) return sinks[j]->fetch_any(key);
	}

	std::stringstream ss;
	ss << "Tried to fetch nonexistent key '" << key << '\'';
	throw invalid_argument_exception(ss.str());
}

void pipeline_base::order_before(pipeline_base & other) {
	if (get_node_map()->find_authority()
		== other.get_node_map()->find_authority()) {

		tpie::log_debug()
			<< "Ignoring pipeline ordering hint since node maps are already shared"
			<< std::endl;
		return;
	}
	runtime rt1(get_node_map()->find_authority());
	runtime rt2(other.get_node_map()->find_authority());

	std::vector<node *> mySinks;
	std::vector<node *> otherSources;

	rt1.get_item_sinks(mySinks);
	rt2.get_item_sources(otherSources);

	if (mySinks.size() == 0) {
		throw tpie::exception("pipeline::order_before: mySinks is empty");
	}
	if (otherSources.size() == 0) {
		throw tpie::exception("pipeline::order_before: otherSources is empty");
	}

	for (size_t i = 0; i < otherSources.size(); ++i) {
		for (size_t j = 0; j < mySinks.size(); ++j) {
			otherSources[i]->add_dependency(*mySinks[j]);
		}
	}
}

void pipeline_base_base::output_memory(std::ostream & o) const {
	bits::node_map::ptr nodeMap = get_node_map()->find_authority();
	for (bits::node_map::mapit i = nodeMap->begin(); i != nodeMap->end(); ++i) {
		bits::node_map::val_t p = nodeMap->get(i->first);
		o << p->get_name() << ": min=" << p->get_minimum_memory() << "; max=" << p->get_available_memory() << "; prio=" << p->get_memory_fraction() << ";" << std::endl;

	}
}

void subpipeline_base::begin(stream_size_type items, progress_indicator_base & pi,
							 memory_size_type filesAvailable, memory_size_type mem,
							 const char * file, const char * function) {
	rt.reset(new runtime(m_nodeMap->find_authority()));
	gc = rt->go_init(items, pi, filesAvailable, mem, file, function);
	rt->go_until(gc.get(), frontNode);
}
	
void subpipeline_base::end() {
	rt->go_until(gc.get(), nullptr);
	gc.reset();
	rt.reset();
}

	
} // namespace bits

pipeline * pipeline::m_current = NULL;
	

} // namespace pipelining

} // namespace tpie
