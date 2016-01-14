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

#include <tpie/fractional_progress.h>
#include <tpie/progress_indicator_null.h>
#include <tpie/disjoint_sets.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/runtime.h>
#include <boost/functional/hash.hpp>

namespace tpie {

namespace pipelining {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Directed graph with nodes of type T.
///
/// The node set is implied by the endpoints of the edges.
///
/// Computes the topological order using depth first search.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class graph {
public:
	void add_node(T v) {
		m_nodes.insert(v);
		m_edgeLists[v]; // ensure v has an edge list
	}

	void add_edge(T u, T v) {
		add_node(u);
		add_node(v);
		m_edgeLists[u].push_back(v);
	}

	const std::vector<T> & get_edge_list(const T & i) const {
		return m_edgeLists.find(i)->second;
	}

	size_t size() const {
		return m_nodes.size();
	}

	bool has_edge(T u, T v) const {
		const std::vector<T> & edgeList = m_edgeLists.find(u)->second;
		return std::find(edgeList.begin(), edgeList.end(), v) != edgeList.end();
	}

	void topological_order(std::vector<T> & result) const {
		const size_t N = m_nodes.size();
		depth_first_search dfs(m_edgeLists);
		std::vector<std::pair<size_t, T> > nodes(N);
		for (typename std::map<T, std::vector<T> >::const_iterator i = m_edgeLists.begin();
			 i != m_edgeLists.end(); ++i)
			nodes.push_back(std::make_pair(dfs.visit(i->first), i->first));
		std::sort(nodes.begin(), nodes.end(), std::greater<std::pair<size_t, T> >());
		result.resize(N);
		for (size_t i = 0; i < N; ++i) result[i] = nodes[i].second;
	}

	void rootfirst_topological_order(std::vector<T> & result) const { // a topological order where the root of trees are always visited first in the DFS
		std::vector<T> topologicalOrder;
		topological_order(topologicalOrder);

		const size_t N = m_nodes.size();
		depth_first_search dfs(m_edgeLists);
		std::vector<std::pair<size_t, T> > nodes(N);
		for (typename std::vector<T>::const_iterator i = topologicalOrder.begin(); i != topologicalOrder.end(); ++i)
			nodes.push_back(std::make_pair(dfs.visit(*i), *i));
		std::sort(nodes.begin(), nodes.end(), std::greater<std::pair<size_t, T> >());
		result.resize(N);
		for (size_t i = 0; i < N; ++i) result[i] = nodes[i].second;
	}

private:
	std::set<T> m_nodes;
	std::map<T, std::vector<T> > m_edgeLists;

	class depth_first_search {
	public:
		depth_first_search(const std::map<T, std::vector<T> > & edgeLists)
			: m_time(0)
			, m_edgeLists(edgeLists)
		{
		}

		size_t visit(T u) {
			if (m_finishTime.count(u)) return m_finishTime[u];
			m_finishTime[u] = 0;
			++m_time;
			const std::vector<T> & edgeList = get_edge_list(u);
			for (size_t i = 0; i < edgeList.size(); ++i) visit(edgeList[i]);
			return m_finishTime[u] = m_time++;
		}

	private:
		const std::vector<T> & get_edge_list(T u) {
			typename std::map<T, std::vector<T> >::const_iterator i = m_edgeLists.find(u);
			if (i == m_edgeLists.end())
				throw tpie::exception("get_edge_list: no such node");
			return i->second;
		}

		size_t m_time;
		const std::map<T, std::vector<T> > & m_edgeLists;
		std::map<T, size_t> m_finishTime;
	};
};

///////////////////////////////////////////////////////////////////////////////
/// Helper methods for memory assignment.
/// The memory assignment algorithm is in runtime::get_memory_factor.
///////////////////////////////////////////////////////////////////////////////
class memory_runtime {
public:
	memory_runtime(const std::vector<node *> & nodes);

	// Node accessors
	memory_size_type minimum_memory(size_t i) const;
	memory_size_type maximum_memory(size_t i) const;
	double fraction(size_t i) const;

	// Node accessor aggregates
	memory_size_type sum_minimum_memory() const;
	memory_size_type sum_maximum_memory() const;
	double sum_fraction() const;

	// Node mutator
	void set_memory(size_t i, memory_size_type mem);

	void assign_memory(double factor);

	// Special case of assign_memory when factor is zero.
	void assign_minimum_memory();

	memory_size_type sum_assigned_memory(double factor) const;

	memory_size_type get_assigned_memory(size_t i, double factor) const;

	static memory_size_type clamp(memory_size_type lo, memory_size_type hi,
								  double v);

