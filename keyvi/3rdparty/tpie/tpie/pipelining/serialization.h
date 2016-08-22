// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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
/// \file pipelining/serialization.h  Serialization stream glue.
///////////////////////////////////////////////////////////////////////////////

#ifndef TPIE_PIPELINING_SERIALIZATION_H
#define TPIE_PIPELINING_SERIALIZATION_H

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/pair_factory.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/serialization_stream.h>

namespace tpie {

namespace pipelining {

namespace serialization_bits {

template <typename dest_t>
class input_t : public node {
	dest_t dest;
	serialization_reader * rd;

public:
	typedef typename push_type<dest_t>::type item_type;

	input_t(dest_t dest, serialization_reader * rd)
		: dest(std::move(dest))
		, rd(rd)
	{
		set_name("Serialization reader");
		add_push_destination(this->dest);
		set_minimum_memory(rd->memory_usage());
		set_minimum_resource_usage(FILES, 1);
	}

	virtual void propagate() override {
		set_steps(rd->size());
	}

	virtual void go() override {
		item_type x;
		stream_size_type bytesRead = 0;
		while (rd->can_read()) {
			rd->unserialize(x);
			dest.push(x);

			stream_size_type bytesRead2 = rd->offset();
			step(bytesRead2 - bytesRead);
			bytesRead = bytesRead2;
		}
	}
};

typedef factory<input_t, serialization_reader *> input_factory;


template <typename T>
class output_t : public node {
	serialization_writer * wr;

public:
	typedef T item_type;

	output_t(serialization_writer * wr)
		: wr(wr)
	{
		set_name("Serialization writer");
		set_minimum_memory(wr->memory_usage());
		set_minimum_resource_usage(FILES, 1);
	}

	void push(const T & x) {
		wr->serialize(x);
	}
};

template <typename T>
struct output_factory {
	typedef termfactory<output_t<T>, serialization_writer *> type;
};

} // namespace serialization_bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that reads items from a \ref serialization_reader.
/// \param rd The reader from which which items should be read
///////////////////////////////////////////////////////////////////////////////
pipe_begin<serialization_bits::input_factory>
inline serialization_input(serialization_reader & rd) {
	return pipe_begin<serialization_bits::input_factory>(&rd);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that writes item to a \ref serialization_writer.
/// \param wr The writer to which items should be written
///////////////////////////////////////////////////////////////////////////////
template <typename T>
pipe_end<typename serialization_bits::output_factory<T>::type>
serialization_output(serialization_writer & wr) {
	return typename serialization_bits::output_factory<T>::type(&wr);
}

namespace serialization_bits {

template <typename T>
class reverser_input_t : public node {
public:
	typedef T item_type;

	reverser_input_t(const node_token & token,
					 std::shared_ptr<node> output=std::shared_ptr<node>())
		: node(token), output(output)
		, wr()
		, items(0)
	{
		this->set_name("Serialization reverse writer");
		//TODO memory
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
	}

	void propagate() override {
		file.construct();
		forward<tpie::maybe<tpie::temp_file>*>("__srev_file", &file, 1);
	}

	void begin() override {
		wr.open(file->path());
	}

	void push(const item_type & x) {
		wr.serialize(x);
		++items;
	}

	void end() override {
		wr.close();
		forward<stream_size_type>("items", items);
	}

private:
	tpie::maybe<tpie::temp_file> file;
	std::shared_ptr<node> output;
	serialization_reverse_writer wr;
	stream_size_type items;
};

template <typename dest_t>
class reverser_output_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	reverser_output_t(dest_t dest, const node_token & input_token)
		: dest(std::move(dest))
	{
		set_name("Serialization reverse reader");
		add_dependency(input_token);
		add_push_destination(this->dest);
		//TODO memory
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED);
	}

	void propagate() override {
		file = fetch<tpie::maybe<tpie::temp_file> *>("__srev_file");
		if (!file->is_constructed())
			throw tpie::exception("No one created my file");
		rd.open((*file)->path());
		this->set_steps(rd.size());
	}

	void go() override {
		item_type x;
		stream_size_type bytesRead = 0;
		while (rd.can_read()) {
			rd.unserialize(x);
			dest.push(x);

			stream_size_type bytesRead2 = rd.offset();
			step(bytesRead2 - bytesRead);
			bytesRead = bytesRead2;
		}
	}

	void end() override {
		rd.close();
		file->destruct();
	}
private:
	serialization_reverse_reader rd;
	tpie::maybe<tpie::temp_file> * file;
	dest_t dest;
};

template <typename T>
class reverser_pull_output_t : public node {
public:
	typedef T item_type;

