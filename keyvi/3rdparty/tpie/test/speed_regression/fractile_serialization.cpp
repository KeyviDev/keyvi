// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#include <tpie/serialization_stream.h>
#include <tpie/serialization_sorter.h>
#include "fractile.h"

class fractile_serialization {
	tpie::serialization_sorter<item_type, std::less<item_type> > sorter;
	tpie::stream_size_type items;

public:
	fractile_serialization()
		: sorter()
	{
		tpie::memory_size_type mem =
			tpie::get_memory_manager().available()
			- tpie::serialization_reader::memory_usage();
		sorter.set_available_memory(mem);
	}

	void open(std::string path) {
		tpie::serialization_reader input;
		input.open(path);
		sorter.begin();
		items = 0;
		item_type x;
		while (input.can_read()) {
			input.unserialize(x);
			sorter.push(x);
			++items;
		}
		input.close();
		sorter.end();
		sorter.merge_runs();
	}
	void close() {
	}
	item_type read() {
		return sorter.pull();
	}
	tpie::stream_size_type size() {
		return items;
	}
};

int main(int argc, const char ** argv) {
	return fractile_main<fractile_serialization>(argc, argv);
}