	void print_memory(double c, std::ostream & os);

private:
	const std::vector<node *> & m_nodes;
	memory_size_type m_minimumMemory;
	memory_size_type m_maximumMemory;
	double m_fraction;
};

///////////////////////////////////////////////////////////////////////////////
/// Helper methods for memory assignment.
/// The memory assignment algorithm is in runtime::get_memory_factor.
///////////////////////////////////////////////////////////////////////////////
class datastructure_runtime {
public:
	datastructure_runtime(const std::vector<std::vector<node *> > & phases, node_map & nodeMap);

	memory_size_type sum_minimum_memory(size_t phase) const; // sum the minimum memory for datastructures used in the phase
	double sum_fraction(size_t i) const; // sum the fractions for datastructures used in phase i
	memory_size_type sum_assigned_memory(double factor, size_t phase) const; // sum the assigned memory for datastructures used in the phase
	void minimize_factor(double factor, size_t phase); // the factor for the datastructure in the phase is set to be no higher than the given factor
	memory_size_type sum_assigned_memory(size_t phase) const; // sum the assigned memory for datastructures used in the phase using the factors given to the minimize_factor method
	void assign_memory();

	//void print_memory(double c, std::ostream & os);
private:
	static memory_size_type clamp(memory_size_type lo, memory_size_type hi, double v);

	struct datastructure_info_t {
		memory_size_type min;
		memory_size_type max;
		double priority;
		memory_size_type right_most_phase;
		memory_size_type left_most_phase;
		double factor;

		datastructure_info_t()
		: min(0)
		, max(std::numeric_limits<memory_size_type>::max())
		, priority(1)
		, right_most_phase(0)
		, left_most_phase(std::numeric_limits<memory_size_type>::max())
		, factor(std::numeric_limits<double>::max())
		{}
	};

	std::map<std::string, datastructure_info_t> m_datastructures;
	node_map & m_nodeMap;
};


	
///////////////////////////////////////////////////////////////////////////////
/// \brief  Helper class for RAII-style progress indicators.
///
/// init calls fractional_progress::init,
/// and the destructor calls fractional_progress::done.
///
/// Instantiate phase_progress_indicators
/// to call init and done on subindicators.
///////////////////////////////////////////////////////////////////////////////
class progress_indicators {
public:
	progress_indicators(): fp(nullptr) {}

	~progress_indicators() {
		if (fp) fp->done();
		for (size_t i = 0; i < m_progressIndicators.size(); ++i) {
			delete m_progressIndicators[i];
		}
		m_progressIndicators.resize(0);
		delete fp;
		fp = nullptr;
	}

	void init(stream_size_type n,
			  progress_indicator_base & pi,
			  const std::vector<std::vector<node *> > & phases,
			  const char * file,
			  const char * function) {
		const size_t N = phases.size();
		m_progressIndicators.resize(N);
		fp = nullptr;
		if (!file|| !function) {
			for (size_t i = 0; i < N; ++i) 
				m_progressIndicators[i] = new progress_indicator_null();
			return;
		}
		
		fp = new fractional_progress(&pi);
		std::size_t uuid = 0;
		for (size_t i = 0; i < N; ++i) {
			for (node * n: phases[i])
				boost::hash_combine(uuid, n->get_name());
			std::string name = get_phase_name(phases[i]);
			char id[128];
			// since an int like i cannot be 17 chars long, this cannot overflow
			sprintf(id, "p%03d:%.100s:%08llX", (int)i, name.c_str(), (unsigned long long)uuid);
			m_progressIndicators[i] = new fractional_subindicator(
				*fp, id, file, function, n, name.c_str());
		}
		fp->init();
	}

private:
	std::string get_phase_name(const std::vector<node *> & phase) {
		priority_type highest = std::numeric_limits<priority_type>::min();
		size_t highest_node = 0;
		for (size_t i = 0; i < phase.size(); ++i) {
			if (phase[i]->get_phase_name_priority() > highest) {
				highest_node = i;
				highest = phase[i]->get_phase_name_priority();
			}
		}
		std::string n = phase[highest_node]->get_phase_name();
		if (!n.empty()) return n;

		highest_node = 0;
		for (size_t i = 0; i < phase.size(); ++i) {
			if (phase[i]->get_name_priority() > highest) {
				highest_node = i;
				highest = phase[i]->get_name_priority();
			}
		}
		return phase[highest_node]->get_name();
	}

	friend class phase_progress_indicator;

	fractional_progress * fp;
	std::vector<progress_indicator_base *> m_progressIndicators;
};

///////////////////////////////////////////////////////////////////////////////
/// RAII-style progress indicator for a single phase.
/// Constructor computes number of steps and calls init; destructor calls done.
///////////////////////////////////////////////////////////////////////////////
class phase_progress_indicator {
public:
	phase_progress_indicator(progress_indicators & pi, size_t phaseNumber,
							 const std::vector<node *> & nodes)
		: m_pi(pi.m_progressIndicators[phaseNumber])
	{
		stream_size_type steps = 0;
		for (size_t j = 0; j < nodes.size(); ++j) {
			steps += nodes[j]->get_steps();
		}
		m_pi->init(steps);
	}

