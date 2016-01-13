// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_PAIR_FACTORY_H__
#define __TPIE_PIPELINING_PAIR_FACTORY_H__

#include <tpie/types.h>
#include <tpie/tpie_log.h>
#include <tpie/pipelining/priority_type.h>
#include <tpie/pipelining/factory_base.h>
#include <boost/scoped_array.hpp>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename child_t>
class pair_factory_base {
public:
	pair_factory_base()
		: m_maps(new node_map::ptr[2])
		, m_final(false)
	{
	}

	pair_factory_base(const pair_factory_base & other)= delete;
	pair_factory_base(pair_factory_base && other) {
		using std::swap;
		m_final = other.m_final;
		swap(m_maps, other.m_maps);
	}

	pair_factory_base & operator=(const pair_factory_base & other) = delete;
	pair_factory_base & operator=(pair_factory_base && other) {
		using std::swap;
		m_final = other.m_final;
		swap(m_maps, other.m_maps);
	}

	inline double memory() const {
		return self().fact1.memory() + self().fact2.memory();
	}

	inline void name(const std::string & n, priority_type) {
		push_breadcrumb(n);
	}

	void push_breadcrumb(const std::string & n) {
		self().fact1.push_breadcrumb(n);
		self().fact2.push_breadcrumb(n);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  See factory_base::hook_initialization.
	///////////////////////////////////////////////////////////////////////////
	void hook_initialization(factory_init_hook * hook) {
		self().hook_initialization_impl(hook);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Internal - used by subclasses to record references to
	/// node_maps for a later connectivity check.
	///////////////////////////////////////////////////////////////////////////
	template <typename pipe_t>
	pipe_t record(size_t idx, pipe_t && pipe) const {
		m_maps[idx] = pipe.get_node_map();
		if (idx == 0 && m_final) {
			// Now is the opportunity to check that the constructed pipeline is
			// connected.
			assert_connected();
			self().recursive_connected_check();
		}
		return std::move(pipe);
	}

	void assert_connected() const {
		if (m_maps[0]->find_authority() != m_maps[1]->find_authority()) {
			log_error() << "Node map disconnected - more information in debug log"
						<< " (" << typeid(child_t).name() << ")" << std::endl;
			log_debug()
				<< "Note about node implementations.\n\n"
				   "In a node constructor that accepts a destination node,\n"
				   "a relation should always be established between the current node\n"
				   "and the destination using one of the member functions add_push_destination,\n"
				   "add_pull_source and add_dependency.\n\n"
				   "If this relational graph is not connected, some nodes will not\n"
				   "be initialized: prepare(), begin(), end() and other methods will never\n"
				   "be called, and memory will not be assigned.\n"
				   "---------------------------------------------------------------------------" << std::endl;
			throw tpie::exception("Node map disconnected - did you forget to add_push_destination?");
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Signal that this factory is used to instantiate a pipeline_impl,
	/// i.e. that it was made by piping together a pipe_begin and a pipe_end.
	///////////////////////////////////////////////////////////////////////////
	child_t & finalize() {
		m_final = true;
		return self();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief
	///////////////////////////////////////////////////////////////////////////
	void set_destination_kind_push() {
		self().fact2.set_destination_kind_push();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief
	///////////////////////////////////////////////////////////////////////////
	void set_destination_kind_pull() {
		self().fact1.set_destination_kind_pull();
	}

private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}

	boost::scoped_array<node_map::ptr> m_maps;
	bool m_final;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Contains a method `check` that calls recursive_connected_check when
/// fact_t is a pair factory, and otherwise does nothing.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct maybe_check_connected;

template <typename fact1_t, typename fact2_t>
class pair_factory : public pair_factory_base<pair_factory<fact1_t, fact2_t> > {
public:
	template <typename dest_t>
	struct constructed {
		typedef typename fact1_t::template constructed<typename fact2_t::template constructed<dest_t>::type>::type type;
	};

	pair_factory(const pair_factory &) = delete;
	pair_factory(pair_factory &&) = default;
	pair_factory & operator=(const pair_factory &) = delete;
	pair_factory & operator=(pair_factory &&) = default;

	pair_factory(fact1_t && fact1, fact2_t && fact2)
		: fact1(std::move(fact1)), fact2(std::move(fact2)) {
	}

	template <typename dest_t>
	typename constructed<dest_t>::type
	construct(dest_t && dest) {
		return this->record(0, fact1.construct(this->record(1, fact2.construct(std::forward<dest_t>(dest)))));
	}

	template <typename dest_t>
	typename constructed<dest_t>::type
	construct_copy(dest_t && dest) {
		return this->record(0, fact1.construct_copy(this->record(1, fact2.construct_copy(std::forward<dest_t>(dest)))));
	}

	void recursive_connected_check() const {
		maybe_check_connected<fact1_t>::check(fact1);
		maybe_check_connected<fact2_t>::check(fact2);
	}

	void hook_initialization_impl(factory_init_hook * hook) {
		fact1.hook_initialization(hook);
		fact2.hook_initialization(hook);
	}

	fact1_t fact1;
	fact2_t fact2;
};

template <typename fact1_t, typename termfact2_t>
class termpair_factory : public pair_factory_base<termpair_factory<fact1_t, termfact2_t> > {
public:
	typedef typename fact1_t::template constructed<typename termfact2_t::constructed_type>::type constructed_type;



	termpair_factory(const termpair_factory &) = delete;
	termpair_factory(termpair_factory &&) = default;
	termpair_factory & operator=(const termpair_factory &) = delete;
	termpair_factory & operator=(termpair_factory &&) = default;

	termpair_factory(fact1_t && fact1, termfact2_t && fact2)
		: fact1(std::move(fact1))
		, fact2(std::move(fact2))
	{
	}

	fact1_t fact1;
	termfact2_t fact2;

	constructed_type construct() {
		return this->record(0, fact1.construct(this->record(1, fact2.construct())));
	}

	constructed_type construct_copy() {
		return this->record(0, fact1.construct_copy(this->record(1, fact2.construct_copy())));
	}

	void recursive_connected_check() const {
		maybe_check_connected<fact1_t>::check(fact1);
		maybe_check_connected<termfact2_t>::check(fact2);
	}

	void hook_initialization_impl(factory_init_hook * hook) {
		fact1.hook_initialization(hook);
		fact2.hook_initialization(hook);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief For pair factories, recursively check that the nodes created
/// share their node_map.
///////////////////////////////////////////////////////////////////////////////
template <typename fact1_t, typename fact2_t>
struct maybe_check_connected<pair_factory<fact1_t, fact2_t> > {
	static void check(const pair_factory<fact1_t, fact2_t> & fact) {
		fact.assert_connected();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief See pair_factory specialization.
///////////////////////////////////////////////////////////////////////////////
template <typename fact1_t, typename termfact2_t>
struct maybe_check_connected<termpair_factory<fact1_t, termfact2_t> > {
	static void check(const termpair_factory<fact1_t, termfact2_t> & fact) {
		fact.assert_connected();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief For user-defined factories in the general case, we cannot do
/// anything to ensure that node maps are joined together.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct maybe_check_connected {
	static void check(const fact_t & /*fact*/) {
	}
};

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PAIR_FACTORY_H__
