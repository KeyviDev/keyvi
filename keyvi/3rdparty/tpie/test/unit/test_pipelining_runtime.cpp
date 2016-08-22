// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2014 The TPIE development team
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

#include "common.h"
#include <tpie/pipelining/runtime.h>
#include <tpie/pipelining/runtime.cpp>
#include <tpie/pipelining/node.h>

using namespace tpie;
using namespace tpie::pipelining;
using namespace tpie::pipelining::bits;

class evac_node : public node {
public:
	// This override makes no difference for the evacuate unit test.
	// The pipelining runtime concludes that the phase should be evacuated
	// without consulting the can_evacuate of each node in the phase.
	virtual bool can_evacuate() override {
		return true;
	}
};

class no_evac_node : public node {
public:

	virtual bool can_evacuate() override {
		return false;
	}
};

bool evacuate_test() {
	const size_t N = 7;
	evac_node nodes[N];

	node_map::ptr nodeMap = nodes[0].get_node_map();
	for (size_t i = 1; i < N; ++i) nodes[i].get_node_map()->union_set(nodeMap);
	nodeMap = nodeMap->find_authority();

	// In our model, each node is its own phase.
	std::map<node *, size_t> phaseMap;
	for (size_t i = 0; i < N; ++i) phaseMap[&nodes[i]] = i;

	graph<size_t> phaseGraph;
	for (size_t i = 0; i < N; ++i) phaseGraph.add_node(i);

	std::vector<std::pair<size_t, size_t> > edges{{0,1}, {0,2}, {1,3}, {2,3}, {3,4}, {3,5}, {4,6}, {5,6}};
	
	for (auto e: edges) {
		phaseGraph.add_edge(e.first, e.second);
		log_info() << nodes[e.second].get_id() << " " << nodes[e.first].get_id() << std::endl;
		nodes[e.second].add_memory_share_dependency(nodes[e.first]);
	}
	
	// 0 -- 1 ---- 3 -- 4 ---- 6
	//  \         / \         /
	//   `---- 2 ´   `---- 5 ´
	//

	std::unordered_set<uint64_t> evacuateWhenDone;
	std::vector<std::vector<node *> > phases;

	{
		runtime rt(nodeMap);
		rt.get_phases(phaseMap, phaseGraph, evacuateWhenDone, phases);
	}
		
	std::unordered_map<size_t, size_t> nodePhases;
	for (size_t i=0; i < phases.size(); ++i)
		for (node * n: phases[i])
			nodePhases.emplace((evac_node*)n-nodes, i);
	
	bool bad = false;
	for (size_t i=0; i < N; ++i) {
		size_t evac = 0;
		for (auto e: edges) {
			if (e.first != i) continue;
			if (nodePhases[i]+1 == nodePhases[e.second]) continue;
			evac = 1;
		}
		if (evacuateWhenDone.count(nodes[i].get_id()) != evac)  {
			if (evac)
				log_error() << "Evac of node " << i << " should be evaced but is not"  << std::endl;
			else
				log_error() << "Evac of node " << i << " is evacuated but should not be"  << std::endl;
			bad = true;
		}
	}
	
	return !bad;
}

bool get_phase_graph_test() {
	const size_t N = 8;
	const size_t P = 4;
	evac_node nodes[N];
	node_map::id_t ids[N];
	for (size_t i = 0; i < N; ++i) ids[i] = nodes[i].get_id();

	node_map::ptr nodeMap = nodes[0].get_node_map();
	for (size_t i = 1; i < N; ++i) nodes[i].get_node_map()->union_set(nodeMap);
	nodeMap = nodeMap->find_authority();

#define PUSH(i, j) nodeMap->add_relation(ids[i], ids[j], pushes)
#define DEP(i, j) nodeMap->add_relation(ids[i], ids[j], depends)
	PUSH(0,1);
	PUSH(0,2);
	DEP(3,1);
	DEP(4,2);
	DEP(5,3);
	DEP(6,4);
	PUSH(5,7);
	PUSH(6,7);

	std::map<node *, size_t> phaseMap;
	graph<size_t> phaseGraph;
	{
		runtime rt(nodeMap);
		rt.get_phase_map(phaseMap);
		rt.get_phase_graph(phaseMap, phaseGraph);
	}
	if (phaseMap.size() != N) {
		log_error() << "phaseMap has wrong size" << std::endl;
		return false;
	}
	size_t n[N];
	for (size_t i = 0; i < N; ++i) n[i] = phaseMap[&nodes[i]];
	if (phaseMap.size() != N) {
		log_error() << "phaseMap has wrong entries" << std::endl;
		return false;
	}
	if (n[0] != 0 || n[1] != 0 || n[2] != 0) {
		log_error() << "phase 0 is wrong" << std::endl;
		return false;
	}
	if (n[5] != 3 || n[6] != 3 || n[7] != 3) {
		log_error() << "phase 3 is wrong" << std::endl;
		return false;
	}
	if ((n[3] != 1 && n[3] != 2) || (n[4] != 1 && n[4] != 2)) {
		log_error() << "node 3 or 4 is wrong" << std::endl;
		return false;
	}
	const bool expectEdge[P][P] = {
		{ false, true,  true,  false },
		{ false, false, false, true  },
		{ false, false, false, true  },
		{ false, false, false, false }
	};
	for (size_t i = 0; i < P; ++i) {
		for (size_t j = 0; j < P; ++j) {
			if (phaseGraph.has_edge(i, j) != expectEdge[i][j]) {
				log_error() << "Edge set is wrong at " << i << "," << j << std::endl;
				return false;
			}
		}
	}
	return true;
}

