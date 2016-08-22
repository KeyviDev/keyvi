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

#ifndef __TPIE_PIPELINING_STD_GLUE_H__
#define __TPIE_PIPELINING_STD_GLUE_H__

#include <vector>

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename dest_t, typename T, typename A>
class input_vector_t : public node {
public:
	typedef T item_type;

	input_vector_t(dest_t dest, const std::vector<T, A> & input) : dest(std::move(dest)), input(input) {
		add_push_destination(this->dest);
	}

	void propagate() override {
		forward("items", static_cast<stream_size_type>(input.size()));
		set_steps(input.size());
	}

	void go() override {
		for (auto & i: input) {
			dest.push(i);
			step();
		}
	}
private:
	dest_t dest;
	const std::vector<T, A> & input;
};

template <typename T, typename A>
class pull_input_vector_t : public node {
public:
	typedef T item_type;
	
	pull_input_vector_t(const std::vector<T, A> & input) : input(input) {}

	void propagate() override {
		forward("items", static_cast<stream_size_type>(input.size()));
	}

	void begin() override {idx=0;};
	bool can_pull() const {return idx < input.size();}
	const T & peek() const {return input[idx];}
	const T & pull() {return input[idx++];}
	
private:
	size_t idx;
	const std::vector<T, A> & input;
};


template <typename T, typename A>
class output_vector_t : public node {
public:
	typedef T item_type;

	output_vector_t(std::vector<T, A> & output) : output(output) {}

	void push(const T & item) {
		output.push_back(item);
	}
private:
	std::vector<item_type, A> & output;
};


template <typename F>
class lambda_t {
public:
	template <typename dest_t>
	class type: public node {
	public:
		typedef typename F::argument_type item_type;
		
		type(dest_t dest, const F & f): f(f), dest(std::move(dest)) {
		}
		
		void push(const item_type & item) {
			dest.push(f(item));
		}
	private:
		F f;
		dest_t dest;
	};
};

template <typename F>
class exclude_lambda_t {
public:
	template <typename dest_t>
	class type: public node {
	public:
		typedef typename F::argument_type item_type;
		
		type(dest_t dest, const F & f): f(f), dest(std::move(dest)) {
		}
		
		void push(const item_type & item) {
			typename F::result_type t=f(item);
			if (t.second) dest.push(t.first);
		}
	private:
		F f;
		dest_t dest;
	};
};


} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that pushes the contents of the given vector to the
/// next node in the pipeline.
/// \param input The vector from which it pushes items
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename A>
inline pipe_begin<tfactory<bits::input_vector_t, Args<T, A>, const std::vector<T, A> &> > input_vector(const std::vector<T, A> & input) {
	return {input};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that pushes the contents of the given vector to the
/// next node in the pipeline.
/// \param input The vector from which it pushes items
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename A>
inline pullpipe_begin<termfactory<bits::pull_input_vector_t<T, A>, const std::vector<T, A> &> > pull_input_vector(const std::vector<T, A> & input) {
	return {input};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining node that pushes items to the given vector.
/// \param output The vector to push items to
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename A>
inline pipe_end<termfactory<bits::output_vector_t<T, A>, std::vector<T, A> &> > output_vector(std::vector<T, A> & output) {
	return termfactory<bits::output_vector_t<T, A>, std::vector<T, A> &>(output);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that applies to given functor to items in
/// the stream. The functor should have a typedef named argument_type
/// that is the type of the argument given to the call operator.
/// \param f The functor that should be applied to items
///////////////////////////////////////////////////////////////////////////////
template <typename F>
inline pipe_middle<tempfactory<bits::lambda_t<F>, F> > lambda(const F & f) {
	return tempfactory<bits::lambda_t<F>, F>(f);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that applies to given functor to items in
/// the stream. The functor should have a typedef named argument_type
/// that is the type of the argument given to the call operator. It is required
/// that the functor returns a pair. The first item should be a boolean
/// indicating whether the item should be pushed to the next node. The second
/// should be the value itself.
/// \param f The functor that should be applied to items
///////////////////////////////////////////////////////////////////////////////
template <typename F>
inline pipe_middle<tempfactory<bits::exclude_lambda_t<F>, F> > exclude_lambda(const F & f) {
	return tempfactory<bits::exclude_lambda_t<F>, F>(f);
}


} // namespace pipelining

} // namespace tpie

#endif
