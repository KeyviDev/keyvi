// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
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

///////////////////////////////////////////////////////////////////////////////
/// \file pipelining/buffer.h  Plain old file_stream buffer.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_BUFFER_H__
#define __TPIE_PIPELINING_BUFFER_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/file_stream.h>
#include <tpie/maybe.h>
#include <memory>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename T>
class buffer_pull_output_t: public node {
	tpie::maybe<file_stream<T> > * m_queue_ptr;
	file_stream<T> * m_queue;
public:
	typedef T item_type;

	buffer_pull_output_t(const node_token & input_token) {
		add_dependency(input_token);
		set_name("Fetching items", PRIORITY_SIGNIFICANT);
		set_minimum_memory(file_stream<T>::memory_usage());
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED);
	}

	virtual void propagate() override {
		m_queue_ptr = fetch<tpie::maybe<file_stream<T> > *>("queue");
		m_queue = &**m_queue_ptr;
		m_queue->seek(0);
		forward("items", m_queue->size());
		set_steps(m_queue->size());
	}

	bool can_pull() const {
		return m_queue->can_read();
	}

	T pull() {
		step();
		return m_queue->read();
	}

	virtual void end() override {
		(*m_queue_ptr).destruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Input node for buffer.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class buffer_input_t: public node {
public:
	typedef T item_type;

	buffer_input_t(const node_token & token, std::shared_ptr<node> output=std::shared_ptr<node>())
		: node(token)
		, m_output(output)
	{
		set_name("Storing items", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(tpie::file_stream<item_type>::memory_usage());
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
	}
	
	void begin() override {
		m_queue.construct();
		m_queue->open(static_cast<memory_size_type>(0), access_sequential, compression_normal);
	}

	void push(const T & item) {
		m_queue->write(item);
	}

	void end() override {
		forward("queue", &m_queue, 1);
	}

private:
	tpie::maybe< file_stream<T> > m_queue;
	std::shared_ptr<node> m_output;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Output node for buffer.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class buffer_output_t: public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	buffer_output_t(dest_t dest, const node_token & input_token)
		: dest(std::move(dest))
	{
		add_dependency(input_token);
		add_push_destination(this->dest);
		set_minimum_memory(tpie::file_stream<item_type>::memory_usage());
		set_minimum_resource_usage(FILES, 1);
		set_name("Buffer", PRIORITY_INSIGNIFICANT);
		set_plot_options(PLOT_BUFFERED);
	}


	void propagate() override {
		m_queue_ptr = fetch<tpie::maybe<file_stream<item_type> > *>("queue");
		m_queue = &**m_queue_ptr;
		forward("items", m_queue->size());
		set_steps(m_queue->size());
	}

	void go() override {
		m_queue->seek(0);
		while (m_queue->can_read()) {
			dest.push(m_queue->read());
			step();
		}
	}

	void end() override {
		m_queue_ptr->destruct();
	}
private:
	tpie::maybe<file_stream<item_type> > * m_queue_ptr;
	file_stream<item_type> * m_queue;
	dest_t dest;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Plain old file_stream buffer. Does nothing to the item stream, but
/// it inserts a phase boundary.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class passive_buffer {
public:
	typedef T item_type;
	typedef bits::buffer_input_t<T> input_t;
	typedef bits::buffer_pull_output_t<T> output_t;
private:
	typedef termfactory<input_t,  const node_token &> inputfact_t;
	typedef termfactory<output_t, const node_token &> outputfact_t;
public:
	typedef pipe_end      <inputfact_t>  inputpipe_t;
	typedef pullpipe_begin<outputfact_t> outputpipe_t;


	passive_buffer() {}

	inline input_t raw_input() {
		return input_t(input_token);
	}

	inline output_t raw_output() {
		return output_t(input_token);
	}

	inline inputpipe_t input() {
		return inputfact_t(input_token);
	}

	inline outputpipe_t output() {
		return outputfact_t(input_token);
	}

private:
	node_token input_token;

	passive_buffer(const passive_buffer &);
	passive_buffer & operator=(const passive_buffer &);
};

///////////////////////////////////////////////////////////////////////////////
/// \brief The buffer node inserts a phase boundary into the pipeline by
/// writing items to disk. It does not change the contents of the stream.
///////////////////////////////////////////////////////////////////////////////
typedef pipe_middle<split_factory<bits::buffer_input_t, node, bits::buffer_output_t> > buffer;

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_BUFFER_H__