// See tpie::pipelining::bits::runtime::get_phases for description of edge colors
enum edge_color {
	BLACK,
	RED,
	GREEN,
};
const char * edge_color_names[] = {
	"black",
	"red",
	"green",
};

struct edge_t {
	edge_color color;
	size_t from;
	size_t to;
};

void evacuate_phase_graph_test(teststream & ts,
							   bool should_fail,
							   size_t expected_satisfied_reds,
							   const char * name,
							   const std::vector<edge_t> & edges) {
	ts << name << std::endl;

	std::vector<node *> nodeList;
	std::map<size_t, std::shared_ptr<node>> nodes;
	std::map<size_t, bool> can_evac_node;

	// Make sure red edges come from nodes that can be evacuated
	// and green edges come from nodes that can't
	// Black edge can come from both types
	for (const edge_t & e : edges) {
		if (e.color == BLACK) continue;

		bool can_evac = e.color == RED;

		if (can_evac_node.find(e.from) != can_evac_node.end()) {
			tp_assert(can_evac == can_evac_node[e.from],
					  "Red and green edge with from same node not possible: " + std::to_string(e.from));
		}

		can_evac_node[e.from] = can_evac;
	}

	for (const auto & p : can_evac_node) {
		node * n = p.second? static_cast<node *>(new evac_node()): static_cast<node *>(new no_evac_node());
		nodes[p.first] = std::shared_ptr<node>(n);
		nodeList.push_back(n);
	}

	// For all nodes with neither a red or green edge coming from it
	// we make evacuatable nodes.
	for (const edge_t & e : edges) {
		for (size_t i : {e.from, e.to}) {
			if (nodes.find(i) == nodes.end()) {
				node * n = new evac_node();
				nodes[i] = std::shared_ptr<node>(n);
				nodeList.push_back(n);
			}
		}
	}

	std::map<node *, size_t> revNodes;
	for (const auto & p : nodes) {
		revNodes.insert({p.second.get(), p.first});
	}

	size_t N = nodeList.size();

	node_map::ptr nodeMap = nodeList[0]->get_node_map();
	for (auto n : nodeList) n->get_node_map()->union_set(nodeMap);
	nodeMap = nodeMap->find_authority();

	// In our model, each node is its own phase.
	std::map<node *, size_t> phaseMap;
	for (const auto & p : nodes) phaseMap[p.second.get()] = p.first;

	graph<size_t> phaseGraph;
	for (size_t i = 0; i < N; ++i) phaseGraph.add_node(i);

	for (const edge_t & e : edges) {
		phaseGraph.add_edge(e.from, e.to);
		log_info() << e.from << " -(" << edge_color_names[e.color] << ")-> " << e.to << std::endl;
		if (e.color == BLACK) {
			nodes[e.to]->add_dependency(*nodes[e.from]);
		} else {
			nodes[e.to]->add_memory_share_dependency(*nodes[e.from]);
		}
	}

	std::unordered_set<uint64_t> evacuateWhenDone;
	std::vector<std::vector<node *>> phases;

	{
		runtime rt(nodeMap);
		try {
			rt.get_phases(phaseMap, phaseGraph, evacuateWhenDone, phases);
		} catch (exception & e) {
			if (!should_fail) {
				log_error() << e.what() << std::endl;
			}
			ts << result(should_fail);
			return;
		}
	}

	if (should_fail) {
		log_error() << "Constructed phase ordering successfully" << std::endl;
	}

	log_info() << "Phase order: ";
	std::vector<size_t> phaseOrder;
	for (const auto & phase : phases) {
		size_t i = revNodes[phase[0]];
		phaseOrder.push_back(i);
		log_info() << i << ", ";
	}
	log_info() << std::endl;

	log_info() << "Evacuated nodes: ";
	std::unordered_set<size_t> evacuatedNodes;
	for (auto id : evacuateWhenDone) {
		size_t i = revNodes[nodeMap->get(id)];
		evacuatedNodes.insert(i);
		log_info() << i << ", ";
	}
	log_info() << std::endl;

	if (should_fail) {
		ts << result(false);
		return;
	}

	bool bad = false;
	size_t satisfied_reds = 0;
	size_t reds = 0;
	for (const edge_t & e : edges) {
		auto from = std::find(phaseOrder.begin(), phaseOrder.end(), e.from);
		auto to   = std::find(phaseOrder.begin(), phaseOrder.end(), e.to);
		bool satisfied = to - from == 1;

		if (e.color == GREEN) {
			if (evacuatedNodes.count(e.from) != 0) {
				log_error() << "Evacuated a node with a green edge going out: " << e.from << std::endl;
				bad = true;
			}
			if (!satisfied) {
				log_error() << "Phases with green edge between not consecutive: "
							<< e.from << " -> " << e.to << std::endl;
				bad = true;
			}
		} else if (e.color == RED) {
			reds++;
			if (satisfied) satisfied_reds++;
		}
	}

	log_info() << "Satisfied " << satisfied_reds << " out of " << reds << " red edges" << std::endl;

	if (satisfied_reds != expected_satisfied_reds) {
		log_error() << "Satisfied " << satisfied_reds << " red edges, expected " << expected_satisfied_reds << std::endl;
		ts << result(false);
		return;
	}

	ts << result(!bad);
}

