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

#include <set>
#include <iostream>
#include <tpie/tpie.h>
#include <boost/filesystem.hpp>
#include <tpie/fractional_progress.h>

namespace {

void usage(const char app[]) {
	std::cout
		<< "Usage: " << app << " db1 db2 [db3 [db4 ...]]\n"
		<< "\n"
		<< "Merges all the given fraction databases into the db1 file.\n"
		;
}

class tpie_initer {
	tpie::flags<tpie::subsystem> subsystems() {
		return (tpie::ALL & ~(tpie::DEFAULT_LOGGING | tpie::PRIMEDB | tpie::JOB_MANAGER))
			| tpie::CAPTURE_FRACTIONS;
	}
public:
	tpie_initer() {
		tpie::tpie_init(subsystems() );
	}

	~tpie_initer() {
		tpie::tpie_finish(subsystems() );
	}
};

} // anonymous namespace

int main(int argc, char ** argv) {
	tpie_initer initer;

	if (argc <= 1 || std::string(argv[1]) == "-h") {
		usage(argv[0]);
		return 1;
	}

	std::set<std::string> filenames;
	for (int i = 1; i != argc; ++i) {
		if (!boost::filesystem::exists(argv[i])) {
			std::cout << argv[i] << ": No such file or directory" << std::endl;
			return 1;
		}
		if (filenames.insert(argv[i]).second) {
			tpie::load_fractions(argv[i]);
		}
	}

	if (filenames.size() > 1) {
		tpie::save_fractions(argv[1], true);
	} else {
		std::cout << "Only one db given; no-op.\n";
	}

	return 0;
}
