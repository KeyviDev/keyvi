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

#ifndef __TPIE_PIPELINING_FACTORY_HELPERS_H__
#define __TPIE_PIPELINING_FACTORY_HELPERS_H__

#include <tpie/pipelining/container.h>
#include <tpie/pipelining/factory_base.h>
#include <tpie/pipelining/node.h>

namespace tpie {
namespace pipelining {

///////////////////////////////////////////////////////////////////////////////
/// \class factory
/// Node factory for variadic argument generators.
///////////////////////////////////////////////////////////////////////////////
template <template <typename dest_t> class R, typename... T>
class factory : public factory_base {
public:
	factory(const factory &) = delete;
	factory(factory &&) = default;
	factory & operator=(const factory &) = delete;
	factory & operator=(factory &&) = default;

	template <typename... Args>
	factory(Args && ... v) : cont(std::forward<Args>(v)...) {}

	template<typename dest_t>
	struct constructed {
		typedef R<typename bits::remove<dest_t>::type> type;
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct(dest_t && dest) {
		node_token tok = dest.get_token();
		typename constructed<dest_t>::type r = container_construct<typename constructed<dest_t>::type>(cont, std::forward<dest_t>(dest));
		this->init_node(r);
		this->add_default_edge(r, tok);
		this->add_node_set_edges(r);
		return r;
	}

	template <typename dest_t>
	typename constructed<dest_t>::type construct_copy(dest_t && dest) {
		node_token tok = dest.get_token();
		typename constructed<dest_t>::type r = container_construct_copy<typename constructed<dest_t>::type>(cont, std::forward<dest_t>(dest));
		this->init_node(r);
		this->add_default_edge(r, tok);
		this->add_node_set_edges(r);
		return r;
	}

private:
	container<T...> cont;
};

///////////////////////////////////////////////////////////////////////////////
/// \class tempfactory
/// Node factory for variadic argument templated generators.
///////////////////////////////////////////////////////////////////////////////
template <typename Holder, typename... T>
class tempfactory : public factory_base {
public:
	tempfactory(const tempfactory & o) = delete;
	tempfactory(tempfactory && o) = default;
	tempfactory & operator=(const tempfactory & o) = delete;
	tempfactory & operator=(tempfactory && o) = default;

	template <typename... Args>
	tempfactory(Args && ... v) : cont(std::forward<Args>(v)...) {}

	template<typename dest_t>
	struct constructed {
		typedef typename Holder::template type<typename bits::remove<dest_t>::type> type;
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct(dest_t && dest) {
		node_token tok = dest.get_token();
		typename constructed<dest_t>::type r = container_construct<typename constructed<dest_t>::type>(cont, std::forward<dest_t>(dest));
		this->init_node(r);
		this->add_default_edge(r, tok);
		this->add_node_set_edges(r);
		return r;
	}

	template <typename dest_t>
	typename constructed<dest_t>::type construct_copy(dest_t && dest) {
		node_token tok = dest.get_token();
		typename constructed<dest_t>::type r = container_construct_copy<typename constructed<dest_t>::type>(cont, std::forward<dest_t>(dest));
		this->init_node(r);
		this->add_default_edge(r, tok);
		this->add_node_set_edges(r);
		return r;
	}
private:
	container<T...> cont;
};

///////////////////////////////////////////////////////////////////////////////
/// \class termfactory
/// Node factory for variadic argument terminators.
///////////////////////////////////////////////////////////////////////////////
template <typename R, typename... T>
class termfactory : public factory_base {
public:
	typedef R constructed_type;

	termfactory(const termfactory & o) = delete;
	termfactory(termfactory && o) = default;
	termfactory & operator=(const termfactory & o) = delete;
	termfactory & operator=(termfactory && o) = default;

	template<typename... Args>
	termfactory(Args && ... v) : cont(std::forward<Args>(v)...) {}

	R construct() {
		R r = container_construct<R>(cont);
		this->init_node(r);
		this->add_node_set_edges(r);
		return r;
	}

	R construct_copy() {
		R r = container_construct_copy<R>(cont);
		this->init_node(r);
		this->add_node_set_edges(r);
		return r;
	}
private:
	container<T...> cont;
};


template <typename ...>
class Args;

///////////////////////////////////////////////////////////////////////////////
/// \class tfactory
/// Node factory for variadic argument terminators.
///////////////////////////////////////////////////////////////////////////////
template <template <typename dest_t, typename ... X> class R, typename Args, typename... T>
class tfactory {/*We should never use this*/};

template <template <typename dest_t, typename ... X> class R,
		  typename ...TT, typename... T>
class tfactory<R, Args<TT...>, T...> : public factory_base {
public:
	tfactory(const tfactory & o) = delete;
	tfactory(tfactory && o) = default;
	tfactory & operator=(const tfactory & o) = delete;
	tfactory & operator=(tfactory && o) = default;

	template<typename... Args>
	tfactory(Args && ... v) : cont(std::forward<Args>(v)...) {}

	template<typename dest_t>
	struct constructed {
		typedef R<typename bits::remove<dest_t>::type, TT...> type;
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct(dest_t && dest) {
		node_token tok = dest.get_token();
		typename constructed<dest_t>::type r = container_construct<typename constructed<dest_t>::type>(cont, std::forward<dest_t>(dest));
		this->init_node(r);
		this->add_default_edge(r, tok);
		this->add_node_set_edges(r);
		return r;
	}

	template <typename dest_t>
	typename constructed<dest_t>::type construct_copy(dest_t && dest) {
		node_token tok = dest.get_token();
		typename constructed<dest_t>::type r = container_construct_copy<typename constructed<dest_t>::type>(cont, std::forward<dest_t>(dest));
		this->init_node(r);
		this->add_default_edge(r, tok);
		this->add_node_set_edges(r);
		return r;
	}
private:
	container<T...> cont;
};


///////////////////////////////////////////////////////////////////////////////
/// Node factory for split nodes, typically used for phase boundary nodes.
/// \tparam I the type of the input node
/// \tparam item_type the type of item used by both nodes
/// \tparam OB the base class for both input and output node. Typically
/// \ref tpie::pipelining::node
/// \tparam O the type of the output node
///////////////////////////////////////////////////////////////////////////////
template <template <typename item_type> class I, typename OB, template<typename dest_t> class O>
class split_factory : public factory_base {
public:
	template <typename dest_t>
	struct constructed {
		typedef typename push_type<dest_t>::type item_type;
		typedef I<item_type> type;
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct(dest_t && dest) const {
		node_token input_token;
		typedef typename push_type<dest_t>::type item_type;
		std::shared_ptr<OB> o = std::make_shared<O<dest_t> >(std::forward<dest_t>(dest), input_token);
		return I<item_type>(input_token, std::move(o));
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct_copy(dest_t && dest) const {
		return construct(std::forward<dest_t>(dest));
	};
};

} // namespace pipelining
} // namespace tpie

#endif // __TPIE_PIPELINING_FACTORY_HELPERS_H__