	~phase_progress_indicator() {
		m_pi->done();
	}

	progress_indicator_base & get() {
		return *m_pi;
	}

private:
	progress_indicator_base * m_pi;
};

///////////////////////////////////////////////////////////////////////////////
/// begin/end handling on nodes.
///////////////////////////////////////////////////////////////////////////////
class begin_end {
public:
	begin_end(graph<node *> & actorGraph) {
		actorGraph.topological_order(m_topologicalOrder);
	}

	void begin() {
		for (size_t i = m_topologicalOrder.size(); i--;) {
			m_topologicalOrder[i]->set_state(node::STATE_IN_BEGIN);
			m_topologicalOrder[i]->begin();
			m_topologicalOrder[i]->set_state(node::STATE_AFTER_BEGIN);
		}
	}

	void end() {
		for (size_t i = 0; i < m_topologicalOrder.size(); ++i) {
			m_topologicalOrder[i]->set_state(node::STATE_IN_END);
			m_topologicalOrder[i]->end();
			m_topologicalOrder[i]->set_state(node::STATE_AFTER_END);
		}
	}

private:
	std::vector<node *> m_topologicalOrder;
};

memory_runtime::memory_runtime(const std::vector<node *> & nodes)
	: m_nodes(nodes)
	, m_minimumMemory(0)
	, m_maximumMemory(0)
	, m_fraction(0.0)
{
	const size_t N = m_nodes.size();
	for (size_t i = 0; i < N; ++i) {
		m_minimumMemory += minimum_memory(i);
		m_maximumMemory += maximum_memory(i);
		m_fraction += fraction(i);
	}
}

// Node accessors
memory_size_type memory_runtime::minimum_memory(size_t i) const {
	return m_nodes[i]->get_minimum_memory();
}

memory_size_type memory_runtime::maximum_memory(size_t i) const {
	return m_nodes[i]->get_maximum_memory();
}

double memory_runtime::fraction(size_t i) const {
	return m_nodes[i]->get_memory_fraction();
}

// Node accessor aggregates
memory_size_type memory_runtime::sum_minimum_memory() const {
	return m_minimumMemory;
}

memory_size_type memory_runtime::sum_maximum_memory() const {
	return m_maximumMemory;
}

double memory_runtime::sum_fraction() const {
	return m_fraction;
}


// Node mutator
void memory_runtime::set_memory(size_t i, memory_size_type mem) {
	m_nodes[i]->set_available_memory(mem);
}

void memory_runtime::assign_memory(double factor) {
	for (size_t i = 0; i < m_nodes.size(); ++i)
		set_memory(i, get_assigned_memory(i, factor));
}

// Special case of assign_memory when factor is zero.
void memory_runtime::assign_minimum_memory() {
	for (size_t i = 0; i < m_nodes.size(); ++i)
		set_memory(i, minimum_memory(i));
}

memory_size_type memory_runtime::sum_assigned_memory(double factor) const {
	memory_size_type memoryAssigned = 0;
	for (size_t i = 0; i < m_nodes.size(); ++i)
		memoryAssigned += get_assigned_memory(i, factor);
	return memoryAssigned;
}

memory_size_type memory_runtime::get_assigned_memory(size_t i,
													 double factor) const {
	return clamp(minimum_memory(i), maximum_memory(i),
				 factor * fraction(i));
}

/*static*/
memory_size_type memory_runtime::clamp(memory_size_type lo,
									   memory_size_type hi,
									   double v)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return static_cast<memory_size_type>(v);
}

void memory_runtime::print_memory(double c, std::ostream & os) {
	size_t cw = 12;
	size_t prec_frac = 2;
	std::string sep(2, ' ');

	os	<< "\nPipelining phase memory assigned\n"
		<< std::setw(cw) << "Minimum"
		<< std::setw(cw) << "Maximum"
		<< std::setw(cw) << "Fraction"
		<< std::setw(cw) << "Assigned"
		<< sep << "Name\n";

	for (size_t i = 0; i < m_nodes.size(); ++i) {
		std::string frac;
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision(prec_frac)
				<< fraction(i);
			frac = ss.str();
		}

		stream_size_type lo = minimum_memory(i);
		stream_size_type hi = maximum_memory(i);
		stream_size_type assigned = get_assigned_memory(i, c);

		os	<< std::setw(cw) << lo;
		if (hi == std::numeric_limits<stream_size_type>::max()) {
			os << std::setw(cw) << "inf";
		} else {
			os << std::setw(cw) << hi;
		}
		os	<< std::setw(cw) << frac
			<< std::setw(cw) << assigned
			<< sep
			<< m_nodes[i]->get_name().substr(0, 50) << '\n';
	}
	os << std::endl;
}

