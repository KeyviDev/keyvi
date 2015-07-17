// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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
#include "../app_config.h"
#include <tpie/portability.h>
#include <tpie/tpie.h>
#include <tpie/util.h>
#include <tpie/stack.h>
#include <tpie/stream.h>
#include <tpie/prime.h>
#include <tpie/tempname.h>
#include "common.h"
#include <boost/filesystem.hpp>

using namespace tpie;

bool ami_named_stack_test() {
	boost::filesystem::remove("temp_stack");

	{
		ami::stack<size_t> s("temp_stack");
		const size_t size= 1234;
		for(size_t i=1; i < size; ++i) 
			s.push(i);
	}

	{
		ami::stack<size_t> s("temp_stack");
		const size_t size= 1234;
		for(size_t i=size-1; i >= 1; --i) {
			const size_t * x = 0;
			s.pop(&x);
			if (*x != i) return false;
		}
	}
	boost::filesystem::remove("temp_stack");
	return true;
}

bool named_stack_test() {
	boost::filesystem::remove("temp_stack");

	{
		stack<size_t> s("temp_stack");
		const size_t size= 1234;
		for(size_t i=1; i < size; ++i) 
			s.push(i);
	}

	{
		stack<size_t> s("temp_stack");
		const size_t size= 1234;
		for(size_t i=size-1; i >= 1; --i) {
			const size_t x = s.pop();
			if (x != i) return false;
		}
	}
	boost::filesystem::remove("temp_stack");
	return true;
}

bool ami_stack_test(size_t size) {
  ami::stack<size_t> s;
  size_t i=1234;
  for(size_t _=0; _ < size; ++_) {
    s.push(i) ;
    ++i;
    if ((size_t)s.size() != _ +1) {
		tpie::log_error() << "size failed" << std::endl;
		return false;
    }
  }
  size_t o=i-1;
  for(size_t _=0; _ < size; ++_) {
    s.push(i) ;
    const size_t * x = 0;
    s.pop(&x);
    if (*x != i) {
		tpie::log_error() << "Wrong element" << std::endl;
		return false;
    }
    ++i;
    
    if ((size_t)s.size() != size) {
		tpie::log_error() << "size failed 2" << std::endl;
		return false;
    }
  }

  for(size_t _=0; _ < size; ++_) {
    const size_t * x = 0;
    s.pop(&x);
    if (*x != o) {
		tpie::log_error() << "Wrong element 2" << std::endl;
		return false;
    }
    --o;
  }
  
  if (s.size() != 0) return false;
  return true;
}

#define ASSERT(cond, msg) if (!(cond)) { tpie::log_error() << msg << std::endl; return false; }
bool stack_test(size_t size) {
	stack<size_t> s;
	ASSERT(s.size() == 0, "Wrong initial size");
	for (size_t i=0; i < size; ++i) {
		size_t x = i+1234;
		s.push(x);
		ASSERT(s.size() == i+1, "Wrong size after push");
	}

	for (size_t i=0; i < size; ++i) {
		size_t x = 1233+size-i;
		size_t read1 = s.top();
		ASSERT(x == read1, "Wrong item on top: Expected " << x << ", got " << read1);
		size_t read = s.pop();
		ASSERT(s.size() == size-i-1, "Wrong size after pop");
		ASSERT(x == read, "Wrong item popped: Expected " << x << ", got " << read);
	}

	return true;
}


bool io_test() {
	typedef uint64_t test_t;
	stack<test_t> s;

	// some block boundary, in bytes
	const size_t block_boundary = 4*1024*1024;

	for (size_t i = 0; i < block_boundary/sizeof(test_t); ++i) {
		s.push(test_t());
	}
	// stack now contains block_boundary bytes

	const stream_size_type before = get_bytes_written();
	// enter a new block, forcing a write of the full buffer
	s.push(test_t());
	const stream_size_type after = get_bytes_written();
	const stream_size_type write = after-before;
	tpie::log_info() << "Before: " << before << ", after: " << after << " (difference " << write << ")" << std::endl;

	stream_size_type prev = after;
	// cross the block boundary a number of times
	const size_t repeats = 100;
	for (size_t i = 0; i < repeats; ++i) {
		s.pop();
		s.pop();
		s.push(test_t());
		s.push(test_t());
		stream_size_type now = get_bytes_written();
		//std::cerr << now << " (" << (now-prev) << ")" << std::endl;
		prev = now;
	}
	tpie::log_info() << "Crossing the block boundary " << repeats << " times, in total writing " << (prev-after) << " bytes" << std::endl;
	if ((prev-after) > 2*write) {
		tpie::log_error() << "Too inefficient!" << std::endl;
		return false;
	}
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(ami_stack_test, "ami", "size", 1024*1024*3)
		.test(ami_named_stack_test, "named-ami")
		.test(stack_test, "new", "size", 1024*1024*3)
		.test(named_stack_test, "named-new")
		.test(io_test, "io");
}
