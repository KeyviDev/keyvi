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
#include <tpie/pipelining.h>

namespace tp = tpie::pipelining;

template <typename dest_t>
class line_reader_type : public tp::node {
	dest_t dest;

public:
	line_reader_type(dest_t dest)
		: dest(std::move(dest))
	{
		this->add_push_destination(dest);
	}

	void go() {
		std::string line;
		while (std::getline(std::cin, line)) {
			dest.push(line);
		}
	}
};

typedef tp::pipe_begin<tp::factory<line_reader_type> > line_reader;

class line_writer_type : public tp::node {
public:
	typedef std::string item_type;

	line_writer_type() {
	}

	void push(const std::string & line) {
		std::cout << line << '\n';
	}
};

typedef tp::pipe_end<tp::termfactory<line_writer_type> > line_writer;

int main() {
	tpie::tpie_init();
	const tpie::memory_size_type memory = 100*1024*1024;
	tpie::get_memory_manager().set_limit(memory);
	{
	tp::pipeline p = line_reader() | tp::serialization_sort() | line_writer();
	p.plot(std::clog);
	p();
	}
	tpie::log_info() << "Temp file usage: " << tpie::get_temp_file_usage() << std::endl;
	tpie::tpie_finish();
	return 0;
}