	reverser_pull_output_t(const node_token & input_token)
		{
		set_name("Serialization reverse reader");
		add_dependency(input_token);
		//TODO memory
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED);
	}

	void propagate() override {
		file = fetch<tpie::maybe<tpie::temp_file> *>("__srev_file");
		if (!file->is_constructed())
			throw tpie::exception("No one created my file");
		rd.open((*file)->path());
		this->set_steps(rd.size());
	}

	bool can_pull() {
		return rd.can_read();
	}

	T pull() {
		item_type x;
		stream_size_type bytesRead = rd.offset();
		rd.unserialize(x);
		stream_size_type bytesRead2 = rd.offset();
		step(bytesRead2 - bytesRead);
		return x;
	}

	void end() override {
		rd.close();
		file->destruct();
	}

private:
	serialization_reverse_reader rd;
	tpie::maybe<tpie::temp_file> * file;
};

template <typename T>
class buffer_input_t : public node {
public:
	typedef T item_type;

	buffer_input_t(const node_token & token,
				   std::shared_ptr<node> output = std::shared_ptr<node>())
		: node(token)
		, output(output)
		, wr()
		, items(0) {
		set_name("Serialization buffer writer");
		//TODO memory
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
	}

	void propagate() override {
		file.construct();
		forward<tpie::maybe<tpie::temp_file>*>("__sbuf_file", &file, 1);
	}

	void begin() override {
		wr.open(file->path());
	}

	void push(const item_type & x) {
		wr.serialize(x);
		++items;
	}

	void end() override {
		wr.close();
		this->forward<stream_size_type>("items", items);
	}
public:
	std::shared_ptr<node> output;
	tpie::maybe<tpie::temp_file> file;
	serialization_writer wr;
	stream_size_type items;
};


template <typename dest_t>
class buffer_output_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	buffer_output_t(dest_t dest, const node_token & input_token)
		: dest(std::move(dest))
	{
		add_dependency(input_token);
		add_push_destination(this->dest);
		//TODO MEMORY
		set_minimum_resource_usage(FILES, 1);
		set_name("Serialization buffer reader");
		set_plot_options(PLOT_BUFFERED);
	}

	void propagate() override {
		file = fetch<tpie::maybe<tpie::temp_file> *>("__sbuf_file");
		if (!file->is_constructed())
			throw tpie::exception("No one created my file");

		rd.open((*file)->path());
		set_steps(rd.size());
	}

	void go() override {
		item_type x;
		stream_size_type bytesRead = 0;
		while (rd.can_read()) {
			rd.unserialize(x);
			dest.push(x);

			stream_size_type bytesRead2 = rd.offset();
			step(bytesRead2 - bytesRead);
			bytesRead = bytesRead2;
		}
	}

	void end() override {
		rd.close();
		file->destruct();
	}
private:
	tpie::maybe<tpie::temp_file> * file;
	serialization_reader rd;
	dest_t dest;
};

template <typename T>
class buffer_pull_output_t: public node {
public:
	typedef T item_type;

	buffer_pull_output_t(const node_token & input_token) {
		add_dependency(input_token);
		set_name("Fetching items", PRIORITY_SIGNIFICANT);
		//TODO memory
		set_minimum_resource_usage(FILES, 1);
		set_plot_options(PLOT_BUFFERED);
	}