datastructure_runtime::datastructure_runtime(const std::vector<std::vector<node *> > & phases, node_map & nodeMap)
	: m_nodeMap(nodeMap)
{
	for (size_t phase = 0; phase < phases.size(); ++phase) {
		for (std::vector<node *>::const_iterator node = phases[phase].begin(); node != phases[phase].end(); ++node) {
			const node::datastructuremap_t & node_datastructures = (*node)->get_datastructures();
			for (node::datastructuremap_t::const_iterator datastructure = node_datastructures.begin(); datastructure != node_datastructures.end(); ++datastructure) {
				const std::string & name = datastructure->first;
				const node::datastructure_info_t & info = datastructure->second;

				std::map<std::string, datastructure_info_t>::iterator i = m_datastructures.find(name);
				if (i == m_datastructures.end()) {
					datastructure_info_t agg_info;
					agg_info.min = info.min;
					agg_info.max = info.max;
					agg_info.priority = info.priority;
					agg_info.left_most_phase = phase;
					agg_info.right_most_phase = phase;
					m_datastructures[name] = agg_info;
					continue;
				}

				datastructure_info_t & agg_info = i->second;
				agg_info.min = std::max(agg_info.min, info.min);
				agg_info.max = std::min(agg_info.max, info.max);
				agg_info.priority = std::min(agg_info.priority, info.priority);
				agg_info.right_most_phase = phase;
			}
		}
	}
}

memory_size_type datastructure_runtime::sum_minimum_memory(size_t phase) const {
	memory_size_type r = 0;
	for(std::map<std::string, datastructure_info_t>::const_iterator i = m_datastructures.begin(); i != m_datastructures.end(); ++i) {
		const datastructure_info_t & agg_info = i->second;
		if(agg_info.left_most_phase <= phase && phase <= agg_info.right_most_phase)
			r += agg_info.min;
	}

	return r;
}

double datastructure_runtime::sum_fraction(size_t phase) const {
	double r = 0.0;
	for(std::map<std::string, datastructure_info_t>::const_iterator i = m_datastructures.begin(); i != m_datastructures.end(); ++i) {
		const datastructure_info_t & agg_info = i->second;
		if(agg_info.left_most_phase <= phase && phase <= agg_info.right_most_phase)
			r += agg_info.priority;
	}

	return r;
}

memory_size_type datastructure_runtime::sum_assigned_memory(double factor, size_t phase) const {
	memory_size_type r = 0;
	for(std::map<std::string, datastructure_info_t>::const_iterator i = m_datastructures.begin(); i != m_datastructures.end(); ++i) {
		const datastructure_info_t & agg_info = i->second;
		if(agg_info.left_most_phase <= phase && phase <= agg_info.right_most_phase)
			r += clamp(agg_info.min, agg_info.max, agg_info.priority * factor);
	}

	return r;
}

void datastructure_runtime::minimize_factor(double factor, size_t phase) {
	for(std::map<std::string, datastructure_info_t>::iterator i = m_datastructures.begin(); i != m_datastructures.end(); ++i) {
		datastructure_info_t & agg_info = i->second;
		if(agg_info.left_most_phase <= phase && phase <= agg_info.right_most_phase)
			agg_info.factor = std::min(agg_info.factor, factor);
	}
}

memory_size_type datastructure_runtime::sum_assigned_memory(size_t phase) const {
	memory_size_type r = 0;
	for(std::map<std::string, datastructure_info_t>::const_iterator i = m_datastructures.begin(); i != m_datastructures.end(); ++i) {
		const datastructure_info_t & agg_info = i->second;
		if(agg_info.left_most_phase <= phase && phase <= agg_info.right_most_phase)
			r += clamp(agg_info.min, agg_info.max, agg_info.priority * agg_info.factor);
	}

	return r;
}

memory_size_type datastructure_runtime::clamp(memory_size_type lo, memory_size_type hi, double v) {
	if(v < lo) return lo;
	if(v > hi) return hi;
	return static_cast<memory_size_type>(v);
}

void datastructure_runtime::assign_memory() {
	for(std::map<std::string, datastructure_info_t>::iterator i = m_datastructures.begin(); i != m_datastructures.end(); ++i) {
		memory_size_type mem = clamp(i->second.min, i->second.max, i->second.factor * i->second.priority);
		m_nodeMap.get_datastructures().insert(std::make_pair(i->first, std::make_pair(mem, boost::any())));
	}
}

