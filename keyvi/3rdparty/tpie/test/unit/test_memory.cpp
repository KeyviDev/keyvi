// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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

#include "common.h"
#include <tpie/memory.h>
#include <vector>
#include <tpie/internal_queue.h>
#include <tpie/internal_vector.h>
#include <random>
#include <tpie/job.h>
#include <tpie/cpu_timer.h>

struct mtest {
	size_t & r;
	const size_t ve;
	mtest(const size_t & a, const size_t & b, size_t & c): r(c), ve(b) {r=a;}
	~mtest(){r=ve;}
};

struct mtesta {
	virtual ~mtesta() {}
};

struct mtestb: public mtesta {
	size_t & x;
	mtestb(size_t & _): x(_) {}
	virtual ~mtestb() {x=42;}
};

bool basic_test() {
	{
		//Test allocation and deallocation
		size_t a1 = tpie::get_memory_manager().used();
		
		size_t * x = tpie::tpie_new<size_t>(42);
		if (*x != 42) return false;
		size_t a2 = tpie::get_memory_manager().used();
		
		tpie::tpie_delete(x);
		x = 0;
		tpie::tpie_delete(x);
		
		size_t a3 = tpie::get_memory_manager().used();
		if (a2 != a1 + sizeof(size_t) || a1 != a3) return false;
	}

	{
		//Test arrayes
		size_t a1 = tpie::get_memory_manager().used();
		
		size_t * x = tpie::tpie_new_array<size_t>(1234);
		size_t a2 = tpie::get_memory_manager().used();
		tpie::tpie_delete_array(x, 1234);
		size_t a3 = tpie::get_memory_manager().used();
		if (a2 != a1 + sizeof(size_t)*1234 || a1 != a3) return false;
	}
	
	{
		//Test constructors with references and destructors
		size_t foo=1;
		mtest * x = tpie::tpie_new<mtest>(2, 3, foo);
		if (foo != 2) return false;
		tpie::tpie_delete(x);
		if (foo != 3) return false;
	}

	{
		//Test virtual destructors
		size_t a1 = tpie::get_memory_manager().used();
		size_t foo=1;
		mtesta * x = tpie::tpie_new<mtestb>(foo);
		if (tpie::tpie_size(x) != sizeof(mtestb) || tpie::get_memory_manager().used() != sizeof(mtestb) +  a1)
			return false;
		tpie::tpie_delete(x);
		size_t a2 = tpie::get_memory_manager().used();
		if (a1 != a2 || foo != 42) return false;
	}

	{
		//Test unique ptr
		size_t a1 = tpie::get_memory_manager().used();
		{
			tpie::unique_ptr<size_t> x(tpie::tpie_new<size_t>(32));
			tpie::unique_ptr<size_t> y = std::move(x);
			y.reset(tpie::tpie_new<size_t>(54));
		}
		size_t a2 = tpie::get_memory_manager().used();
		if (a1 != a2) return false;
	}

	{
		//Test auto ptr
		size_t a1 = tpie::get_memory_manager().used();
		{
			tpie::tpie_delete(tpie::tpie_new<std::ifstream>("tmp",std::ios::binary | std::ios::out));
		}
		size_t a2 = tpie::get_memory_manager().used();
		if (a1 != a2) return false;
	}

	{ 
		//Allocator
		size_t a1;
		{
			a1 = tpie::get_memory_manager().used();
			std::vector<size_t, tpie::allocator<size_t> > myvect;
			myvect.resize(16);
			for(size_t i=0; i < 12345; ++i) {
				if (a1 + myvect.capacity() * sizeof(size_t) != tpie::get_memory_manager().used()) return false;
				myvect.push_back(12);
			}
			for(size_t i=0; i < 12345; ++i) {
				if (a1 + myvect.capacity() * sizeof(size_t) != tpie::get_memory_manager().used()) return false;
				myvect.pop_back();
			}
		}
		if (tpie::get_memory_manager().used() != a1) return false;
	}

	return true;
}

struct tpie_alloc {
	template <typename T>
	static T * alloc() { return tpie::tpie_new<T>(); }
	template <typename T>
	static void dealloc(T * t) { tpie::tpie_delete(t); }
};

struct std_alloc {
	template <typename T>
	static T * alloc() { return new T; }
	template <typename T>
	static void dealloc(T * t) { delete t; }
};

struct c_alloc {
	template <typename T>
	static T * alloc() { return (T *) malloc(sizeof(T)); }
	template <typename T>
	static void dealloc(T * t) { free(t); }
};

template <typename Alloc>
class memory_user : public tpie::job {
	typedef int test_t;
	size_t times;
	tpie::internal_queue<test_t *> pointers;
	std::default_random_engine rnd;
	std::uniform_real_distribution<double> urnd;

public:
	memory_user(size_t times, size_t capacity) : times(times), pointers(capacity) {}

	virtual void operator()() override {
		for (size_t i = 0; i < times; ++i) {
			if (pointers.empty() ||
				(!pointers.full() && urnd(rnd) <= (cos(static_cast<double>(i) * 60.0 / static_cast<double>(pointers.size())) + 1.0)/2.0)) {

				pointers.push(Alloc::template alloc<test_t>());
			} else {
				Alloc::dealloc(pointers.front());
				pointers.pop();
			}
		}
		while (!pointers.empty()) {
			Alloc::dealloc(pointers.front());
			pointers.pop();
		}
	}
};

template <typename Alloc>
bool parallel_test(const size_t nJobs, const size_t times, const size_t capacity) {
	tpie::cpu_timer t;
	t.start();
	tpie::internal_vector<memory_user<Alloc> *> workers(nJobs);
	for (size_t i = 0; i < nJobs; ++i) {
		workers[i] = tpie::tpie_new<memory_user<Alloc> >(times, capacity);
		workers[i]->enqueue();
	}
	for (size_t i = 0; i < nJobs; ++i) {
		workers[i]->join();
		tpie::tpie_delete(workers[i]);
	}
	t.stop();
	tpie::log_info() << t << std::endl;
	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv, 128)
		.test(basic_test, "basic")
		.test(parallel_test<tpie_alloc>, "parallel",
			  "n", static_cast<size_t>(8),
			  "times", static_cast<size_t>(500000),
			  "capacity", static_cast<size_t>(50000))
		.test(parallel_test<std_alloc>, "parallel_stdnew",
			  "n", static_cast<size_t>(8),
			  "times", static_cast<size_t>(500000),
			  "capacity", static_cast<size_t>(50000))
		.test(parallel_test<c_alloc>, "parallel_malloc",
			  "n", static_cast<size_t>(8),
			  "times", static_cast<size_t>(500000),
			  "capacity", static_cast<size_t>(50000))
		;
}
