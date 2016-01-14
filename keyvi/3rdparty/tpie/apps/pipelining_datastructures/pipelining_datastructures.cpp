// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE. If not, see <http://www.gnu.org/licenses/>
/* This is an example TPIE program.
*/

#include <tpie/tpie.h>
#include <tpie/sort.h>
#include <tpie/pipelining.h>
#include <set>

using namespace tpie;
using namespace tpie::pipelining;

///////////////////////////////////////////////////////////////////////////////
/// An implementation of a node that generates integers in the range [1;100]
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class generator_type : public node {
public:
	static const memory_size_type count = 100;
	generator_type(dest_t dest)
	: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("Generate integers");
	}
	virtual void go() override {
		for(memory_size_type i = 1; i <= count; ++i)
			dest.push(i);
	}
private:
	dest_t dest;
};

typedef pipe_begin<factory<generator_type> > generator;

///////////////////////////////////////////////////////////////////////////////
/// An implementation of a node that store the 20 largest items in a set
/// using pipelining datastructures
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class set_filler_type : public node {
	dest_t dest;
public:
	typedef int item_type;
	static const memory_size_type set_size = 20;

	set_filler_type(dest_t dest)
	: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("Fill set");
	}

	virtual void prepare() override {
		// register the usage of the datastructure and set the memory requirements.
		register_datastructure_usage("item_set");

		memory_size_type limit = sizeof(int) * set_size;
		set_datastructure_memory_limits("item_set", limit, limit);
	}

	virtual void propagate() override {
		// set the datastructure that was previously registered.
		m_set = tpie_new<std::multiset<int> >();
		set_datastructure("item_set", m_set);
	}

	void push(int item) {
		// insert the item into the set and delete the now smallest item from the set.
		m_set->insert(item);
		if(m_set->size() > set_size)	
			m_set->erase(m_set->begin());

		// push the item to the next node.
		dest.push(item);
	}
private:
	std::multiset<int> * m_set;
};

typedef pipe_middle<factory<set_filler_type> > set_filler;

///////////////////////////////////////////////////////////////////////////////
/// An implementation of a node that calculates the sum of differences to the 
/// 2000 largest items using pipelining datastructures
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class sum_differences_type : public node {
	dest_t dest;
public:
	typedef int item_type;

	sum_differences_type(dest_t dest)
	: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("Sum differences");
	}

	virtual void prepare() override {
		// register the usage of the datastructure
		// the memory requirements have already been set for this datastructure in another node
		register_datastructure_usage("item_set");
	}

	virtual void propagate() override {
		m_set = get_datastructure<std::multiset<int> *>("item_set");
	}

	void push(int item) {
		int difference = 0;
		for(std::multiset<int>::iterator i = m_set->begin(); i != m_set->end(); ++i) {
			difference += (*i - item);
		}
		dest.push(difference);
	}

	virtual void end() override {
		tpie_delete(m_set);
	}
private:
	std::multiset<int> * m_set;
};

typedef pipe_middle<factory<sum_differences_type> > sum_differences;

int main() {
	tpie::tpie_init();

	// Calling tpie_finish() before the pipeline is destructed would result in a segmentation fault. A new scope is created to avoid this.
	{

	tpie::get_memory_manager().set_limit(50*1024*1024);

	pipeline p = generator()
		| set_filler()
		| sort()
		| sum_differences()
		| printf_ints();
	p();

	}

	tpie::tpie_finish();
}