runtime::runtime(node_map::ptr nodeMap)
	: m_nodeMap(*nodeMap)
{
}

size_t runtime::get_node_count() {
	return m_nodeMap.size();
}

size_t calculate_recursive_flush_priority(size_t phase, std::vector<std::pair<size_t, bool> > & mem, const std::vector<size_t> & flushPriorities, const graph<size_t> & phaseGraph) {
	if(mem[phase].second)
		return mem[phase].first;

	size_t priority = flushPriorities[phase];

	const std::vector<size_t> & edges = phaseGraph.get_edge_list(phase);
	for(std::vector<size_t>::const_iterator i = edges.begin(); i != edges.end(); ++i) {
		priority = std::max(priority, calculate_recursive_flush_priority(*i, mem, flushPriorities, phaseGraph));
	}

	mem[phase] = std::make_pair(priority, true);
	return priority;
}

class flush_priority_greater_comp {
public:
	flush_priority_greater_comp(const std::vector<std::pair<size_t, bool> > & priorities)
		: m_priorities(priorities) 
	{}

	bool operator()(size_t a, size_t b) const {
		return m_priorities[a].first > m_priorities[b].first;
	}

private:
	const std::vector<std::pair<size_t, bool> > & m_priorities;
};

void runtime::get_ordered_graph(const std::vector<size_t> & flushPriorities, const graph<size_t> & phaseGraph, graph<size_t> & orderedPhaseGraph) {
	std::vector<std::pair<size_t, bool> > recursiveFlushPriorites;
	recursiveFlushPriorites.resize(phaseGraph.size());
	std::fill(recursiveFlushPriorites.begin(), recursiveFlushPriorites.end(), std::make_pair(0, false));

	for(size_t i = 0; i != recursiveFlushPriorites.size(); ++i) {
		calculate_recursive_flush_priority(i, recursiveFlushPriorites, flushPriorities, phaseGraph);
	}

	for(size_t i = 0; i < phaseGraph.size(); ++i)
		orderedPhaseGraph.add_node(i);

	// Build ordered phase graph
	for(size_t i = 0; i < phaseGraph.size(); ++i) {
		std::vector<size_t> edges = phaseGraph.get_edge_list(i);
		std::sort(edges.begin(), edges.end(), flush_priority_greater_comp(recursiveFlushPriorites));
		for(std::vector<size_t>::iterator j = edges.begin(); j != edges.end(); ++j) {
			orderedPhaseGraph.add_edge(i, *j);
		}
	}
}

void runtime::get_flush_priorities(const std::map<node *, size_t> & phaseMap, std::vector<size_t> & flushPriorities) {
	for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
		node * a = i->second;
		size_t phase = phaseMap.find(a)->second;

		while(flushPriorities.size() <= phase)
			flushPriorities.push_back(0);
		flushPriorities[phase] = std::max(flushPriorities[phase], a->get_flush_priority());
	}
}

void runtime::go(stream_size_type items,
				 progress_indicator_base & progress,
				 memory_size_type memory,
				 const char * file,
				 const char * function) {
	if (get_node_count() == 0)
		throw tpie::exception("no nodes in pipelining graph");

	// Partition nodes into phases (using union-find)
	std::map<node *, size_t> phaseMap;
	get_phase_map(phaseMap);
	if (phaseMap.size() != get_node_count())
		throw tpie::exception("get_phase_map did not return "
							  "correct number of nodes");

	// Calculate phase flush priorities
	std::vector<size_t> flushPriorities;
	get_flush_priorities(phaseMap, flushPriorities);

	// Build phase graph
	graph<size_t> phaseGraph;
	get_phase_graph(phaseMap, phaseGraph);

	// Calculate recursive flush priorities
	graph<size_t> orderedPhaseGraph;
	get_ordered_graph(flushPriorities, phaseGraph, orderedPhaseGraph);

	// Build phases vector

	std::vector<std::vector<node *> > phases;
	std::vector<bool> evacuateWhenDone;
	get_phases(phaseMap, orderedPhaseGraph, evacuateWhenDone, phases);

	// Build item flow graph and actor graph for each phase
	std::vector<graph<node *> > itemFlow;
	get_item_flow_graphs(phases, itemFlow);
	std::vector<graph<node *> > actor;
	get_actor_graphs(phases, actor);

	// Check that each phase has at least one initiator
	ensure_initiators(phases);

	// Toposort item flow graph for each phase
	// and call node::prepare in item source to item sink order
	prepare_all(itemFlow);

	// build the datastructure runtime
	datastructure_runtime drt(phases, m_nodeMap); 

	// Gather node memory requirements and assign memory to each phase
	assign_memory(phases, memory, drt);

	// Exception guarantees are the following:
	//   Progress indicators:
	//     We use RAII to match init() calls with done() calls.
	//     This means that we call done() on a progress indicator
	//     during stack unwinding if an exception is thrown.
	//   begin() and end():
	//     If an exception is thrown by an initiator,
	//     we do not call end() even though we called begin().
	//     This is to signal to the nodes that processing was aborted.
	//     A node may do finalization cleanup in its destructor
	//     rather than in end() to handle exceptions robustly.

	// Construct fractional progress indicators:
	// Get the name of each phase and call init() on the given indicator.
	progress_indicators pi;
	pi.init(items, progress, phases, file, function);

	for (size_t i = 0; i < phases.size(); ++i) {
		// Run each phase:
		// Evacuate previous if necessary
		if (i > 0 && evacuateWhenDone[i-1]) evacuate_all(phases[i-1]);
		// call propagate in item source to item sink order
		propagate_all(itemFlow[i]);
		// reassign memory to all nodes in the phase
		reassign_memory(phases, i, memory, drt);
		// sum number of steps and call pi.init()
		phase_progress_indicator phaseProgress(pi, i, phases[i]);
		// set progress indicators on each node
		set_progress_indicators(phases[i], phaseProgress.get());
		// call begin in leaf to root actor order
		begin_end beginEnd(actor[i]);
		beginEnd.begin();
		// call go on initiators
		go_initiators(phases[i]);
		// call end in root to leaf actor order
		beginEnd.end();
		// call pi.done in ~phase_progress_indicator
	}
	// call fp->done in ~progress_indicators
}

