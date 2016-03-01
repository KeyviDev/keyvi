// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, 2015 The TPIE development team
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

#ifndef __TPIE_PIPELINING_AMI_GLUE_H__
#define __TPIE_PIPELINING_AMI_GLUE_H__

namespace tpie {

namespace pipelining {

namespace bits {

template <typename dest_t>
class ami_input_t : public node {
public:
	typedef typename dest_t::item_type item_type;
	ami_input_t(dest_t && dest, typename tpie::ami::stream<item_type> & stream)
		: stream(stream), dest(std::move(dest)) {}

	void propagate() override {
		forward("items", (stream_size_type)stream.stream_len());
		set_steps(stream.stream_len());
	}

	void begin() override {
		stream.seek(0);
	}

	void go() override {
		item_type *item;
		while (stream.read_item(&item) == tpie::ami::NO_ERROR) {
			dest.push(*item);
			step(1);
		}
	}

private:

	typename tpie::ami::stream<item_type> & stream;
	dest_t dest;
};

template <typename dest_t>
class ami_input_stack_t : public node {
public:
	typedef typename dest_t::item_type item_type;
	ami_input_stack_t(dest_t && dest, typename tpie::ami::stack<item_type> & stack)
		: stack(stack), dest(std::move(dest)) {}
	
	void propagate() override {
		forward("items", (stream_size_type) stack.size());
		set_steps(stack.size());
	}
	
	void go() override {
		const item_type *item;
		while (stack.pop(&item) == tpie::ami::NO_ERROR) {
			dest.push(*item);
			step(1);
		}
	}

private:

	typename tpie::ami::stack<item_type> & stack;
	dest_t dest;
};

template <typename T>
class ami_pull_input_stack_t : public node {
public:
	typedef T item_type;
	ami_pull_input_stack_t(tpie::ami::stack<item_type> & stack) 
		: stack(stack) {}
	
	void propagate() override {
		forward("items", (stream_size_type) stack.size());
		set_steps(stack.size());
	}

	bool can_pull() {
		return !stack.is_empty();
	}
	
	T pull() {
		const item_type *item;
		stack.pop(&item);
		return *item;
	}

private:

	typename tpie::ami::stack<item_type> & stack;
};

///////////////////////////////////////////////////////////////////////////////
/// \class ami_output_t
///
/// file_stream output terminator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class ami_output_t : public node {
public:
	typedef T item_type;

	inline ami_output_t(tpie::ami::stream<T> & stream) : stream(stream) {
		set_name("Write", PRIORITY_INSIGNIFICANT);
	}

	inline void push(const T & item) {
		stream.write_item(item);
	}
private:
	tpie::ami::stream<T> & stream;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that pushes the contents of the given ami::stream
/// to the next node in the pipeline.
/// \param input The ami::stream from which it pushes items
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pipe_begin<factory<bits::ami_input_t, tpie::ami::stream<T> &> > 
ami_input(tpie::ami::stream<T> & input) {
	return factory<bits::ami_input_t, tpie::ami::stream<T> &>(input);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that pushes the contents of the given ami::stack
/// to the next node in the pipeline.
/// \param input The ami::stack from which it pushes items
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pipe_begin<factory<bits::ami_input_stack_t, tpie::ami::stack<T> &> > 
ami_input_stack(tpie::ami::stack<T> & input) {
	return factory<bits::ami_input_stack_t, tpie::ami::stack<T> &>(input);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining pull-node that reads items from the given ami::stack
/// \param fs The ami::stack from which it reads items.
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pullpipe_begin<termfactory<bits::pull_input_t<T>, tpie::ami::stack<T> &> > 
ami_pull_input_stack(tpie::ami::stack<T> & fs) {
	return termfactory<bits::pull_input_t<T>, tpie::ami::stack<T> &>(fs);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that writes the pushed items to an ami stream.
/// \param fs The stream that items should be written to
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_end<termfactory<bits::ami_output_t<T>, tpie::ami::stream<T> &> > ami_output(tpie::ami::stream<T> & fs) {
	return termfactory<bits::ami_output_t<T>, tpie::ami::stream<T> &>(fs);
}

} // namespace pipelining

} // namespace tpie

#endif
