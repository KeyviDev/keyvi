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

#ifndef __TPIE_PIPELINING_STDIO_H__
#define __TPIE_PIPELINING_STDIO_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <cstdio>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename dest_t>
class scanf_ints_t : public node {
public:
	typedef int item_type;

	inline scanf_ints_t(dest_t dest) : dest(std::move(dest)) {
		add_push_destination(this->dest);
	}

	virtual void go() override {
		int in;
		while (scanf("%d", &in) == 1) {
			dest.push(in);
		}
	}

private:
	dest_t dest;
};

class printf_ints_t : public node {
public:
	typedef int item_type;

	inline printf_ints_t() {
	}

	inline void push(item_type i) {
		printf("%d\n", i);
	}
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that pushes the integers it reads using scanf
///////////////////////////////////////////////////////////////////////////////
typedef pipe_begin<factory<bits::scanf_ints_t> > scanf_ints;

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that prints the items that are pushed to it.
///////////////////////////////////////////////////////////////////////////////
typedef pipe_end<termfactory<bits::printf_ints_t> > printf_ints;

} // namespace pipelining

} // namespace tpie

#endif
