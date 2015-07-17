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

#include <tpie/pipelining/factory_base.h>
#include <tpie/pipelining/node.h>
#ifdef TPIE_VARIADIC_FACTORIES
#include <tuple>
#endif // TPIE_VARIADIC_FACTORIES

namespace tpie {
namespace pipelining {

#ifdef TPIE_VARIADIC_FACTORIES

///////////////////////////////////////////////////////////////////////////////
/// \class factory
/// Node factory for variadic argument generators.
///////////////////////////////////////////////////////////////////////////////
template <template <typename dest_t> class R, typename... T>
class factory : public factory_base {
public:
	factory(T... v) : m_v(v...) {}

	template<typename dest_t>
	struct constructed {
		typedef R<typename bits::remove<dest_t>::type> type;
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct(dest_t && dest) const {
		return invoker<sizeof...(T)>::go(std::forward<dest_t>(dest), *this);
	}

private:
	std::tuple<T...> m_v;

	template <size_t N, size_t... S>
	class invoker {
	public:
		template <typename dest_t>
		static typename constructed<dest_t>::type
		go(dest_t && dest, const factory & parent) {
			return invoker<N-1, N-1, S...>::go(std::forward<dest_t>(dest), parent);
		}
	};

	template <size_t... S>
	class invoker<0, S...> {
	public:
		template <typename dest_t>
		static typename constructed<dest_t>::type
		go(dest_t && dest, const factory & parent) {
			node_token tok = dest.get_token();
			typename constructed<dest_t>::type
				r(std::forward<dest_t>(dest), std::get<S>(parent.m_v)...);
			parent.init_node(r);
			parent.add_default_edge(r, tok);
			parent.add_node_set_edges(r);
			return r;
		}
	};

	template <size_t N, size_t... S>
	friend class invoker;
};

///////////////////////////////////////////////////////////////////////////////
/// \class tempfactory
/// Node factory for variadic argument templated generators.
///////////////////////////////////////////////////////////////////////////////
template <typename Holder, typename... T>
class tempfactory : public factory_base {
public:
	tempfactory(T... v) : m_v(v...) {}

	template<typename dest_t>
	struct constructed {
		typedef typename Holder::template type<typename bits::remove<dest_t>::type> type;
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct(dest_t && dest) const {
		return invoker<sizeof...(T)>::go(std::forward<dest_t>(dest), *this);
	}

private:
	std::tuple<T...> m_v;

	template <size_t N, size_t... S>
	class invoker {
	public:
		template <typename dest_t>
		static typename constructed<dest_t>::type go(dest_t && dest, const tempfactory & parent) {
			return invoker<N-1, N-1, S...>::go(std::forward<dest_t>(dest), parent);
		}
	};

	template <size_t... S>
	class invoker<0, S...> {
	public:
		template <typename dest_t>
		static typename constructed<dest_t>::type go(dest_t && dest, const tempfactory & parent) {
			node_token tok = dest.get_token();
			typename constructed<dest_t>::type r(std::forward<dest_t>(dest), std::get<S>(parent.m_v)...);
			parent.init_node(r);
			parent.add_default_edge(r, tok);
			parent.add_node_set_edges(r);
			return r;
		}
	};

	template <size_t N, size_t... S>
	friend class invoker;
};

///////////////////////////////////////////////////////////////////////////////
/// \class termfactory
/// Node factory for variadic argument terminators.
///////////////////////////////////////////////////////////////////////////////
template <typename R, typename... T>
class termfactory : public factory_base {
public:
	typedef R constructed_type;

	termfactory(T... v) : m_v(v...) {}

	inline R construct() const {
		return invoker<sizeof...(T)>::go(*this);
	}

private:
	std::tuple<T...> m_v;

	template <size_t N, size_t... S>
	class invoker {
	public:
		static R go(const termfactory & parent) {
			return invoker<N-1, N-1, S...>::go(parent);
		}
	};

	template <size_t... S>
	class invoker<0, S...> {
	public:
		static R go(const termfactory & parent) {
			R r(std::get<S>(parent.m_v)...);
			parent.init_node(r);
			parent.add_node_set_edges(r);
			return r;
		}
	};

	template <size_t N, size_t... S>
	friend class invoker;
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
		boost::shared_ptr<OB> o(new O<dest_t>(std::forward<dest_t>(dest), input_token));
		return I<item_type>(input_token, std::move(o));
	};
};

#else

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
	typename constructed<dest_t>::type construct(TPIE_TRANSFERABLE(dest_t) dest) const {
		node_token input_token;
		typedef typename push_type<dest_t>::type item_type;
		boost::shared_ptr<OB> o(new O<dest_t>(TPIE_MOVE(dest), input_token));
		return I<item_type>(input_token, o);
	};
};

#endif

///////////////////////////////////////////////////////////////////////////////
// Legacy typedefs
///////////////////////////////////////////////////////////////////////////////

#if defined(TPIE_CPP_TEMPLATE_ALIAS) && defined(TPIE_VARIADIC_FACTORIES)

template <template <typename dest_t> class R>
using factory_0 = factory<R>;
template <template <typename dest_t> class R, typename T1>
using factory_1 = factory<R, T1>;
template <template <typename dest_t> class R, typename T1, typename T2>
using factory_2 = factory<R, T1, T2>;
template <template <typename dest_t> class R, typename T1, typename T2, typename T3>
using factory_3 = factory<R, T1, T2, T3>;
template <template <typename dest_t> class R, typename T1, typename T2, typename T3, typename T4>
using factory_4 = factory<R, T1, T2, T3, T4>;
template <template <typename dest_t> class R, typename T1, typename T2, typename T3, typename T4, typename T5>
using factory_5 = factory<R, T1, T2, T3, T4, T5>;
template <template <typename dest_t> class R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
using factory_6 = factory<R, T1, T2, T3, T4, T5, T6>;

template <typename Holder>
using tempfactory_0 = tempfactory<Holder>;
template <typename Holder, typename T1>
using tempfactory_1 = tempfactory<Holder, T1>;
template <typename Holder, typename T1, typename T2>
using tempfactory_2 = tempfactory<Holder, T1, T2>;
template <typename Holder, typename T1, typename T2, typename T3>
using tempfactory_3 = tempfactory<Holder, T1, T2, T3>;
template <typename Holder, typename T1, typename T2, typename T3, typename T4>
using tempfactory_4 = tempfactory<Holder, T1, T2, T3, T4>;
template <typename Holder, typename T1, typename T2, typename T3, typename T4, typename T5>
using tempfactory_5 = tempfactory<Holder, T1, T2, T3, T4, T5>;
template <typename Holder, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
using tempfactory_6 = tempfactory<Holder, T1, T2, T3, T4, T5, T6>;

template <typename R>
using termfactory_0 = termfactory<R>;
template <typename R, typename T1>
using termfactory_1 = termfactory<R, T1>;
template <typename R, typename T1, typename T2>
using termfactory_2 = termfactory<R, T1, T2>;
template <typename R, typename T1, typename T2, typename T3>
using termfactory_3 = termfactory<R, T1, T2, T3>;
template <typename R, typename T1, typename T2, typename T3, typename T4>
using termfactory_4 = termfactory<R, T1, T2, T3, T4>;
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
using termfactory_5 = termfactory<R, T1, T2, T3, T4, T5>;
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
using termfactory_6 = termfactory<R, T1, T2, T3, T4, T5, T6>;

#else

#ifndef DOXYGEN
#include <tpie/pipelining/factory_helpers.inl>
#endif

#endif

} // namespace pipelining
} // namespace tpie

#endif // __TPIE_PIPELINING_FACTORY_HELPERS_H__
