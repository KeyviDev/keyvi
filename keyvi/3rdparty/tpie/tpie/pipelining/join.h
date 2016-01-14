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

#ifndef TPIE_PIPELINING_JOIN_H
#define TPIE_PIPELINING_JOIN_H

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/pipe_base.h>

namespace tpie {
namespace pipelining {

///////////////////////////////////////////////////////////////////////////////
/// \brief Joins multiple push streams into one.
///
/// Multiple pipes can push into \c sink(), and a single pipe
/// can be constructed using \c source().
///
/// \tparam T The type of item pushed
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class join {
public:
	class source_base : public node {
	public:
		source_base(node_token token)
			: node(token)
		{
		}

		source_base(source_base &&) = default;
		
		virtual void push(const T & v) = 0;

	protected:
		~source_base() {}
	};

	template <typename dest_t>
	class source_impl : public source_base {
	public:
		source_impl(dest_t dest, node_token token, source_base ** the_source)
			: source_base(token)
			, the_source(the_source)
			, dest(std::move(dest))
		{
			this->set_name("Join source", PRIORITY_INSIGNIFICANT);
			this->add_push_destination(this->dest);
		}

		source_impl(source_impl &&) = default;

		virtual void prepare() override {
			if (*the_source != NULL && *the_source != this) {
				// If join.source() is used twice, the second construction of node()
				// should fail since the node_token is already used.
				// Thus, this exception should never be thrown.
				throw exception("Attempted to set join source a second time");
			}
			*the_source = this;
		};

		virtual void push(const T & v) override {
			dest.push(v);
		}

	private:
		source_base ** the_source;
		dest_t dest;
	};

	pipe_begin<factory<source_impl, node_token, source_base **> > source() {
		return factory<source_impl, node_token, source_base **>(source_token, &the_source);
	}

	class sink_impl : public node {
	public:
		typedef T item_type;

		sink_impl(node_token source_token, source_base ** the_source)
			: the_source(the_source)
		{
			set_name("Join sink", PRIORITY_INSIGNIFICANT);
			add_push_destination(source_token);
		}

		virtual void begin() override {
			the_source_cache = *the_source;
		}

		void push(const T & v) {
			the_source_cache->push(v);
		}

	private:
		source_base * the_source_cache;
		source_base ** the_source;
	};

	pipe_end<termfactory<sink_impl, node_token, source_base **> > sink() {
		return termfactory<sink_impl, node_token, source_base **>(source_token, &the_source);
	}

	join() : the_source(NULL) {}
private:
	source_base * the_source;
	node_token source_token;
};

} // namespace pipelining
} // namespace tpie

#endif // TPIE_PIPELINING_JOIN_H
