// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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
#ifndef __COMMON_H__
#define __COMMON_H__

#include "../app_config.h"
#include <tpie/memory.h>
#include <tpie/util.h>
#include <iostream>
#include <boost/cstdint.hpp>
#include <tpie/tpie.h>
#include <tpie/sysinfo.h>
#include <tpie/exception.h>
#include <tpie/unittest.h>
#include <tpie/tpie_log.h>

struct bit_permute {
	boost::uint64_t operator()(boost::uint64_t i) const{
		return (i & 0xAAAAAAAAAAAAAAAALL) >> 1 | (i & 0x5555555555555555LL) << 1;
	}
};

template <typename T=std::less<uint64_t> >
struct bit_pertume_compare: std::binary_function<boost::uint64_t, boost::uint64_t, bool> {
	bit_permute bp;
	T c;
	typedef boost::uint64_t first_argument_type;
	typedef boost::uint64_t second_argument_type;

	bool operator()(boost::uint64_t a, boost::uint64_t b) const {
		return c(bp(a), bp(b));
	}
};

struct memory_monitor {
	tpie::size_type base;
	tpie::size_type used;
	inline void begin() {
		used = base = tpie::get_memory_manager().used();
	}
	inline void sample() {
		used = std::max(used, tpie::get_memory_manager().used());
	}
	inline void clear() {
		used = tpie::get_memory_manager().used();
	}
	inline void empty() {
		used = base;
	}
	inline tpie::size_type usage() {
		return used-base;
	}
};

class memory_test {
public:
	virtual void free() = 0;
	virtual void alloc() = 0;
	virtual void use() {}
	virtual tpie::size_type claimed_size() = 0;
	bool operator()() {
		bool res=true;
		tpie::get_memory_manager().set_limit(128*1024*1024);
		tpie::size_type g = claimed_size();
		memory_monitor mm;
		mm.begin();
		alloc();
		mm.sample();
		use();
		mm.sample();
		if (mm.usage() > g) {
			tpie::log_error() << "Claimed to use " << g << " but used " << mm.usage() << std::endl;
			res=false;
		}
		free();
		mm.empty();
		mm.sample();
		if (mm.usage() > 0) {
			tpie::log_error() << "Leaked memory " << mm.usage() << std::endl;
			res=false;
		}
		return res;
	}
};

class tpie_initer {
public:
	tpie_initer(size_t memory_limit=50) {
		tpie::tpie_init();
		tpie::get_memory_manager().set_limit(memory_limit*1024*1024);
	}
	
	~tpie_initer() {
		tpie::tpie_finish();
	}
};

// Type of test function
typedef bool fun_t();

// Type of fixture function
typedef void fixture_t();

struct unittests {

	const char * progname;

	// How many tests were run (if 0, usage is printed)
	int tests;

	// True if all tests pass, false otherwise
	bool result;

	// Name of test to run
	std::string testname;

	// Whether we should run all tests
	bool testall;

	// If testall, capture a list of failing tests
	std::stringstream faillog;

	// List of tests concatenated by '|' (for the usage string)
	std::stringstream usagestring;

	std::vector<fixture_t *> fixtures;

	tpie_initer initer;

	unittests(int argc, char ** argv)
		: progname(argv[0])
		, tests(0)
		, result(true)
		, testname("")
		, testall(false)
		, initer(32)
	{
		tpie::get_memory_manager().set_enforcement(tpie::memory_manager::ENFORCE_THROW);
		if (argc > 1) {
			testname = argv[1];
		}
		if (testname == "all") {
			testall = true;
			tpie::sysinfo s;
			std::cerr << s;
		}
	}

	operator int() {
		if (!tests) usage();
		if (testall) {
			std::cerr << std::string(79, '=');
			if (result) std::cerr << "\nAll tests passed" << std::endl;
			else std::cerr << "\nThe following tests FAILED:\n" << faillog.str() << std::flush;
		}
		return result ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	// Run test, increment `tests', set `result' if failed, output if `testall'
	template <fun_t f>
	inline unittests & test(std::string name) {
		usagestring << '|' << name;

		if (!testall && testname != name) return *this;
		++tests;
		if (testall)
			std::cerr << std::string(79,'=') << "\nStart " << std::setw(2) << tests << ": " << name << std::endl;
		for (size_t i = 0; i < fixtures.size(); ++i) {
			fixtures[i]();
		}
		bool pass = false;
		try {
			pass = f();
		} catch (tpie::exception & e) {
			std::cerr << "Caught a tpie::exception (actually " << typeid(e).name() << ") in test \"" << name << "\"\ne.what() = " << e.what() << std::endl;
		} catch (std::exception & e) {
			std::cerr << "Caught a std::exception (actually " << typeid(e).name() << ") in test \"" << name << "\"\ne.what() = " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Caught something that is not an exception in test \"" << name << "\"" << std::endl;
		}
		for (size_t i = 0; i < fixtures.size(); ++i) {
			fixtures[i]();
		}
		if (testall) {
			std::cerr << "\nTest  " << std::setw(2) << tests << ": " << name << ' ' << std::string(59-name.size(), '.')
			<< (pass ? "   Passed" : "***Failed") << std::endl;
			if (!pass) faillog << "  * " << name << '\n';
		}

		if (!pass) result = false;

		return *this;
	}

	inline unittests & fixture(fixture_t f) {
		fixtures.push_back(f);
		return *this;
	}

	void usage() {
		std::cerr << "Usage: " << progname << " [all" << usagestring.str() << ']' << std::endl;
		exit(EXIT_FAILURE);
	}

};

#endif //__COMMON_H__
