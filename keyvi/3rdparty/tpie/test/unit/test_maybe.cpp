// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
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

#include "tpie/maybe.h"
#include "tpie/memory.h"

using namespace tpie;

// Method coverage of tpie::maybe
//
// Method           			Covered by unit test
//
// ctor()						basic, unique_ptr_test
// ctor(const maybe>T> & o)		basic, unique_ptr_test
// operator=					basic, unique_ptr_test
// construct					basic, unique_ptr_test
// destruct						basic, unique_ptr_test
// is_constructed				basic, unique_ptr_test
// operator*					basic, unique_ptr_test
// const operator* const		basic, unique_ptr_test
// operator->					unique_ptr_test
// const operator-> const		unique_ptr_test

bool basic_test() {
	maybe<std::string> foo; 

	// test copy constructor and = operator
	try {
		maybe<std::string> bar(foo); 
		maybe<std::string> baz = foo; 
	} catch(maybe_exception e) {
		TEST_FAIL("Should not throw an exception");
	}

	foo.construct("Horse, cow, cat, walrus");


	// test copy constructor and = operator
	try {
		maybe<std::string> bar(foo); 
		maybe<std::string> baz = foo; 

		TEST_FAIL("Should have thrown an exception");
	} catch(maybe_exception e) {
		// should throw an exception
	}

	TEST_ENSURE(foo.is_constructed(), "Should be constructed"); 
	TEST_ENSURE_EQUALITY(*foo, "Horse, cow, cat, walrus", "Wrong value"); 
	TEST_ENSURE_EQUALITY(foo->size(), 23, "Wrong value"); 
	

	foo.destruct(); 
	TEST_ENSURE(!foo.is_constructed(), "Should not be constructed"); 
	
	return true;
}

bool unique_ptr_test() {
	maybe<unique_ptr<int> > foo; 

	// test copy constructor and = operator
	try {
		maybe<unique_ptr<int> > bar(foo); 
		maybe<unique_ptr<int> > baz = foo; 
	} catch(maybe_exception e) {
		std::cout << "Should not throw an exception" << " " << e.what();
		return false;
	}


	foo.construct(tpie_new<int>(5));


	// test copy constructor and = operator
	try {
		maybe<unique_ptr<int> > bar(foo); 
		maybe<unique_ptr<int> > baz = foo; 

		std::cout << "Should have thrown an exception";
		return false; 
	} catch(maybe_exception e) {
		// should throw an exception
	}

	TEST_ENSURE(foo.is_constructed(), "Should be constructed"); 

	TEST_ENSURE_EQUALITY(**foo, 5, "Wrong value"); 
	

	foo.destruct(); 
	TEST_ENSURE(!foo.is_constructed(), "Should not be constructed"); 

	return true; 
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv, 128)
		.test(basic_test, "basic")
		.test(unique_ptr_test, "unique_ptr")
		;
}