void evacuate_phase_graph_multi(teststream & ts) {
	evacuate_phase_graph_test(ts, true, 0, "Simple fail", {
		{BLACK, 0, 1},
		{BLACK, 1, 2},
		{GREEN, 0, 2},
	});
	evacuate_phase_graph_test(ts, false, 0, "Diamond working", {
		{GREEN, 0, 1},
		{BLACK, 1, 3},
		{BLACK, 0, 2},
		{GREEN, 2, 3},
	});
	evacuate_phase_graph_test(ts, true, 0, "Diamond failing", {
		{GREEN, 0, 1},
		{GREEN, 1, 3},
		{BLACK, 0, 2},
		{BLACK, 2, 3},
	});
	evacuate_phase_graph_test(ts, false, 0, "Green path", {
		{GREEN, 2, 3},
		{GREEN, 1, 2},
		{GREEN, 3, 4},
		{GREEN, 0, 1},
	});
	evacuate_phase_graph_test(ts, false, 0, "Green bridges", {
		{GREEN, 0, 1},
		{BLACK, 1, 2},
		{BLACK, 2, 4},
		{BLACK, 1, 3},
		{BLACK, 3, 4},
		{GREEN, 4, 5},
		{BLACK, 5, 6},
		{BLACK, 6, 8},
		{BLACK, 5, 7},
		{BLACK, 7, 8},
		{GREEN, 8, 9},
		{BLACK, 9, 10},
		{GREEN, 10, 11},
	});
	evacuate_phase_graph_test(ts, false, 2, "Red diamond", {
		{RED, 0, 1},
		{RED, 1, 3},
		{RED, 0, 2},
		{RED, 2, 3},
	});
	evacuate_phase_graph_test(ts, false, 2, "3/4 Red diamond 1", {
		{RED, 0, 1},
		{RED, 1, 3},
		{BLACK, 0, 2},
		{RED, 2, 3},
	});
	evacuate_phase_graph_test(ts, false, 2, "3/4 Red diamond 2", {
		{BLACK, 0, 1},
		{RED, 1, 3},
		{RED, 0, 2},
		{RED, 2, 3},
	});
	evacuate_phase_graph_test(ts, false, 1, "Contracted node w/ outgoing red & black", {
		{GREEN, 0, 1},
		{BLACK, 0, 2},
		{RED, 1, 2},
		{BLACK, 0, 3},
	});
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.test(evacuate_test, "evacuate")
	.test(get_phase_graph_test, "get_phase_graph")
	.multi_test(evacuate_phase_graph_multi, "evacuate_phase_graph")
	;
}
