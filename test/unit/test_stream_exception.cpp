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

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include "common.h"
#include <boost/filesystem/operations.hpp>
#include <iostream>

using namespace tpie;

typedef int test_t;
static const std::string TEMP_FILE = "tmp";

static bool test() {
	boost::filesystem::remove(TEMP_FILE);
	file_stream<test_t> fs;
	fs.open(TEMP_FILE);
	fs.write(1);
	fs.write(2);
	fs.write(3);
	bool got_exception = false;
	bool got_other = false;
	try {
		fs.read();
	} catch (const end_of_stream_exception & e) {
		got_exception = true;
	} catch (const std::runtime_error & e) {
		got_other = true;
		log_error() << "Got an unexpected " << typeid(e).name() << ": " << e.what() << std::endl;
	}
	if (!got_exception)
		log_error() << "Did not get a tpie::end_of_stream_exception" << std::endl;
	fs.close();
	boost::filesystem::remove(TEMP_FILE);
	return got_exception && !got_other;
}
int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(test, "basic");
}
