// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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

// This program tests sequential reads and writes of 8 MB of 64-bit int items,
// sequential read and write of 8 MB of 64-bit int arrays,
// random seeking in 8 MB followed by either a read or a write.

#include "common.h"
#include <iostream>
#include <tpie/tpie.h>
#include <tpie/array.h>
#include <tpie/file_stream.h>
#include <tpie/util.h>
#include <tpie/stats.h>

using namespace tpie;

bool test_about(stream_size_type val, stream_size_type expect, const char * name) {
	if (val + 10240 < 9*expect/10 ||
		val > 11*expect/10 + 10240) {
		tpie::log_error() << "Wrong " << name << " got " << val << " expected " << expect << std::endl; 
		return false;
	}
	return true;
}

bool simple_test(size_type size) {
	stream_size_type asize=size*sizeof(uint64_t);
	if (!test_about(get_bytes_read(), 0, "bytes read")) return false;
	if (!test_about(get_bytes_written(), 0, "bytes written")) return false;
	if (!test_about(get_temp_file_usage(), 0, "temp file usage")) return false;
	{
		temp_file tf;
		{
			file_stream<uint64_t> s;
			s.open(tf);
			for(size_t i=0; i < size; ++i) s.write(i);
		}
		if (!test_about(get_bytes_read(), 0, "bytes read")) return false;
		if (!test_about(get_bytes_written(), asize, "bytes written")) return false;
		if (!test_about(get_temp_file_usage(), asize, "temp file usage")) return false;
		{
			file_stream<uint64_t> s;
			s.open(tf);
			for(size_t i=0; i < size; ++i) s.read();
		}
		if (!test_about(get_bytes_read(), asize, "bytes read")) return false;
		if (!test_about(get_bytes_written(), asize, "bytes written")) return false;
		if (!test_about(get_temp_file_usage(), asize, "temp file usage")) return false;
		{
			file_stream<uint64_t> s;
			s.open(tf);
			s.truncate(0);
		}
	}
	if (!test_about(get_bytes_read(), asize, "bytes read")) return false;
	if (!test_about(get_bytes_written(), asize, "bytes written")) return false;
	if (!test_about(get_temp_file_usage(), 0, "temp file usage")) return false;
	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(simple_test, "simple", "size", 1024*1024*10);
}
