// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013 The TPIE development team
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
#include <tpie/pipelining.h>
#include <tpie/serialization_stream.h>
#include <random>
#include <ctime>

using namespace tpie;
using namespace tpie::pipelining;

void populate_test_data(std::vector<std::string> & test) {
	char c = '!';
	size_t ante = 0;
	size_t prev = 1;
	size_t size = 0;
	for (size_t i = 0; i < 31; ++i) {
		size_t cur = ante+prev;
		ante = prev;
		prev = cur;

		test.push_back(std::string(cur, c++));

		size += cur;
	}
}

bool basic_test() {
	tpie::temp_file f_in;
	tpie::temp_file f_out;
	std::vector<std::string> testData;
	populate_test_data(testData);
	{
		serialization_writer wr;
		wr.open(f_in.path());
		for (size_t i = 0; i < testData.size(); ++i) {
			wr.serialize(testData[i]);
		}
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f_in.path());
		serialization_writer wr;
		wr.open(f_out.path());
		pipeline p = serialization_input(rd) | serialization_output<std::string>(wr);
		p.plot(log_info());
		p();
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f_out.path());
		for (size_t i = 0; i < testData.size(); ++i) {
			if (!rd.can_read()) {
				log_error() << "Could not read item" << std::endl;
				return false;
			}
			std::string d;
			rd.unserialize(d);
			if (d != testData[i]) {
				log_error() << "Wrong item read" << std::endl;
				return false;
			}
		}
	}
	return true;
}

bool reverse_test() {
	tpie::temp_file f_in;
	tpie::temp_file f_out;
	std::vector<std::string> testData;
	populate_test_data(testData);
	{
		serialization_writer wr;
		wr.open(f_in.path());
		for (size_t i = testData.size(); i--;) {
			wr.serialize(testData[i]);
		}
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f_in.path());
		serialization_writer wr;
		wr.open(f_out.path());
		pipeline p = serialization_input(rd) | serialization_reverser() | serialization_output<std::string>(wr);
		p.plot(log_info());
		p();
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f_out.path());
		for (size_t i = 0; i < testData.size(); ++i) {
			if (!rd.can_read()) {
				log_error() << "Could not read item" << std::endl;
				return false;
			}
			std::string d;
			rd.unserialize(d);
			if (d != testData[i]) {
				log_error() << "Wrong item read" << std::endl;
				return false;
			}
		}
	}
	return true;
}

template <typename dest_t>
class random_strings_type : public node {
	dest_t dest;
	stream_size_type n;

public:
	random_strings_type(dest_t dest, stream_size_type n)
		: dest(std::move(dest))
		, n(n)
	{
		add_push_destination(dest);
	}

	virtual void go() override {
		std::mt19937 rng(std::time(0));
		for (stream_size_type i = 0; i < n; ++i) {
			size_t length = rng() % 10;
			std::string s(length, '\0');
			for (size_t j = 0; j < length; ++j) s[j] = 'a' + (rng() % 26);
			dest.push(s);
		}
	}
};

typedef pipe_begin<factory<random_strings_type, stream_size_type> > random_strings;

class sort_verifier_type : public node {
	bool & res;
	std::string prev;

public:
	typedef std::string item_type;

	sort_verifier_type(bool & res)
		: res(res)
	{
		res = true;
	}

	void push(const std::string & s) {
		if (s < prev) {
			res = false;
			log_error() << "Got an out of order string" << std::endl;
		}
		prev = s;
	}
};

typedef pipe_end<termfactory<sort_verifier_type, bool &> > sort_verifier;

bool sort_test(stream_size_type n) {
	bool result = false;
	pipeline p =
		random_strings(n)
		| serialization_sort()
		| sort_verifier(result)
		;
	p();
	return result;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.test(basic_test, "basic")
	.test(reverse_test, "reverse")
	.test(sort_test, "sort", "n", static_cast<stream_size_type>(1000))
	;
}
