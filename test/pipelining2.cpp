// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2012, The TPIE development team
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

/**
 * Given a list of integers a0, a1, a2, ... an on standard input,
 * sort the numbers and add the ith and the (n-i)th number in the sorted order
 * and print the sums to standard out.
 */

#include <tpie/tpie.h>
#include <tpie/pipelining.h>
#include <random>
#include <tpie/file_stream.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <tpie/progress_indicator_arrow.h>

using namespace tpie;
using namespace tpie::pipelining;
using namespace std;

template <typename src_pipe_t>
class add_t {
	typedef typename src_pipe_t::factory_type src_fact_t;
	typedef typename src_fact_t::constructed_type src_t;

public:
	template <typename dest_t>
	class type : public node {
		dest_t dest;
		src_t src;
	public:
		typedef int item_type;

		type(dest_t dest, src_pipe_t srcpipe)
			: dest(std::move(dest))
			, src(srcpipe.factory.construct())
		{
			add_push_destination(dest);
			add_pull_source(src);
		}

		void push(int i) {
			dest.push(i+src.pull());
		}
	};
};

template <typename src_pipe_t>
pipe_middle<tempfactory<add_t<src_pipe_t>, src_pipe_t> >
add(src_pipe_t && srcpipe) {
	return tempfactory<add_t<src_pipe_t>, src_pipe_t>(std::forward<src_pipe_t>(srcpipe));
}

void go() {
	passive_buffer<int> buf;
	pipeline p
		= push_input_iterator(istream_iterator<int>(cin), istream_iterator<int>())
		| sort()             // sort the input
		| fork(buf.input())  // buffer the sorted items
		| reverser()         // reverse the items
		| add(buf.output())  // add the reversed and the buffered sequence
		| printf_ints();
	p.plot();
	p();
}

int main() {
	tpie_init();
	get_memory_manager().set_limit(50*1024*1024);
	go();
	tpie_finish();
	return 0;
}
