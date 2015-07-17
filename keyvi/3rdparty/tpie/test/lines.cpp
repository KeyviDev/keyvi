// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino=(0 :
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

#include <iostream>

#include <tpie/tpie.h>
#include <tpie/serialization_sorter.h>
#include <tpie/serialization_stream.h>

#include "../doc/code/serialization.inl"

int main(int argc, char ** argv) {
	std::string arg = (argc < 3) ? "" : argv[1];
	std::string filename = argv[2];
	tpie::tpie_init();
	if (arg == "read") {
		read_lines(std::cout, filename);
	} else if (arg == "write") {
		write_lines(std::cin, filename);
	} else if (arg == "reverse") {
		reverse_lines(filename);
	} else if (arg == "sort") {
		sort_lines(filename);
	} else {
		std::cerr << "Usage: " << argv[0] << " <read|write|reverse|sort> <filename>\n";
		return 1;
	}
	tpie::tpie_finish();
	return 0;
}
