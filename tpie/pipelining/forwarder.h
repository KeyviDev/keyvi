// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2015 The TPIE development team
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

#ifndef __TPIE_PIPELINING_FORWARDER_H__
#define __TPIE_PIPELINING_FORWARDER_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/node_name.h>

namespace tpie {
namespace pipelining {
namespace bits {

template <typename dest_t>
class Forwarder: public node {
public:
	typedef typename push_type<dest_t>::type item_type;
	typedef std::vector<std::pair<std::string, boost::any> > values_t;
	
	Forwarder(dest_t dest, values_t values)
		: values(std::move(values)), dest(std::move(dest)) {}

	void prepare() override {
		for (typename values_t::iterator i=values.begin(); i != values.end(); ++i)
			forward_any(i->first, i->second);
	}

	template <typename T>
	void push(T && t) {dest.push(std::forward<T>(t));}
private:
	std::vector<std::pair<std::string, boost::any> > values;
	dest_t dest;
};

} //namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that will forward values on prepare, and
// pass though items on push
///////////////////////////////////////////////////////////////////////////////
inline pipe_middle<factory<bits::Forwarder, std::vector<std::pair<std::string, boost::any> > > > forwarder(std::vector<std::pair<std::string, boost::any> > items) {
	return factory<bits::Forwarder, std::vector<std::pair<std::string, boost::any> >  >(std::move(items));
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that will forward value on prepare, and
// pass though items on push
///////////////////////////////////////////////////////////////////////////////
template <typename VT>
pipe_middle<factory<bits::Forwarder, std::vector<std::pair<std::string, boost::any> > > > forwarder(std::string name, VT value) {
	std::vector<std::pair<std::string, boost::any> > v;
	v.push_back(std::make_pair(name, boost::any(value)));
	return forwarder(std::move(v));
}

} //namespace pipelining
} //namespace terrastream

#endif //__TPIE_PIPELINING_FORWARDER_H__

