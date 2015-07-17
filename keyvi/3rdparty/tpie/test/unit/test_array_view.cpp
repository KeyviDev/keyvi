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

#include "common.h"

#include <tpie/array_view.h>

using namespace tpie;

// Method coverage of tpie::array_view
//
// Method                           Covered by unit test
//
// ctor(std::vector)                TODO
// ctor(tpie::array)                TODO
// ctor(tpie::internal_vector)      TODO
// ctor(std::vector, start, end)    TODO
// ctor(tpie::array, start, end)    TODO
// ctor(T*, T*)                     TODO
// ctor(T*, n)                      TODO
// at                               TODO
// back                             TODO
// begin                            TODO
// empty                            TODO
// end                              TODO
// find                             TODO
// front                            TODO
// !=                               TODO
// ==                               TODO
// []                               TODO
// size                             TODO

bool basic_test() {
	return false;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic")
		;
}
