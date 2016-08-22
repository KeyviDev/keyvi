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

#ifndef __TPIE_PIPELINING_REVERSE_H__
#define __TPIE_PIPELINING_REVERSE_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/stack.h>
#include <stack>

namespace tpie {

namespace pipelining {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief input node for reverser stored in external memory
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class reverser_input_t: public node {
public:
	typedef T item_type;

	inline reverser_input_t(const node_token & token, std::shared_ptr<node> output=std::shared_ptr<node>())
		: node(token), m_output(output)
	{
		set_name("Store items", PRIORITY_SIGNIFICANT);
		set_minimum_memory(stack<item_type>::memory_usage());
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
	}

	void begin() override {
		m_stack.construct();
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Pushes an item to the node
	///////////////////////////////////////////////////////////////////////////////
	void push(const item_type & t) {
		m_stack->push(t);
	}

	void end() override {
		forward("stack", &m_stack, 1);
	}
private:
	tpie::maybe<stack<item_type> > m_stack;
	std::shared_ptr<node> m_output;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief input node for reverser stored in internal memory
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class internal_reverser_input_t: public node {
public:
	typedef T item_type;

	inline internal_reverser_input_t(const node_token & token, std::shared_ptr<node> output=std::shared_ptr<node>())
		: node(token), m_output(output)
	{
		set_name("Store items", PRIORITY_SIGNIFICANT);
		set_minimum_memory(sizeof(std::stack<item_type>));
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
	}

	virtual void propagate() override {
		m_stack = tpie_new<std::stack<item_type> >();
		forward("stack", m_stack, 1);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Pushes an item to the node
	///////////////////////////////////////////////////////////////////////////////
	void push(const item_type & t) {
		m_stack->push(t);
	}
private:
	std::stack<item_type> * m_stack;
	std::shared_ptr<node> m_output;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Output node for reverser stored in external memory
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class reverser_output_t: public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	reverser_output_t(dest_t dest, const node_token & input_token)
		: dest(std::move(dest))
	{
		add_dependency(input_token);
		add_push_destination(this->dest);
		set_name("Output reversed", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(this->m_stack->memory_usage());
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED);
	}

	void propagate() override {
		m_stack_ptr = fetch<tpie::maybe<stack<item_type> > *>("stack");
		m_stack = &**m_stack_ptr;
		forward("items", m_stack->size());
		set_steps(m_stack->size());
	}

	void go() override {
		while (!m_stack->empty()) {
			dest.push(m_stack->pop());
			step();
		}
	}

	void end() override {
		m_stack_ptr->destruct();
		}
private:
	dest_t dest;
	tpie::maybe<stack<item_type> > * m_stack_ptr;
	stack<item_type> * m_stack;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Output node for reverser stored in internal memory
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class internal_reverser_output_t: public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	internal_reverser_output_t(dest_t dest, const node_token & input_token)
		: dest(std::move(dest))
	{
		add_dependency(input_token);
		add_push_destination(this->dest);
		set_name("Output reversed", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(sizeof(std::stack<item_type>));
		set_plot_options(PLOT_BUFFERED);
	}

	virtual void propagate() override {
		m_stack = fetch<std::stack<item_type> *>("stack");
		forward("items", m_stack->size());
		set_steps(m_stack->size());
	}

	virtual void go() override {
		while (!m_stack->empty()) {
			dest.push(m_stack->pop());
			step();
		}
	}

	virtual void end() override {
		tpie_delete(m_stack);
	}
private:
	dest_t dest;
	std::stack<item_type> * m_stack;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Output node for passive reverser stored in external memory
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class reverser_pull_output_t : public node {
public:
	typedef T item_type;

	reverser_pull_output_t(const node_token & input_token) {
		add_dependency(input_token);
		set_name("Input items to reverse", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(stack<item_type>::memory_usage());
		set_plot_options(PLOT_BUFFERED);
	}

	void propagate() override {
		m_stack_ptr = fetch<tpie::maybe<stack<item_type> > *>("stack");
		m_stack = &**m_stack_ptr;
		forward("items", m_stack->size());
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Whether an item can be pulled from the node
	///////////////////////////////////////////////////////////////////////////////
	bool can_pull() const {
		return !m_stack->empty();
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Pulls an item from the node
	///////////////////////////////////////////////////////////////////////////////
	T pull() {
		return m_stack->pop();
	}

	void end() override {
		m_stack_ptr->destruct();
	}
private:
	tpie::maybe<stack<item_type> > * m_stack_ptr;
	stack<item_type> * m_stack;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Output node for passive reverser stored in internal memory
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class internal_reverser_pull_output_t : public node {
public:
	typedef T item_type;

	internal_reverser_pull_output_t(const node_token & input_token) {
		add_dependency(input_token);
		set_name("Input items to reverse", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(sizeof(std::stack<item_type>));
		set_plot_options(PLOT_BUFFERED);
	}

	virtual void propagate() override {
		m_stack = fetch<std::stack<item_type> *>("stack");
		forward("items", m_stack->size());
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Whether an item can be pulled from the node
	///////////////////////////////////////////////////////////////////////////////
	bool can_pull() const {
		return !m_stack->empty();
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Pull an item from the node
	///////////////////////////////////////////////////////////////////////////////
	T pull() {
		T r = m_stack->top();
		m_stack->pop();
		return r;
	}

	virtual void end() override {
		tpie_delete(m_stack);
	}
private:
	std::stack<item_type> * m_stack;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A passive reverser stored in external memory. Reverses the input
/// stream and creates a phase boundary.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class passive_reverser {
public:
	typedef T item_type;
	typedef bits::reverser_input_t<T> input_t;
	typedef bits::reverser_pull_output_t<T> output_t;
private:
	typedef termfactory<input_t,  const node_token &> inputfact_t;
	typedef termfactory<output_t, const node_token &> outputfact_t;
	typedef pipe_end<inputfact_t>  inputpipe_t;
	typedef pullpipe_begin<outputfact_t> outputpipe_t;
public:
	passive_reverser() {}

	inline input_t raw_input() {
		return input_t(input_token);
	}

	inline output_t raw_output() {
		return output_t(input_token);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns a termfactory for the input nodes
	///////////////////////////////////////////////////////////////////////////////
	inline inputpipe_t input() {
		return inputfact_t(input_token);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns a termfactory for the output nodes
	///////////////////////////////////////////////////////////////////////////////
	inline outputpipe_t output() {
		return outputfact_t(input_token);
	}
private:
	node_token input_token;

	passive_reverser(const passive_reverser &);
	passive_reverser & operator=(const passive_reverser &);
};


///////////////////////////////////////////////////////////////////////////////
/// \brief A passive reverser stored in internal memory. Reverses the input
/// stream and creates a phase boundary.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class internal_passive_reverser {
public:
	typedef T item_type;
	typedef bits::internal_reverser_input_t<T> input_t;
	typedef bits::internal_reverser_pull_output_t<T> output_t;
private:
	typedef termfactory<input_t,  const node_token &> inputfact_t;
	typedef termfactory<output_t, const node_token &> outputfact_t;
	typedef pipe_end<inputfact_t>  inputpipe_t;
	typedef pullpipe_begin<outputfact_t> outputpipe_t;
public:
	internal_passive_reverser() {}

	inline input_t raw_input() {
		return input_t(input_token);
	}

	inline output_t raw_output() {
		return output_t(input_token);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns a termfactory for the input nodes
	///////////////////////////////////////////////////////////////////////////////
	inline inputpipe_t input() {
		return inputfact_t(input_token);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns a termfactory for the output nodes
	///////////////////////////////////////////////////////////////////////////////
	inline outputpipe_t output() {
		return outputfact_t(input_token);
	}
private:
	node_token input_token;

	internal_passive_reverser(const internal_passive_reverser &);
	internal_passive_reverser & operator=(const internal_passive_reverser &);
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Constructs a reverser node stored in external memory. Reverses
/// the stream and creates a phase boundary
///////////////////////////////////////////////////////////////////////////////
typedef pipe_middle<split_factory<bits::reverser_input_t, node, bits::reverser_output_t> > reverser;

///////////////////////////////////////////////////////////////////////////////
/// \brief Constructs a reverser node stored in internal memory. Reverses
/// the stream and creates a phase boundary
///////////////////////////////////////////////////////////////////////////////
typedef pipe_middle<split_factory<bits::internal_reverser_input_t, node, bits::internal_reverser_output_t> > internal_reverser;

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_REVERSE_H__
