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

#include <sstream>

#include <tpie/tpie.h>
#include <tpie/tpie_log.h>

struct parameters {
	int argi;
	int argc;
	const char ** argv;
	size_t fractiles;
	std::string filename;

	void parse_args();
	void parse_filename();
};

typedef double item_type;

template <typename N>
N parse_num(std::string arg) {
	std::stringstream ss(arg);
	N res;
	ss >> res;
	return res;
}

std::string next_arg(int & argi, int argc, const char ** argv) {
	++argi;
	if (argi == argc) throw std::invalid_argument("Missing value for option.");
	return argv[argi];
}

void parameters::parse_args() {
	while (argi < argc) {
		std::string param = argv[argi];

		if (param == "-n")
			fractiles = parse_num<size_t>(next_arg(argi, argc, argv));
		else
			break;

		++argi;
	}
}

void parameters::parse_filename() {
	if (argi == argc) throw std::invalid_argument("Missing filename.");
	filename = argv[argi++];
}

template <typename Impl>
void go(parameters & params, Impl & impl) {
	impl.open(params.filename);
	tpie::stream_size_type nextFrac = 0;
	tpie::stream_size_type nextGoal = 0;
	tpie::stream_size_type n = impl.size();
	for (tpie::stream_size_type i = 0; i < n; ++i) {
		item_type x = impl.read();
		if (i >= nextGoal) {
			std::cout << x << std::endl;
			++nextFrac;
			nextGoal = (n-1) * nextFrac / params.fractiles;
		}
	}
	impl.close();
}

template <typename Impl>
int fractile_main(int argc, const char ** argv) {
	parameters params;

	params.argi = 1;
	params.argc = argc;
	params.argv = argv;
	params.fractiles = 4;

	params.parse_args();
	params.parse_filename();
	params.parse_args();

	tpie::tpie_init(tpie::ALL & ~tpie::DEFAULT_LOGGING);
	tpie::get_memory_manager().set_limit(200*1024*1024);
	{
		tpie::stderr_log_target tgt(tpie::LOG_DEBUG);
		tpie::get_log().add_target(&tgt);
		Impl impl;
		go(params, impl);
		tpie::get_log().remove_target(&tgt);
	}
	tpie::tpie_finish(tpie::ALL & ~tpie::DEFAULT_LOGGING);
	return 0;
}
