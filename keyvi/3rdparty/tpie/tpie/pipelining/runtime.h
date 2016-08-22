// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013, 2014, The TPIE development team
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

#ifndef TPIE_PIPELINING_RUNTIME_H
#define TPIE_PIPELINING_RUNTIME_H

#include <tpie/fractional_progress.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/pipelining/node.h>
#include <set>
#include <unordered_set>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename T>
class graph;
class file_runtime;
class memory_runtime;
class datastructure_runtime;

struct gocontext;
struct gocontextdel {
	void operator()(void *);
};
typedef std::unique_ptr<gocontext, gocontextdel> gocontext_ptr;

	
///////////////////////////////////////////////////////////////////////////////
/// \brief  Execute the pipeline contained in a node_map.
///////////////////////////////////////////////////////////////////////////////
class runtime {
	node_map & m_nodeMap;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Construct a runtime object.
	///
	/// Does nothing other than copy the smart pointer given.
	///////////////////////////////////////////////////////////////////////////
	runtime(node_map::ptr nodeMap);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Number of nodes contained in node map.
	///
	/// Returns m_nodeMap.size().
	///////////////////////////////////////////////////////////////////////////
	size_t get_node_count();

	gocontext_ptr go_init(stream_size_type items,
						 progress_indicator_base & progress,
						 memory_size_type files,
						 memory_size_type memory,
						 const char * file, const char * function);
	
	void go_until(gocontext * gc, node * node=nullptr);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Execute the pipeline.
	///
	/// This is the main entry point. The method go() sets up all nodes for
	/// execution and executes all initiators in turn:
	///
	/// Call node::prepare in item source to item sink order for each phase.
	///
	/// Assign memory according to memory constraints and memory priorities.
	///
	/// For each phase, call propagate, begin, go and end on nodes as
	/// appropriate. We call propagate in item source to item sink order;
	/// we call begin in leaf to root actor order; we call end in root to leaf
	/// actor order.
	///////////////////////////////////////////////////////////////////////////
	void go(stream_size_type items,
			progress_indicator_base & progress,
			memory_size_type files,
			memory_size_type memory,
			const char * file, const char * function);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get all sources of the item flow graph.
	///
	/// An item source node has no ingoing edges in the item flow graph
	/// of its phase, and its phase does not depend on any phases.
	///
	/// This is the set of nodes used when forwarding out-of-band data into
	/// the pipeline.
	///////////////////////////////////////////////////////////////////////////
	void get_item_sources(std::vector<node *> & itemSources);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get all sinks of the item flow graph.
	///
	/// An item sink node has no outgoing edges in the item flow graph
	/// of its phase, and no phase depends on its phase.
	///
	/// This is the set of nodes used when fetching out-of-band data from
	/// the pipeline.
	///////////////////////////////////////////////////////////////////////////
	void get_item_sinks(std::vector<node *> & itemSinks);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Partition nodes into phases (using union-find).
	///////////////////////////////////////////////////////////////////////////
	void get_phase_map(std::map<node *, size_t> & phaseMap);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set up phase graph so we can find a topological order.
	///////////////////////////////////////////////////////////////////////////
	void get_phase_graph(const std::map<node *, size_t> & phaseMap,
						 graph<size_t> & phaseGraph);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the inverse of a permutation.
	///
	/// A permutation of N elements is given as a std::vector of size N,
	/// in which each entry maps to a distinct integer in [0, N).
	/// The inverse permutation of f is g if and only if
	/// f[g[i]] == g[f[i]] == i  for all i in [0,N).
	///////////////////////////////////////////////////////////////////////////
	static std::vector<size_t> inverse_permutation(const std::vector<size_t> & f);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute topological phase order.
	///
	/// The vector phases[i] will contain the nodes in the ith phase to run.
	/// For each node in phase[i], if the node has a memory share dependency to
	/// any node not in phases[i-1], the node is contained in evacuateWhenDone.
	///////////////////////////////////////////////////////////////////////////
	void get_phases(const std::map<node *, size_t> & phaseMap,
					const graph<size_t> & phaseGraph,
					std::unordered_set<node_map::id_t> & evacuateWhenDone,
					std::vector<std::vector<node *> > & phases);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	void get_item_flow_graphs(std::vector<std::vector<node *> > & phases,
							  std::vector<graph<node *> > & itemFlow);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	void get_actor_graphs(std::vector<std::vector<node *> > & phases,
						  std::vector<graph<node *> > & actors);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by get_{actor,item_flow}_graphs().
	///////////////////////////////////////////////////////////////////////////
	void get_graph(std::vector<node *> & phase, graph<node *> & result,
				   bool itemFlow);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if the node is a phase initiator.
	///
	/// A node is a phase initiator if it has no ingoing edges in the actor
	/// graph, or in other words if no node pushes to it or pulls from it.
	///////////////////////////////////////////////////////////////////////////
	bool is_initiator(node * n);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Equivalent to any_of(begin(phase), end(phase), is_initiator).
	///////////////////////////////////////////////////////////////////////////
	bool has_initiator(const std::vector<node *> & phase);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Ensure that all phases have at least one initiator.
	///
	/// If a phase has no initiators, throw no_initiator_node().
	///////////////////////////////////////////////////////////////////////////
	void ensure_initiators(const std::vector<std::vector<node *> > & phases);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call prepare on all nodes in item source to sink order.
	///////////////////////////////////////////////////////////////////////////
	void prepare_all(const std::vector<graph<node *> > & itemFlow);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call evacuate on all nodes in evacuateWhenDone for which
	/// can_evacuate() is true.
	///////////////////////////////////////////////////////////////////////////
	void evacuate_all(const std::vector<node *> & phase,
					  const std::unordered_set<node_map::id_t> & evacuateWhenDone);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call propagate on all nodes in item source to sink order.
	///////////////////////////////////////////////////////////////////////////
	void propagate_all(const graph<node *> & itemFlow);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call set_progress_indicator on all nodes in the phase.
	///////////////////////////////////////////////////////////////////////////
	void set_progress_indicators(const std::vector<node *> & phase,
								 progress_indicator_base & pi);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call go() on all initiators after setting the given progress
	/// indicator.
	///////////////////////////////////////////////////////////////////////////
	void go_initiators(const std::vector<node *> & phase);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	static void set_resource_being_assigned(const std::vector<node *> & nodes,
											resource_type type);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	static void assign_files(const std::vector<std::vector<node *> > & phases,
							  memory_size_type files);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	static void reassign_files(const std::vector<std::vector<node *> > & phases,
								memory_size_type phase,
								memory_size_type files);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by assign_memory().
	///////////////////////////////////////////////////////////////////////////
	static double get_files_factor(memory_size_type files,
								   const file_runtime & frt);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	static void assign_memory(const std::vector<std::vector<node *> > & phases,
							  memory_size_type memory, datastructure_runtime & drt);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	static void reassign_memory(const std::vector<std::vector<node *> > & phases,
								memory_size_type phase,
								memory_size_type memory, const datastructure_runtime & drt);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by assign_memory().
	///////////////////////////////////////////////////////////////////////////
	static double get_memory_factor(memory_size_type memory,
									memory_size_type phase,
									const memory_runtime & mrt,
									const datastructure_runtime & drt,
									bool datastructures_locked);
private:
	void get_flush_priorities(const std::map<node *, size_t> & phaseMap, std::vector<size_t> & flushPriorities);
	void get_ordered_graph(const std::vector<size_t> & flushPriorities, const graph<size_t> & phaseGraph, graph<size_t> & orderedPhaseGraph);

};

}

}

}

#endif // TPIE_PIPELINING_RUNTIME_H