void runtime::get_item_sources(std::vector<node *> & itemSources) {
	typedef node_map::id_t id_t;
	std::set<id_t> possibleSources;
	for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
		possibleSources.insert(i->first);
	}
	const node_map::relmap_t & relations = m_nodeMap.get_relations();
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		id_t from = i->first;
		id_t to = i->second.first;
		bits::node_relation rel = i->second.second;

		switch (rel) {
			case pushes:
				possibleSources.erase(to);
				break;
			case pulls:
			case depends:
			case no_forward_depends:
				possibleSources.erase(from);
				break;
		}
	}
	for (std::set<id_t>::iterator i = possibleSources.begin();
		 i != possibleSources.end(); ++i) {
		itemSources.push_back(m_nodeMap.get(*i));
	}
}

void runtime::get_item_sinks(std::vector<node *> & itemSinks) {
	typedef node_map::id_t id_t;
	std::set<id_t> possibleSinks;
	for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
		possibleSinks.insert(i->first);
	}
	const node_map::relmap_t & relations = m_nodeMap.get_relations();
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		id_t from = i->first;
		id_t to = i->second.first;
		bits::node_relation rel = i->second.second;

		switch (rel) {
			case pushes:
				possibleSinks.erase(from);
				break;
			case pulls:
			case depends:
			case no_forward_depends:
				possibleSinks.erase(to);
				break;
		}
	}
	for (std::set<id_t>::iterator i = possibleSinks.begin();
		 i != possibleSinks.end(); ++i) {
		itemSinks.push_back(m_nodeMap.get(*i));
	}
}

void runtime::get_phase_map(std::map<node *, size_t> & phaseMap) {
	typedef node_map::id_t id_t;
	std::map<id_t, size_t> numbering;
	std::vector<node *> nodeOrder;
	for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
		numbering[i->second->get_id()] = nodeOrder.size();
		nodeOrder.push_back(i->second);
	}
	const size_t N = nodeOrder.size();

	tpie::disjoint_sets<size_t> unionFind(N);
	for (size_t i = 0; i < N; ++i)
		unionFind.make_set(i);

	const node_map::relmap_t & relations = m_nodeMap.get_relations();
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		if (i->second.second != depends && i->second.second != no_forward_depends)
			unionFind.union_set(numbering[i->first], numbering[i->second.first]);
	}

	const size_t NIL = N;
	std::vector<size_t> phaseNumber(N, NIL);
	size_t nextPhase = 0;
	for (size_t i = 0; i < N; ++i) {
		size_t & phase = phaseNumber[unionFind.find_set(i)];
		if (phase == NIL) {
			phase = nextPhase++;
		}
		phaseMap[nodeOrder[i]] = phase;
	}
}

void runtime::get_phase_graph(const std::map<node *, size_t> & phaseMap,
							  graph<size_t> & phaseGraph)
{
	for (std::map<node *, size_t>::const_iterator i = phaseMap.begin();
		 i != phaseMap.end(); ++i) {
		phaseGraph.add_node(i->second);
	}

	const node_map::relmap_t & relations = m_nodeMap.get_relations();
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		if (i->second.second == depends || i->second.second == no_forward_depends)
			phaseGraph.add_edge(phaseMap.find(m_nodeMap.get(i->second.first))->second,
								phaseMap.find(m_nodeMap.get(i->first))->second);
	}
}

