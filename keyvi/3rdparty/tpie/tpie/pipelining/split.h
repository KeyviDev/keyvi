// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2016, The TPIE development team
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

#ifndef TPIE_PIPELINING_SPLIT_H
#define TPIE_PIPELINING_SPLIT_H

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/pipe_base.h>

namespace tpie {
namespace pipelining {

///////////////////////////////////////////////////////////////////////////////
/// \brief Split one push streams into multiple.
///
/// A single pipe can push into \c sink(), and multiple pipes
/// can be constructed using \c source(). Every element pushed into \c sink()
/// will be pushed into every \c source().
///
/// \tparam T The type of item pushed
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class split {
public:
	class source_base : public node {
	public:
		source_base() = default;
		source_base(source_base &&) = default;
		
		virtual void push(const T & v) = 0;

	protected:
		~source_base() {}
	};

	template <typename dest_t>
	class source_impl : public source_base {
	public:
		source_impl(dest_t dest, node_token sink_token, std::vector<source_base *> & the_sources)
			: the_sources(the_sources)
			, dest(std::move(dest))
		{
			this->set_name("Split source", PRIORITY_INSIGNIFICANT);
			this->add_push_destination(this->dest);

			this->get_node_map()->union_set(sink_token.get_map());
			bits::node_map::ptr m = this->get_node_map()->find_authority();
			m->add_relation(sink_token.id(), this->get_token().id(), bits::pushes);
		}

		source_impl(source_impl &&) = default;

		virtual void prepare() override {
			the_sources.push_back(this);
		};

		virtual void push(const T & v) override {
			dest.push(v);
		}

	private:
		std::vector<source_base *> & the_sources;
		dest_t dest;
	};

	pipe_begin<factory<source_impl, node_token, std::vector<source_base *> &> > source() {
		return {sink_token, the_sources};
	}

	class sink_impl : public node {
	public:
		typedef T item_type;

		sink_impl(node_token sink_token, std::vector<source_base *> & the_sources)
			: node(sink_token), the_sources(the_sources)
		{
			set_name("Join sink", PRIORITY_INSIGNIFICANT);
		}

		void push(const T & v) {
			for (auto & source : the_sources)
				source->push(v);
		}

	private:
		std::vector<source_base *> & the_sources;
	};

	pipe_end<termfactory<sink_impl, node_token, std::vector<source_base *>&> > sink() {
		return {sink_token, the_sources};
	}

private:
	std::vector<source_base *> the_sources;
	node_token sink_token;
};

} // namespace pipelining
} // namespace tpie

#endif // TPIE_PIPELINING_SPLIT_H