	virtual void propagate() override {
		file = fetch<tpie::maybe<tpie::temp_file> *>("__sbuf_file");
		if (!file->is_constructed())
			throw tpie::exception("No one created my file");
		rd.open((*file)->path());
		set_steps(rd.size());
	}

	bool can_pull() {
		return rd.can_read();
	}

	T pull() {
		item_type x;
		stream_size_type bytesRead = rd.offset();
		rd.unserialize(x);
		stream_size_type bytesRead2 = rd.offset();
		step(bytesRead2 - bytesRead);
		return x;
	}

	void end() override {
		rd.close();
		file->destruct();
	}
private:
	tpie::maybe<tpie::temp_file> * file;
	serialization_reader rd;
};


} // namespace serialization_bits


///////////////////////////////////////////////////////////////////////////////
/// \brief A passive serialization reverser stored in external memory. Reverses the input
/// stream and creates a phase boundary.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class passive_serialization_reverser {
public:
	typedef T item_type;
	typedef serialization_bits::reverser_input_t<T> input_t;
	typedef serialization_bits::reverser_pull_output_t<T> output_t;
private:
	typedef termfactory<input_t,  const node_token &> inputfact_t;
	typedef termfactory<output_t, const node_token &> outputfact_t;
	typedef pipe_end<inputfact_t>  inputpipe_t;
	typedef pullpipe_begin<outputfact_t> outputpipe_t;
public:
	passive_serialization_reverser() {}

	input_t raw_input() {
		return input_t(input_token);
	}

	output_t raw_output() {
		return output_t(input_token);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns a termfactory for the input nodes
	///////////////////////////////////////////////////////////////////////////////
	inputpipe_t input() {
		return inputfact_t(input_token);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns a termfactory for the output nodes
	///////////////////////////////////////////////////////////////////////////////
	outputpipe_t output() {
		return outputfact_t(input_token);
	}
private:
	node_token input_token;

	passive_serialization_reverser(const passive_serialization_reverser &);
	passive_serialization_reverser & operator=(const passive_serialization_reverser &);
};


///////////////////////////////////////////////////////////////////////////////
/// \brief Serialization stream buffer. Does nothing to the item stream, but
/// it inserts a phase boundary.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class passive_serialization_buffer {
public:
	typedef T item_type;
	typedef serialization_bits::buffer_input_t<T> input_t;
	typedef serialization_bits::buffer_pull_output_t<T> output_t;
private:
	typedef termfactory<input_t,  const node_token &> inputfact_t;
	typedef termfactory<output_t, const node_token &> outputfact_t;
	typedef pipe_end      <inputfact_t>  inputpipe_t;
	typedef pullpipe_begin<outputfact_t> outputpipe_t;

public:
	passive_serialization_buffer() {}

	input_t raw_input() {
		return input_t(input_token);
	}

	output_t raw_output() {
		return output_t(input_token);
	}

	inputpipe_t input() {
		return inputfact_t(input_token);
	}

	outputpipe_t output() {
		return outputfact_t(input_token);
	}

private:
	node_token input_token;

	passive_serialization_buffer(const passive_serialization_buffer &);
	passive_serialization_buffer & operator=(const passive_serialization_buffer &);
};


///////////////////////////////////////////////////////////////////////////////
/// A pipelining node that reverses serializable items and creates a phase
/// boundary
///////////////////////////////////////////////////////////////////////////////
typedef pipe_middle<split_factory<serialization_bits::reverser_input_t, node, serialization_bits::reverser_output_t> > serialization_reverser;

///////////////////////////////////////////////////////////////////////////////
/// A pipelining node that acts as a buffer for serializable items and creates
/// a phase boundary
///////////////////////////////////////////////////////////////////////////////
typedef pipe_middle<split_factory<serialization_bits::buffer_input_t, node, serialization_bits::buffer_output_t> > serialization_buffer;

} // namespace pipelining

} // namespace tpie

#endif // TPIE_PIPELINING_SERIALIZATION_H
