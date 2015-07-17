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

#include <iostream>
#include <tpie/file_stream.h>
#include <tpie/compressed/stream.h>

struct file_stream {
	typedef tpie::file_stream<size_t> Stream;
	static void open(Stream & s, const std::string & fileName) {
		s.open(fileName);
	}
};

struct uncompressed_stream {
	typedef tpie::file_stream<size_t> Stream;
	static void open(Stream & s, const std::string & fileName) {
		s.open(fileName, tpie::access_read_write, 0, tpie::access_sequential, tpie::compression_none);
	}
};

struct compressed_stream {
	typedef tpie::file_stream<size_t> Stream;
	static void open(Stream & s, const std::string & fileName) {
		s.open(fileName);
	}
};

size_t f(size_t prev, size_t n) {
	return ((42 + prev) * n) >> 3;
}

void usage() {
	std::cout << "Parameters: <file_stream|uncompressed|compressed> <read|write> <filename> <items>" << std::endl;
}

template <typename Stream>
void write(Stream & s, size_t items) {
	size_t prev = 0;
	for (size_t i = 0; i < items; ++i) {
		s.write(prev);
		prev = f(prev, i);
	}
}

template <typename Stream>
void read(Stream & s, size_t items) {
	size_t prev = 0;
	for (size_t i = 0; i < items; ++i) {
		size_t x = s.read();
		if (x != prev) {
			std::cerr << "bad value at " << i << std::endl;
		}
		prev = f(prev, i);
	}
}

template <typename Traits>
void go_class(const std::string & command, const std::string fileName, size_t items) {
	typename Traits::Stream s;
	Traits::open(s, fileName);
	if (command == "read") read(s, items);
	else if (command == "write") write(s, items);
	else usage();
}

void go(std::string cls, std::string command, std::string fileName, std::string itemsString) {
	size_t items;
	std::stringstream(itemsString) >> items;
	if (cls == "file_stream") go_class<file_stream>(command, fileName, items);
	else if (cls == "uncompressed") go_class<uncompressed_stream>(command, fileName, items);
	else if (cls == "compressed") go_class<compressed_stream>(command, fileName, items);
	else usage();
}

int main(int argc, char ** argv) {
	if (argc != 5) {
		usage();
		return 1;
	}
	tpie::tpie_init();
	go(argv[1], argv[2], argv[3], argv[4]);
	tpie::tpie_finish();
	return 0;
}
