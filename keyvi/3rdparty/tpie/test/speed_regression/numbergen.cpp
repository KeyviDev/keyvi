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

#include <string>
#include <sstream>

#include <random>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/serialization_stream.h>
#include <ctime>
struct parameters {
	int argi;
	int argc;
	const char ** argv;
	uint32_t seed;
	double megabytes;
	bool gen_tpie;
	bool gen_serialization;

	void parse_args();
	std::string stream_name(std::string suffix) {
		std::stringstream ss;
		ss << "numbers." << seed << "." << suffix;
		return ss.str();
	}
};

typedef double item_type;

uint64_t seed_from_time() {
	return static_cast<uint64_t>(time(nullptr));
}

template <typename N>
N parse_num(std::string arg) {
	std::stringstream ss(arg);
	N res;
	ss >> res;
	return res;
}

uint32_t parse_seed(std::string arg) { return parse_num<uint32_t>(arg); }

std::string next_arg(int & argi, int argc, const char ** argv) {
	++argi;
	if (argi == argc) throw std::invalid_argument("Missing value for option.");
	return argv[argi];
}

void parameters::parse_args() {
	while (argi < argc) {
		std::string arg(argv[argi]);

		if (arg == "--seed")
			seed = parse_seed(next_arg(argi, argc, argv));
		else if (arg == "--mb")
			megabytes = parse_num<double>(next_arg(argi, argc, argv));
		else
			break;

		++argi;
	}
}

void go(parameters & params) {
	tpie::file_stream<item_type> fs;
	tpie::serialization_writer ss;
	if (params.gen_tpie) fs.open(params.stream_name("tpie"));
	if (params.gen_serialization) ss.open(params.stream_name("ser"));
	tpie::stream_size_type numbers =
		static_cast<tpie::stream_size_type>(params.megabytes
											* 1024 * 1024 / sizeof(item_type));
	std::mt19937 rng(params.seed);
	//std::uniform_real_distribution<> dist;
	std::exponential_distribution<> dist(2);
	for (tpie::stream_size_type i = 0; i < numbers; ++i) {
		item_type x = dist(rng);
		if (params.gen_tpie) fs.write(x);
		if (params.gen_serialization) ss.serialize(x);
	}
	if (params.gen_tpie) fs.close();
	if (params.gen_serialization) ss.close();
}

int main(int argc, const char ** argv) {
	parameters params;

	params.argi = 1;
	params.argc = argc;
	params.argv = argv;
	params.seed = seed_from_time();
	params.megabytes = 1024;
	params.gen_tpie = true;
	params.gen_serialization = true;
	params.seed = seed_from_time();
	params.megabytes = 1024;

	params.parse_args();
	tpie::tpie_init();
	go(params);
	tpie::tpie_finish();
	return 0;
}
