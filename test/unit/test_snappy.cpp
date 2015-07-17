// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013 The TPIE development team
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

#ifdef TPIE_HAS_SNAPPY
#include <snappy.h>

bool basic_test() {
	std::string input = "Hello world!";
	std::string compressed;
	std::string output;
	snappy::Compress(input.data(), input.size(), &compressed);
	snappy::Uncompress(compressed.data(), compressed.size(), &output);
	return input == output;
}
#else // TPIE_HAS_SNAPPY

bool basic_test() {
	tpie::log_warning() << "ut-snappy: No snappy support built in!" << std::endl;
	return true;
}

#endif // TPIE_HAS_SNAPPY

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.test(basic_test, "basic")
	;
}
