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
#include <tpie/array.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>

using namespace tpie;

bool file_count_test() {
	temp_file tmp;
	memory_size_type avail = get_file_manager().available();
	memory_size_type itemSize;
	memory_size_type blockSize;
	memory_size_type userDataSize = 0;
	{
		file_stream<int> src(1.0);
		src.open(tmp);
		blockSize = file<int>::block_size(1.0);
		itemSize = sizeof(int);
	}
	array<tpie::default_file_accessor> fs(avail+1);
	for (memory_size_type i = 0; i < avail; ++i) {
		try {
			fs[i].open(tmp.path(), true, false, itemSize, blockSize, userDataSize, access_sequential, false);
		} catch (tpie::exception & e) {
			tpie::log_error() << "After opening " << i << " files, got an unexcepted exception of type " << typeid(e).name() << std::endl;
			return false;
		}
	}
	tpie::log_error() << "Opened available_files() == " << avail << " files" << std::endl;
	try {
		fs[avail].open(tmp.path(), true, false, itemSize, blockSize, userDataSize, access_sequential, false);
	} catch (tpie::io_exception & e) {
		tpie::log_error() << "Opening another file yields an exception of type\n" << typeid(e).name() << " (" << e.what() << ")\nwhich is allowed per available_files()" << std::endl;
		return true;
	}
	tpie::log_error() << "available_files() is not a strict bound" << std::endl;
	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(file_count_test,"basic");
}
