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

#include <tpie/pipelining/node.h>

namespace tpie {

namespace pipelining {

namespace bits {

proxy_progress_indicator::proxy_progress_indicator(node & s)
	: progress_indicator_base(1)
	, m_node(s)
{
}

void proxy_progress_indicator::refresh() {
	double proxyMax = static_cast<double>(get_range());
	double proxyCur = static_cast<double>(get_current());
	double parentMax = static_cast<double>(m_node.m_parameters.stepsTotal);
	double parentCur = static_cast<double>(m_node.m_parameters.stepsTotal-m_node.m_stepsLeft);
	double missing = parentMax*proxyCur/proxyMax - parentCur;
	if (missing < 1.0) return;
	stream_size_type times = static_cast<stream_size_type>(1.0+missing);
	times = std::min(m_node.m_stepsLeft, times);
	m_node.step(times);
}

} // namespace bits

const std::string & node::get_name() {
	if (m_parameters.name.empty()) {
		m_parameters.name = bits::extract_pipe_name(typeid(*this).name());
	}
	return m_parameters.name;
}

void node::set_name(const std::string & name, priority_type priority) {
	m_parameters.name = name;
	m_parameters.namePriority = priority;
}

const std::string & node::get_phase_name() {
	return m_parameters.phaseName;
}

void node::set_phase_name(const std::string & name, priority_type priority) {
	m_parameters.phaseName = name;
	m_parameters.phaseNamePriority = priority;
}


node::node()
	: token(this)
	, m_flushPriority(0)
	, m_stepsLeft(0)
	, m_pi(0)
	, m_state(STATE_FRESH)
	, m_plotOptions()
{
}

node::node(node && other)
	: token(other.token, this)
	, m_parameters(std::move(other.m_parameters))
	, m_buckets(std::move(other.m_buckets))
	, m_flushPriority(std::move(other.m_flushPriority))
	, m_stepsLeft(std::move(other.m_stepsLeft))
	, m_pi(std::move(other.m_pi))
	, m_state(std::move(other.m_state))
	, m_plotOptions(std::move(other.m_plotOptions))
{
	if (m_state != STATE_FRESH)
		throw call_order_exception(
			"Tried to move pipeline node after prepare had been called");
}

node & node::operator=(node && other) {
	this->~node();
	new (this)node(std::move(other));
	return *this;
}

node::node(const node_token & token)
	: token(token, this, true)
	, m_parameters()
	, m_flushPriority(0)
	, m_stepsLeft(0)
	, m_pi(0)
	, m_state(STATE_FRESH)
	, m_plotOptions()
{
}

void node::add_push_destination(const node_token & dest) {
	bits::node_map::ptr m = token.map_union(dest);
	m->add_relation(token.id(), dest.id(), bits::pushes);
}

void node::add_push_destination(const node & dest) {
	if (get_state() != STATE_FRESH) {
		throw call_order_exception("add_push_destination called too late");
	}
	add_push_destination(dest.token);
}

void node::add_pull_source(const node_token & dest) {
	if (get_state() != STATE_FRESH) {
		throw call_order_exception("add_pull_source called too late");
	}
	bits::node_map::ptr m = token.map_union(dest);
	m->add_relation(token.id(), dest.id(), bits::pulls);
}

void node::add_pull_source(const node & dest) {
	add_pull_source(dest.token);
}

void node::add_dependency(const node_token & dest) {
	bits::node_map::ptr m = token.map_union(dest);
	m->add_relation(token.id(), dest.id(), bits::depends);
}

void node::add_dependency(const node & dest) {
	add_dependency(dest.token);
}

void node::add_memory_share_dependency(const node_token & dest) {
	bits::node_map::ptr m = token.map_union(dest);
	m->add_relation(token.id(), dest.id(), bits::memory_share_depends);
}

void node::add_memory_share_dependency(const node & dest) {
	add_memory_share_dependency(dest.token);
}


#define TPIE_RESOURCE_SETTER(setter_name, param_type, param_name) \
	void node::setter_name(resource_type type, param_type value) { \
		switch (get_state()) { \
		case STATE_IN_PROPAGATE: \
		case STATE_AFTER_PROPAGATE: \
		case STATE_FRESH: \
		case STATE_IN_PREPARE: \
			break; \
		default: \
			resource_type t = get_resource_being_assigned(); \
			/* If the changed resource is being assigned later,
			 * allow changing it
			 */ \
			if (t != NO_RESOURCE && type > t) \
				break; \
			throw call_order_exception(#setter_name); \
		} \
		m_parameters.resource_parameters[type].param_name = value; \
	}

TPIE_RESOURCE_SETTER(set_minimum_resource_usage, memory_size_type, minimum);
TPIE_RESOURCE_SETTER(set_maximum_resource_usage, memory_size_type, maximum);
TPIE_RESOURCE_SETTER(set_resource_fraction, double, fraction);

#undef TPIE_RESOURCE_SETTER

void node::_internal_set_available_of_resource(resource_type type, memory_size_type available) {
	m_parameters.resource_parameters[type].available = available;
	resource_available_changed(type, available);
	if (type == MEMORY) {
		// Legacy interface
		set_available_memory(available);
	}
}

void node::forward_any(std::string key, any_noncopyable value, memory_size_type k) {
	switch (get_state()) {
		case STATE_FRESH:
		case STATE_IN_PREPARE:
		case STATE_AFTER_PREPARE:
			// Allowed since forward() is allowed in prepare()
			break;
		case STATE_IN_PROPAGATE:
		case STATE_AFTER_PROPAGATE:
			// Allowed since forward() is allowed in propagate()
			break;
		case STATE_IN_BEGIN:
			throw call_order_exception("forward");
		case STATE_AFTER_BEGIN:
		case STATE_IN_END:
		case STATE_AFTER_END:
			// Allowed since forward() is allowed in end()
			break;
		default:
			log_debug() << "forward in unknown state " << get_state() << std::endl;
			break;
	}

	m_forwardedFromHere[key] = std::move(value);

	bits::node_map::ptr nodeMap = get_node_map()->find_authority();

	typedef node_token::id_t id_t;
	std::vector<id_t> successors;
	nodeMap->get_successors(get_id(), successors, k, true);
	for (auto i : successors) {
		nodeMap->get(i)->add_forwarded_data(key, get_id());
	}
}

void node::add_forwarded_data(std::string key, node_token::id_t from_node) {
	m_forwardedToHere[key] = from_node;
}

node::maybeany_t node::fetch_maybe(std::string key) {
	auto nodeMap = get_node_map()->find_authority();

	auto it = m_forwardedToHere.find(key);
	if (it == m_forwardedToHere.end()) {
		// Try to lookup the key in the node map
		return nodeMap->fetch_maybe(key);
	}

	auto fetch_from_id = it->second;
	node *fetch_from = nodeMap->get(fetch_from_id);
	if (!fetch_from) {
		return maybeany_t();
	}

	return fetch_from->get_forwarded_data_maybe(key);
}

any_noncopyable & node::fetch_any(std::string key) {
	maybeany_t value = fetch_maybe(key);
	if (!value) {
		std::stringstream ss;
		ss << "Tried to fetch nonexistent key '" << key << "'" 
		   << " in " << get_name() << " of type " << typeid(*this).name();
		throw invalid_argument_exception(ss.str());
	}
	return *value;
}

node::maybeany_t node::get_forwarded_data_maybe(std::string key) {
	auto it = m_forwardedFromHere.find(key);
	if (it == m_forwardedFromHere.end()) {
		return maybeany_t();
	}
	return maybeany_t(it->second);
}

void node::set_steps(stream_size_type steps) {
	switch (get_state()) {
		case STATE_FRESH:
		case STATE_IN_PREPARE:
		case STATE_IN_PROPAGATE:
			break;
		case STATE_IN_BEGIN:
			log_error() << "set_steps in begin(); use set_steps in propagate() instead." << std::endl;
			throw call_order_exception("set_steps");
		default:
			log_error() << "set_steps in unknown state " << get_state() << std::endl;
			throw call_order_exception("set_steps");
	}
	m_parameters.stepsTotal = m_stepsLeft = steps;
}

void node::step_overflow()  {
	if (m_parameters.stepsTotal != std::numeric_limits<stream_size_type>::max()) {
		log_warning() << typeid(*this).name() << " ==== Too many steps " << m_parameters.stepsTotal << std::endl;
		m_stepsLeft = 0;
		m_parameters.stepsTotal = std::numeric_limits<stream_size_type>::max();
	}
}

progress_indicator_base * node::proxy_progress_indicator() {
	if (m_piProxy.get() != 0) return m_piProxy.get();
	progress_indicator_base * pi = new bits::proxy_progress_indicator(*this);
	m_piProxy.reset(pi);
	return pi;
}

void node::register_datastructure_usage(const std::string & name, double priority) {
	datastructuremap_t::iterator i = m_datastructures.find(name);

	if(i != m_datastructures.end())
		throw tpie::exception("duplicate datastructure registration");

	datastructure_info_t info;
	info.priority = priority;
	m_datastructures.insert(std::make_pair(name, info));
}

void node::set_datastructure_memory_limits(const std::string & name, memory_size_type min, memory_size_type max) {
	datastructuremap_t::iterator i = m_datastructures.find(name);

	if(i == m_datastructures.end())
		throw tpie::exception("attempted to set memory limits of non-registered datastructure");

	i->second.min = min;
	i->second.max = max;
}

memory_size_type node::get_datastructure_memory(const std::string & name) {
	const bits::node_map::datastructuremap_t & structures = get_node_map()->get_datastructures();
	bits::node_map::datastructuremap_t::const_iterator i = structures.find(name);

	if(i == structures.end())
		throw tpie::exception("attempted to get memory of non-registered datastructure");

	return i->second.first;
}

} // namespace pipelining

} // namespace tpie
