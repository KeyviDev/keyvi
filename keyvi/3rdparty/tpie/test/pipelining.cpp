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

#include <tpie/tpie.h>
#include <tpie/pipelining.h>
#include <random>
#include <tpie/file_stream.h>
#include <iostream>
#include <sstream>
#include <tpie/progress_indicator_arrow.h>

namespace TP = tpie;
namespace P = tpie::pipelining;

/* This file should solve the following problem:
 * Given a graph consisting of (nodeid, parentid), find the number of children.
 * We solve this by sorting the input twice. Once by id, once by parent.
 * We then scan through both sorted streams at the same time, annotating each
 * (nodeid, parentid) with the number of nodes whose parentid is nodeid.
 */

// Pipelining item type
struct node {
	size_t id;
	size_t parent;

	friend std::ostream & operator<<(std::ostream & stream, const node & n) {
		return stream << '(' << n.id << ", " << n.parent << ')';
	}
};

node make_node(size_t id, size_t parent) {
	node n;
	n.id = id;
	n.parent = parent;
	return n;
}

// Comparator
struct sort_by_id {
	inline bool operator()(const node & lhs, const node & rhs) {
		return lhs.id < rhs.id;
	}
};

// Comparator
struct sort_by_parent {
	inline bool operator()(const node & lhs, const node & rhs) {
		return lhs.parent < rhs.parent;
	}
};

// Pipelining item type
struct node_output {
	node_output(const node & from) : id(from.id), parent(from.parent), children(0) {
	}
	size_t id;
	size_t parent;
	size_t children;

	friend std::ostream & operator<<(std::ostream & stream, const node_output & n) {
		return stream << '(' << n.id << ", " << n.parent << ", " << n.children << ')';
	}
};

template <typename dest_t>
class input_nodes_t : public P::node {
public:
	typedef node item_type;

	inline input_nodes_t(dest_t dest, size_t nodes)
		: dest(std::move(dest))
		, nodes(nodes)
	{
		set_steps(nodes);
	}

	virtual void go() override {
		static std::mt19937 mt;
		static std::uniform_int_distribution<> dist(0, nodes-1);
		dest.begin();
		for (size_t i = 0; i < nodes; ++i) {
			dest.push(make_node(i, dist(mt)));
			step();
		}
		dest.end();
	}

private:
	dest_t dest;
	size_t nodes;
};

typedef  P::pipe_begin<P::factory<input_nodes_t, size_t> > input_nodes;

template <typename dest_t, typename byid_t, typename byparent_t>
class count_t : public P::node {
	dest_t dest;
	byid_t byid;
	byparent_t byparent;

public:
	count_t(dest_t dest, byid_t byid, byparent_t byparent)
		: dest(std::move(dest)), byid(std::move(byid)), byparent(std::move(byparent))
	{
		add_push_destination(dest);
		add_pull_source(byid);
		add_pull_source(byparent);
	}

	virtual void go() override {
		tpie::unique_ptr<::node> buf(nullptr);
		while (byid.can_pull()) {
			node_output cur = byid.pull();
			if (buf.get()) {
				if (buf->parent != cur.id) {
					goto seen_children;
				} else {
					++cur.children;
				}
			}
			while (byparent.can_pull()) {
				::node child = byparent.pull();
				if (child.parent != cur.id) {
					if (!buf.get()) {
						buf.reset(TP::tpie_new< ::node>(child));
					} else {
						*buf = child;
					}
					break;
				} else {
					++cur.children;
				}
			}
seen_children:
			dest.push(cur);
		}
	}
};

template <typename byid_t, typename byparent_t>
class count_factory : public P::factory_base {
	typedef typename byid_t::factory_type byid_fact_t;
	typedef typename byparent_t::factory_type byparent_fact_t;
	typedef typename byid_fact_t::constructed_type byid_gen_t;
	typedef typename byparent_fact_t::constructed_type byparent_gen_t;

public:
	template <typename dest_t>
	struct constructed {
		typedef count_t<dest_t, byid_gen_t, byparent_gen_t> type;
	};

	count_factory(byid_t && byid, byparent_t && byparent)
		: m_byid(std::move(byid.factory))
		, m_byparent(std::move(byparent.factory))
	{
	}

	template <typename dest_t>
	count_t<dest_t, byid_gen_t, byparent_gen_t>
	construct(dest_t dest) {
		return count_t<dest_t, byid_gen_t, byparent_gen_t>
			(std::move(dest), m_byid.construct(), m_byparent.construct());
	}

private:
	byid_fact_t m_byid;
	byparent_fact_t m_byparent;
};

template <typename byid_t, typename byparent_t>
P::pipe_begin<count_factory<byid_t, byparent_t> >
count(byid_t && byid, byparent_t && byparent) {
	return count_factory<byid_t, byparent_t>(std::forward<byid_t>(byid), std::forward<byparent_t>(byparent));
}

class output_count_t : public P::node {
public:
	size_t children;
	size_t nodes;

	inline output_count_t()
		: children(0)
		, nodes(0)
	{
	}

	virtual void begin() override {
		TP::log_info() << "Begin output" << std::endl;
	}

	virtual void end() override {
		TP::log_info() << "End output" << std::endl;
		TP::log_info() << "We saw " << nodes << " nodes and " << children << " children" << std::endl;
	}

	inline void push(const node_output & node) {
		if (nodes < 32) TP::log_info() << node << std::endl;
		else if (nodes == 32) TP::log_info() << "..." << std::endl;
		children += node.children;
		++nodes;
	}
};

typedef P::pipe_end<P::termfactory<output_count_t> > output_count;

int main(int argc, char ** argv) {
	TP::tpie_init(TP::ALL & ~TP::DEFAULT_LOGGING);
	bool debug_log = false;
	bool progress = true;

	{
		TP::stderr_log_target stderr_target(debug_log ? TP::LOG_DEBUG : TP::LOG_ERROR);
		TP::get_log().add_target(&stderr_target);

		TP::get_memory_manager().set_limit(13*1024*1024);

		size_t nodes = 1 << 24;
		if (argc > 1) std::stringstream(argv[1]) >> nodes;
		P::passive_sorter<node, sort_by_id> byid;
		P::passive_sorter<node, sort_by_parent> byparent;
		P::pipeline p1 =
			input_nodes(nodes)
			| P::fork(byid.input().name("Sort by id"))
			| byparent.input().name("Sort by parent");

		P::pipeline p2 =
			count(byid.output(), byparent.output())
			| output_count();
		p1.plot();
		if (progress) {
			TP::progress_indicator_arrow pi("Test", nodes);
			p1(nodes, pi, TPIE_FSI);
		} else {
			p1();
		}
		TP::get_log().remove_target(&stderr_target);
	}
	TP::tpie_finish(TP::ALL & ~TP::DEFAULT_LOGGING);
	return 0;
}
