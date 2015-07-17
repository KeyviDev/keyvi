// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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

#include <boost/progress.hpp>
#include <boost/serialization/strong_typedef.hpp>
#include <sstream>
#include <tpie/array.h>
#include <tpie/tpie.h>
#include <vector>

size_t copy_constructed;
size_t blah_constructed;
size_t std_constructed;
size_t assigned;

struct strong_size_t {
	typedef size_t T;
	T t;
	inline strong_size_t(const strong_size_t & t) : t(t.t) {
		++copy_constructed;
	}
	inline strong_size_t(const T & t) : t(t) {
		++blah_constructed;
	}
	inline strong_size_t() {
		++std_constructed;
	}
	inline strong_size_t & operator=(const strong_size_t & other) {
		++assigned;
		t = other.t;
		return *this;
	}
	inline strong_size_t operator^(const strong_size_t & other) {
		return t ^ other.t;
	}
	inline strong_size_t & operator^=(const strong_size_t & other) {
		t ^= other.t;
		return *this;
	}
};

std::ostream & operator<<(std::ostream & o, const strong_size_t & s) {
	return o << s.t;
}

using namespace tpie;

template <typename test_t>
void test(size_t mb, size_t repeats) {
	const size_t sz = mb*((1<<20)/sizeof(test_t));
	std::cout << mb << " MB, " << repeats << " repeats" << std::endl;

	std::cout << "===============================================================================" << std::endl;
	std::cout << "tpie::array" << std::endl;
	blah_constructed = copy_constructed = std_constructed = assigned = 0;
	{
		boost::progress_timer _;
		test_t res(0);
		for (size_t j = 0; j < repeats; ++j) {
			tpie::array<test_t> a(sz, res);
			for (size_t i = 0; i < sz; i += 4096/sizeof(test_t)) {
				if (i) res ^= a[i-4096/sizeof(test_t)];
				a[i] = i+1;
			}
		}
		std::cout << res << std::endl;
	}
	std::cout << "copy_constructed blah_constructed std_constructed assigned\n";
	std::cout << copy_constructed << ' ' << blah_constructed << ' ' << std_constructed << ' ' << assigned << std::endl;

	std::cout << "===============================================================================" << std::endl;
	std::cout << "tpie_new_array" << std::endl;
	blah_constructed = copy_constructed = std_constructed = assigned = 0;
	{
		boost::progress_timer _;
		test_t res(0);
		for (size_t j = 0; j < repeats; ++j) {
			test_t * a = tpie_new_array<test_t>(sz);
			for (size_t i = 0; i < sz; i += 4096/sizeof(test_t)) {
				if (i) res ^= a[i-4096/sizeof(test_t)];
				a[i] = i+1;
			}
			tpie_delete_array(a, sz);
		}
		std::cout << res << std::endl;
	}
	std::cout << "copy_constructed blah_constructed std_constructed assigned\n";
	std::cout << copy_constructed << ' ' << blah_constructed << ' ' << std_constructed << ' ' << assigned << std::endl;

	std::cout << "===============================================================================" << std::endl;
	std::cout << "std::vector" << std::endl;
	blah_constructed = copy_constructed = std_constructed = assigned = 0;
	{
		boost::progress_timer _;
		test_t res(0);
		for (size_t j = 0; j < repeats; ++j) {
			std::vector<test_t> a(sz);
			for (size_t i = 0; i < sz; i += 4096/sizeof(test_t)) {
				if (i) res ^= a[i-4096/sizeof(test_t)];
				a[i] = i+1;
			}
		}
		std::cout << res << std::endl;
	}
	std::cout << "copy_constructed blah_constructed std_constructed assigned\n";
	std::cout << copy_constructed << ' ' << blah_constructed << ' ' << std_constructed << ' ' << assigned << std::endl;
	std::cout << "===============================================================================" << std::endl;
}

int main(int argc, char ** argv) {
	std::string progname = argv[0];
	bool boost_strong_typedef = false;
	--argc, ++argv;
	while (argc) {
		std::string arg(argv[0]);
		if (arg == "--help" || arg == "-h") {
			std::cout << "Usage: " << progname << " [--strong] [mb] [repeats]" << std::endl;
		} else if (arg == "--strong") {
			std::cout << "Using Boost strong typedef" << std::endl;
			boost_strong_typedef = true;
		} else {
			break;
		}
		--argc, ++argv;
	}
	tpie_init(ALL & ~JOB_MANAGER);
	size_t mb = 1 << 7;
	if (argc > 0) std::stringstream(argv[0]) >> mb;
	size_t repeats = 64;
	if (argc > 1) std::stringstream(argv[1]) >> repeats;
	if (boost_strong_typedef) {
		test<strong_size_t>(mb, repeats);
	} else {
		test<size_t>(mb, repeats);
	}
	tpie_finish(ALL & ~JOB_MANAGER);
	return 0;
}
