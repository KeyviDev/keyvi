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

	phaseGraph.add_edge(0, 1);
	phaseGraph.add_edge(0, 2);
	phaseGraph.add_edge(1, 3);
	phaseGraph.add_edge(2, 3);
	phaseGraph.add_edge(3, 4);
	phaseGraph.add_edge(3, 5);
	phaseGraph.add_edge(4, 6);
	phaseGraph.add_edge(5, 6);

	// 0 -- 1 ---- 3 -- 4 ---- 6
	//  \         / \         /
	//   `---- 2 ´   `---- 5 ´
	//
	// Since the result of 1 and 4 are not needed in 2 and 5 resp.,
	// (that is, there is no edge 1-2 or 4-5,)
	// 1 and 4 should be evacuated when they are done.

	std::vector<bool> expect(7);
	expect[1] = expect[4] = true;

	std::vector<bool> evacuateWhenDone;
	std::vector<std::vector<node *> > phases;

	{
		runtime rt(nodeMap);
		rt.get_phases(phaseMap, phaseGraph, evacuateWhenDone, phases);
	}

	bool bad = false;
	for (size_t i = 0; i < N; ++i) {
		if (evacuateWhenDone[i] == expect[i]) {
			log_debug() << "Node " << i << ": "
				<< (expect[i] ? "should evacuate" : "don't evacuate")
				<< std::endl;
		} else {
			log_error() << "Node " << i << ": Expected "
				<< (expect[i] ? "should evacuate" : "don't evacuate")
				<< ", got "
				<< (evacuateWhenDone[i] ? "should evacuate" : "don't evacuate")
				<< std::endl;
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

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.test(evacuate_test, "evacuate")
	.test(get_phase_graph_test, "get_phase_graph")
	;
}