// Compute the inverse of a permutation f : {0, 1, ... N-1} -> {0, 1, ... N-1}
/*static*/
std::vector<size_t> runtime::inverse_permutation(const std::vector<size_t> & f) {
	std::vector<size_t> result(f.size(), f.size());
	for (size_t i = 0; i < f.size(); ++i) {
		if (f[i] >= f.size())
			throw tpie::exception("inverse_permutation: f has bad range");
		if (result[f[i]] != f.size())
			throw tpie::exception("inverse_permutation: f is not injective");
		result[f[i]] = i;
	}
	for (size_t i = 0; i < result.size(); ++i) {
		if (result[i] == f.size())
			throw tpie::exception("inverse_permutation: f is not surjective");
	}
	return result;
}

void runtime::get_phases(const std::map<node *, size_t> & phaseMap,
						 const graph<size_t> & phaseGraph,
						 std::vector<bool> & evacuateWhenDone,
						 std::vector<std::vector<node *> > & phases)
{
	std::vector<size_t> topologicalOrder;
	phaseGraph.rootfirst_topological_order(topologicalOrder);
	// topologicalOrder[0] is the first phase to run,
	// topologicalOrder[1] the next, and so on.

	// Compute inverse permutation such that
	// topoOrderMap[i] is the time at which we run phase i.
	std::vector<size_t> topoOrderMap = inverse_permutation(topologicalOrder);

	// Distribute nodes according to the topological order
	phases.resize(topologicalOrder.size());
	for (std::map<node *, size_t>::const_iterator i = phaseMap.begin();
		 i != phaseMap.end(); ++i)
	{
		phases[topoOrderMap[i->second]].push_back(i->first);
	}

	evacuateWhenDone.resize(phases.size(), false);
	for (size_t i = 0; i + 1 < phases.size(); ++i) {
		if (!phaseGraph.has_edge(topologicalOrder[i], topologicalOrder[i+1]))
			evacuateWhenDone[i] = true;
	}
}

void runtime::get_item_flow_graphs(std::vector<std::vector<node *> > & phases,
								   std::vector<graph<node *> > & itemFlow)
{
	itemFlow.resize(phases.size());
	for (size_t i = 0; i < phases.size(); ++i)
		get_graph(phases[i], itemFlow[i], true);
}

void runtime::get_actor_graphs(std::vector<std::vector<node *> > & phases,
							   std::vector<graph<node *> > & actors)
{
	actors.resize(phases.size());
	for (size_t i = 0; i < phases.size(); ++i)
		get_graph(phases[i], actors[i], false);
}

void runtime::get_graph(std::vector<node *> & phase, graph<node *> & result,
						bool itemFlow)
{
	const node_map::relmap_t & relations = m_nodeMap.get_relations();
	typedef node_map::relmapit relmapit;
	for (size_t i = 0; i < phase.size(); ++i) {
		result.add_node(phase[i]);
		std::pair<relmapit, relmapit> edges =
			relations.equal_range(phase[i]->get_id());
		for (relmapit j = edges.first; j != edges.second; ++j) {
			node * u = m_nodeMap.get(j->first);
			node * v = m_nodeMap.get(j->second.first);
			if (j->second.second == depends || j->second.second == no_forward_depends) continue;
			if (itemFlow && j->second.second == pulls) std::swap(u, v);
			result.add_edge(u, v);
		}
	}
}

bool runtime::is_initiator(node * n) {
	node_map::id_t id = n->get_id();
	return m_nodeMap.in_degree(id, pushes) == 0
		&& m_nodeMap.in_degree(id, pulls) == 0;
}

bool runtime::has_initiator(const std::vector<node *> & phase) {
	for (size_t i = 0; i < phase.size(); ++i)
		if (is_initiator(phase[i])) return true;
	return false;
}

void runtime::ensure_initiators(const std::vector<std::vector<node *> > & phases) {
	for (size_t i = 0; i < phases.size(); ++i)
		if (!has_initiator(phases[i])) throw no_initiator_node();
}

void runtime::prepare_all(const std::vector<graph<node *> > & itemFlow) {
	for (size_t i = 0; i < itemFlow.size(); ++i) {
		const graph<node *> & g = itemFlow[i];
		std::vector<node *> topoOrder;
		g.topological_order(topoOrder);
		for (size_t i = 0; i < topoOrder.size(); ++i) {
			topoOrder[i]->set_state(node::STATE_IN_PREPARE);
			topoOrder[i]->prepare();
			topoOrder[i]->set_state(node::STATE_AFTER_PREPARE);
		}
	}
}

