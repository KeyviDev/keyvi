// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, 2014 The TPIE development team
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


/* This is an example TPIE program.
 * It reads integers from standard in, removes duplicates, and writes them back out.
 */


#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>
#include <tpie/pipelining.h>
#include <limits>

#include <boost/filesystem.hpp> // boost::filesystem::remove
#include <tpie/prime.h> // next_prime

// Progress indicators
#include <tpie/progress_indicator_arrow.h>
#include <tpie/fractional_progress.h>

#include <string>

using namespace tpie::pipelining;

///////////////////////////////////////////////////////////////////////////////
/// An implementation of a node that accepts integers and assigns an index to
/// each integer. The integer and its index is then pushed as a
/// std::pair<int, int>
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class pair_item_number_augmenter_type : public node {
	dest_t dest;
	int i;
public:
	typedef int item_type; // the type of item that this node accepts.

	pair_item_number_augmenter_type(dest_t dest)
	: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("Pair item number augmenter");
	}

	virtual void begin() override {
		i = 0;
	}

	void push(item_type item) {
		dest.push(std::make_pair(item, i++)); // assign an index to each integer, starting at 0.
	}
};

typedef pipe_middle<factory<pair_item_number_augmenter_type> > pair_item_number_augmenter;

///////////////////////////////////////////////////////////////////////////////
/// This is an implementation of a node that will remove any consecutive
/// duplicates pushed to it.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class remove_duplicates_type : public node {
	dest_t dest;
	int last_value;
public:
	typedef std::pair<int, int> item_type;

	remove_duplicates_type(dest_t dest)
	: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("Remove duplicates");
	}

	virtual void begin() override {
		last_value = std::numeric_limits<int>::max();
	}

	void push(const item_type & item) {
		if(last_value != std::numeric_limits<int>::max() && item.first == last_value) // if first component of the last pair is equal to the current one do nothing
			return;

		// otherwise update last_value and push the pair to the destination node.
		last_value = item.first;
		dest.push(item);
	}
};

typedef pipe_middle<factory<remove_duplicates_type> > remove_duplicates;

///////////////////////////////////////////////////////////////////////////////
/// This node will accepts std::pair<int, int> and will push the first
/// component of the pair.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class pair_to_int_type : public node {
	dest_t dest;
public:
	typedef std::pair<int, int> item_type;

	pair_to_int_type(dest_t dest)
	: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("Remove line number");
	}

	void push(const item_type &item) {
		dest.push(item.first); // only push the first component of the pair.
	}
};

typedef pipe_middle<factory<pair_to_int_type> > pair_to_int;

///////////////////////////////////////////////////////////////////////////////
/// A comparator in order to sort std::pair<int, int> by the second component
///////////////////////////////////////////////////////////////////////////////
struct second_item_comparator {
	bool operator()(const std::pair<int, int> & a, const std::pair<int, int> & b) const {
		return a.second < b.second; // the second component is unique to each pair.
	}
};

int main() {
	tpie::tpie_init();

	// Calling tpie_finish() before the pipeline is destructed would result in a segmentation fault. A new scope is created to avoid this.
	{
	tpie::get_memory_manager().set_limit(50*1024*1024);

	pipeline p = scanf_ints() // Read integers from standard input
		| pair_item_number_augmenter() // An index is assigned to each node.
		| sort() // The nodes are then sorted by their first component.
		| remove_duplicates() // All duplicates are now consecutive items because of the sort node.
		| sort(second_item_comparator()) // now sort by the second component. The order is now the same as when the items were read from the input.
		| pair_to_int() // Remove the second component.
		| printf_ints(); // Print the integers.
	p();
	}

	tpie::tpie_finish();
}
