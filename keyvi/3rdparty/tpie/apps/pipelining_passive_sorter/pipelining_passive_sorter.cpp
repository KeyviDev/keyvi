// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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

/**
 * Generates n integers, adds one to each,
 * adds them pairwise in sorted order and prints them to stdout.
 */

#include <tpie/tpie.h>
#include <tpie/pipelining.h>

using namespace tpie;
using namespace tpie::pipelining;

// Generates 'n' integers in [0,100).
template <typename dest_t>
class Generator : public node {
public:
	typedef int item_type;

	Generator(dest_t dest, int n)
		: dest(std::move(dest))
		, n(n)
	{
		add_push_destination(dest);
		set_name("Generator");
	}

	virtual void go() override {
		for (int i = 0; i < n; ++i) {
			dest.push(rand()%100);
		}
	}

private:
	dest_t dest;
	int n;
};

typedef pipe_begin<factory<Generator, int> > generator;

// Increases each incoming value by 1.
template <typename dest_t>
class AddOne : public node {
public:
	typedef int item_type;

	AddOne(dest_t dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("AddOne");
	}

	void push(item_type a) {
		dest.push(a+1);
	}

private:
	dest_t dest;
};

typedef pipe_middle<factory<AddOne> > addOne;


template <typename source_t>
class AddPairwise {
public:
	template <typename dest_t>
	class type : public node {
	public:
		typedef int item_type;

		type(dest_t dest, source_t src)
			: dest(std::move(dest))
			, puller(src.construct())
		{
			add_push_destination(dest);
			add_pull_source(puller);
			set_name("AddPairwise");
		}

		virtual void go() override {
			while (puller.can_pull()) {
				// Pull two numbers a,b and push a+b to dest.
				int a = puller.pull();
				if (!puller.can_pull())
					throw std::logic_error("Not an even number of items in the stream.");
				int b = puller.pull();
				dest.push(a+b);
			}
		}

	private:
		dest_t dest;
		typedef typename source_t::constructed_type puller_t;
		puller_t puller;
	};
};

template <typename source_t>
inline pipe_begin<tempfactory<AddPairwise<source_t>, source_t> > addPairwise(source_t source) {
	return {std::move(source)};
}

void go() {
	int n = 100;

	// Generate pipeline that
	// 1) generates "n" numbers
	// 2) adds one to each number
	// 3) sorts the numbers
	// 4) computes the pairwise sum of the numbers
	// 5) prints the numbers to stdout.
	//
	// We use a passive_sorter because we take more than one element of the
	// sorter out at a time.
	passive_sorter<int> sorter;
	pipeline p1 = generator(n) | addOne() | sorter.input();
	pipeline p2 = addPairwise(sorter.output()) | printf_ints();

	// It doesn't matter if we start p1 or p2 here.
	p1.plot();
	p1();
}

int main() {
	tpie_init();
	get_memory_manager().set_limit(50*1024*1024);
	go();
	tpie_finish();
	return 0;
}
