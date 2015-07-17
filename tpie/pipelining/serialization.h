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

	input_t(TPIE_TRANSFERABLE(dest_t) dest, serialization_reader * rd)
		: dest(TPIE_MOVE(dest))
		, rd(rd)
	{
		set_name("Serialization reader");
		add_push_destination(this->dest);
		set_minimum_memory(rd->memory_usage());
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

typedef factory_1<input_t, serialization_reader *> input_factory;


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
	}

	void push(const T & x) {
		wr->serialize(x);
	}
};

template <typename T>
struct output_factory {
	typedef termfactory_1<output_t<T>, serialization_writer *> type;
};

} // namespace serialization_bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that reads items from a \ref serialization_reader.
/// \param rd The reader from which which items should be read
///////////////////////////////////////////////////////////////////////////////
pipe_begin<serialization_bits::input_factory>
inline serialization_input(serialization_reader & rd) {
	return serialization_bits::input_factory(&rd);
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

template <typename> class rev_input_t;

template <typename dest_t>
class rev_output_t : public node {
	friend class rev_input_t<rev_output_t<dest_t> >;

	dest_t dest;
	tpie::temp_file * m_stack;

	serialization_reverse_reader rd;

public:
	typedef typename push_type<dest_t>::type item_type;

	rev_output_t(TPIE_TRANSFERABLE(dest_t) dest)
		: dest(TPIE_MOVE(dest))
		, m_stack(0)
	{
		this->set_name("Serialization reverse reader");
		this->add_push_destination(this->dest);
	}

	virtual void propagate() override {
		if (m_stack == 0)
			throw tpie::exception("No one created my stack");

		rd.open(m_stack->path());
		this->set_steps(rd.size());
	}

	virtual void go() override {
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

	virtual void end() override {
		rd.close();
		delete m_stack;
	}
};

typedef factory_0<rev_output_t> rev_output_factory;

template <typename dest_t>
class rev_input_t;

template <typename output_dest_t>
class rev_input_t<rev_output_t<output_dest_t> > : public node {
	typedef rev_output_t<output_dest_t> dest_t;
	dest_t dest;

	serialization_reverse_writer wr;
	stream_size_type items;

public:
	typedef typename push_type<dest_t>::type item_type;

	rev_input_t(TPIE_TRANSFERABLE(dest_t) dest)
		: dest(TPIE_MOVE(dest))
		, wr()
		, items(0)
	{
		this->set_name("Serialization reverse writer");
		this->dest.add_dependency(*this);
	}

	virtual void begin() override {
		dest.m_stack = new tpie::temp_file();
		wr.open(dest.m_stack->path());
	}

	void push(const item_type & x) {
		wr.serialize(x);
		++items;
	}

	virtual void end() override {
		wr.close();
		this->forward<stream_size_type>("items", items);
	}
};

typedef factory_0<rev_input_t> rev_input_factory;

typedef bits::pair_factory<rev_input_factory, rev_output_factory> reverse_factory;









template <typename> class buffer_input_t;

template <typename dest_t>
class buffer_output_t : public node {
	friend class buffer_input_t<buffer_output_t<dest_t> >;

	dest_t dest;
	tpie::temp_file * m_file;

	serialization_reader rd;

public:
	typedef typename push_type<dest_t>::type item_type;

	buffer_output_t(TPIE_TRANSFERABLE(dest_t) dest)
		: dest(TPIE_MOVE(dest))
		, m_file(0)
	{
		this->set_name("Serialization buffer reader");
	}

	virtual void propagate() override {
		if (m_file == 0)
			throw tpie::exception("No one created my file");

		rd.open(m_file->path());
		this->set_steps(rd.size());
	}

	virtual void go() override {
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

	virtual void end() override {
		rd.close();
		delete m_file;
	}
};

typedef factory_0<buffer_output_t> buffer_output_factory;

template <typename dest_t>
class buffer_input_t;

template <typename output_dest_t>
class buffer_input_t<buffer_output_t<output_dest_t> > : public node {
	typedef buffer_output_t<output_dest_t> dest_t;
	dest_t dest;

	serialization_writer wr;
	stream_size_type items;

public:
	typedef typename push_type<dest_t>::type item_type;

	buffer_input_t(TPIE_TRANSFERABLE(dest_t) dest)
		: dest(TPIE_MOVE(dest))
		, wr()
		, items(0)
	{
		this->set_name("Serialization buffer writer");
		this->dest.add_dependency(*this);
	}

	virtual void begin() override {
		dest.m_file = new tpie::temp_file();
		wr.open(dest.m_file->path());
	}

	void push(const item_type & x) {
		wr.serialize(x);
		++items;
	}

	virtual void end() override {
		wr.close();
		this->forward<stream_size_type>("items", items);
	}
};

typedef factory_0<buffer_input_t> buffer_input_factory;

typedef bits::pair_factory<buffer_input_factory, buffer_output_factory> buffer_factory;











} // namespace serialization_bits

///////////////////////////////////////////////////////////////////////////////
/// A pipelining node that reverses serializable items and creates a phase
/// boundary
///////////////////////////////////////////////////////////////////////////////
pipe_middle<serialization_bits::reverse_factory>
inline serialization_reverser() {
	serialization_bits::rev_input_factory i;
	serialization_bits::rev_output_factory o;
	return serialization_bits::reverse_factory(i, o);
}

///////////////////////////////////////////////////////////////////////////////
/// A pipelining node that acts as a buffer for serializable items and creates 
/// a phase boundary
///////////////////////////////////////////////////////////////////////////////
pipe_middle<serialization_bits::buffer_factory>
inline serialization_buffer() {
	serialization_bits::buffer_input_factory i;
	serialization_bits::buffer_output_factory o;
	return serialization_bits::buffer_factory(i, o);
}

} // namespace pipelining

} // namespace tpie

#endif // TPIE_PIPELINING_SERIALIZATION_H
