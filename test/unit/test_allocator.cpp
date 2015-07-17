// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
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
#include "../app_config.h"
#include "common.h"
#include <list>
#include <queue>
#include <tpie/memory.h>

using namespace tpie;

template <template <typename T, typename Allocator> class Container>
bool allocator_test() {
	memory_size_type m1, m2, m3;
	m1 = get_memory_manager().used();
	typedef int test_t;
	const test_t N = 100;
	{
		Container<test_t, allocator<test_t> > d;
		for (test_t i = 0; i < N; ++i) {
			d.push_back(i);
		}
		m2 = get_memory_manager().used();
	}
	m3 = get_memory_manager().used();
	if (m1 != m3) {
		log_error() << "Memory leak" << std::endl;
	}
	log_info() << "Pushing " << N << " numbers of size " << sizeof(test_t) << " to a " << typeid(Container<test_t, allocator<test_t> >).name() << " uses " << m2-m1 << std::endl;
	return m1 != m2;
}

int main(int argc, char ** argv) {
	return tests(argc, argv)
		.test(allocator_test<std::deque>, "deque")
		.test(allocator_test<std::list>, "list")
		;
}