void runtime::evacuate_all(const std::vector<node *> & phase) {
	for (size_t i = 0; i < phase.size(); ++i) {
		if (phase[i]->can_evacuate()) {
			phase[i]->evacuate();
			tpie::log_debug() << "Evacuated node " << phase[i]->get_id() << std::endl;
		}
	}
}

void runtime::propagate_all(const graph<node *> & itemFlow) {
	std::vector<node *> topoOrder;
	itemFlow.topological_order(topoOrder);
	for (size_t i = 0; i < topoOrder.size(); ++i) {
		topoOrder[i]->set_state(node::STATE_IN_PROPAGATE);
		topoOrder[i]->propagate();
		topoOrder[i]->set_state(node::STATE_AFTER_PROPAGATE);
	}
}

void runtime::set_progress_indicators(const std::vector<node *> & phase,
									  progress_indicator_base & pi) {
	for (size_t i = 0; i < phase.size(); ++i)
		phase[i]->set_progress_indicator(&pi);
}

void runtime::go_initiators(const std::vector<node *> & phase) {
	std::vector<node *> initiators;
	for (size_t i = 0; i < phase.size(); ++i)
		if (is_initiator(phase[i])) initiators.push_back(phase[i]);
	for (size_t i = 0; i < initiators.size(); ++i) {
		initiators[i]->set_state(node::STATE_IN_GO);
		initiators[i]->go();
		initiators[i]->set_state(node::STATE_AFTER_BEGIN);
	}
}

/*static*/
void runtime::assign_memory(const std::vector<std::vector<node *> > & phases,
							memory_size_type memory,
							datastructure_runtime & drt) {
	for (size_t phase = 0; phase < phases.size(); ++phase) {
		memory_runtime mrt(phases[phase]);

		double c = get_memory_factor(memory, phase, mrt, drt, false);
		drt.minimize_factor(c, phase);
	}

	for (size_t phase = 0; phase < phases.size(); ++phase) {
		memory_runtime mrt(phases[phase]);

		double c = get_memory_factor(memory, phase, mrt, drt, true);
#ifndef TPIE_NDEBUG
		mrt.print_memory(c, log_debug());
#endif // TPIE_NDEBUG
		mrt.assign_memory(c);
	}
	drt.assign_memory();
}

/*static*/
void runtime::reassign_memory(const std::vector<std::vector<node *> > & phases,
							  size_t phase,
							  memory_size_type memory,
							  const datastructure_runtime & drt) {
	memory_runtime mrt(phases[phase]);
	double c = get_memory_factor(memory, phase, mrt, drt, true);
#ifndef TPIE_NDEBUG
	mrt.print_memory(c, log_debug());
#endif // TPIE_NDEBUG
	mrt.assign_memory(c);
}

/*static*/
double runtime::get_memory_factor(memory_size_type memory, memory_size_type phase, const memory_runtime & mrt, const datastructure_runtime & drt, bool datastructures_locked) {
	memory_size_type min = mrt.sum_minimum_memory() + drt.sum_minimum_memory(phase);
	if (min > memory) {
		log_warning() << "Not enough memory for pipelining phase ("
					  << min << " > " << memory << ")"
					  << std::endl;
		return 0.0;
	}

	// This case is handled specially to avoid dividing by zero later on.
	double fraction_sum = mrt.sum_fraction() + drt.sum_fraction(phase);
	if (fraction_sum < 1e-9) {
		return 0.0;
	}

	double c_lo = 0.0;
	double c_hi = 1.0;
	// Exponential search
	memory_size_type oldMemoryAssigned = 0;
	while (true) {
		double factor = memory * c_hi / fraction_sum;
		memory_size_type memoryAssigned = mrt.sum_assigned_memory(factor) + (datastructures_locked ? drt.sum_assigned_memory(phase) : drt.sum_assigned_memory(factor, phase));
		if (memoryAssigned < memory && memoryAssigned != oldMemoryAssigned)
			c_hi *= 2;
		else
			break;
		oldMemoryAssigned = memoryAssigned;
	}

	// Binary search
	while (c_hi - c_lo > 1e-6) {
		double c = c_lo + (c_hi-c_lo)/2;
		double factor = memory * c / fraction_sum;
		memory_size_type memoryAssigned = mrt.sum_assigned_memory(factor) + (datastructures_locked ? drt.sum_assigned_memory(phase) : drt.sum_assigned_memory(factor, phase));

		if (memoryAssigned > memory) {
			c_hi = c;
		} else {
			c_lo = c;
		}
	}

	return memory * c_lo / fraction_sum;
}

}

}

}
