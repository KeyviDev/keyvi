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

#include "common.h"
#include <tpie/serialization.h>
#include <tpie/serialization2.h>
#include <tpie/serialization_stream.h>
#include <map>

using namespace tpie;
using namespace std;

struct write_container {
	ostream & o;
	write_container(ostream & o): o(o) {}
	void write(const char * x, size_t t) {
		log_info() << "Write " << t << std::endl;
		o.write(x, t);
	}
};

struct read_container {
	istream & o;
	read_container(istream & o): o(o) {}
	void read(char * x, size_t t) {
		log_info() << "Read " << t << std::endl;
		o.read(x, t);
	}
};

struct serializable_dummy {
	static const char msg[];
};

const char serializable_dummy::msg[] = "Hello, yes, this is dog!";

template <typename D>
void serialize(D & dst, const serializable_dummy &) {
	dst.write(serializable_dummy::msg, sizeof(serializable_dummy::msg));
}

template <typename S>
void unserialize(S & src, serializable_dummy &) {
	std::string s(sizeof(serializable_dummy::msg), '\0');
	src.read(&s[0], s.size());
	if (!std::equal(s.begin(), s.end(), serializable_dummy::msg)) {
		throw tpie::exception("Did not serialize the dummy");
	}
}

bool testSer2() {
	std::stringstream ss;
	std::vector<int> v;
	v.push_back(88);
	v.push_back(74);

	write_container wc(ss);
	serialize(wc, (int)454);
	serialize(wc, (float)4.5);
	serialize(wc, true);
	serialize(wc, v);
	serialize(wc, std::string("Abekat"));
	serialize(wc, serializable_dummy());

	int a;
	float b;
	bool c;
	std::vector<int> d;
	std::string e;
	serializable_dummy f;

	read_container rc(ss);
	unserialize(rc, a);
	unserialize(rc, b);
	unserialize(rc, c);
	unserialize(rc, d);
	unserialize(rc, e);
	unserialize(rc, f);
	std::cout << a << " " << b << " " << c << " " << d[0] << " " << d[1] << " " << e <<  std::endl;
	if (a != 454) return false;
	if (b != 4.5) return false;
	if (c != true) return false;
	std::cout << "Here" << std::endl;
	if (d != v) return false;
	std::cout << "Here" << std::endl;
	if (e != "Abekat") return false;
	return true;
}


bool testSer(bool safe) {
	std::stringstream ss;
	std::vector<int> v;
	v.push_back(88);
	v.push_back(74);
	{
		tpie::serializer ser(ss, safe);
		ser << (size_t)454 << (uint8_t)42 << "Hello world" << std::string("monster") << make_pair(std::string("hello"), (float)3.3) << v;
	}

	{	
		ss.seekg(0);
		tpie::unserializer ser(ss);

		size_t a;
		uint8_t b;
		std::string c;
		std::string d;
		std::pair<std::string, float> e;
		std::vector<int> f;
		try {
			ser >> a >> b >> c >> d >> e >> f;
		} catch(serialization_error e) {
			tpie::log_info() << e.what() << std::endl;
			return false;
		}
		if (a != 454 ||
			b != 42 ||
			c != "Hello world" ||
			d != "monster" ||
			e.first != "hello" ||
			(e.second - 3.3) > 1e-9 ||
			f != v) {
			tpie::log_info() << "Unserzation failed" << std::endl;
				return false;		
		}
	}
	return true;
}

bool safe_test() { return testSer(true); }
bool unsafe_test() { return testSer(false); }

bool stream_test() {
	bool result = true;

	memory_size_type N = 2000;
	tpie::array<memory_size_type> numbers(N);
	for (memory_size_type i = 0; i < N; ++i) {
		numbers[i] = i;
	}

	temp_file f;
	{
	serialization_writer ss;
	ss.open(f.path());
	for (memory_size_type i = 0; i < N; ++i) {
		ss.serialize(&numbers[0], &numbers[i]);
	}
	ss.serialize(serializable_dummy());
	ss.close();
	}
	{
	serialization_reader ss;
	ss.open(f.path());
	for (memory_size_type i = 0; i < N; ++i) {
		if (!ss.can_read()) {
			log_error() << "Expected can_read()" << std::endl;
			result = false;
		}
		for (memory_size_type j = 0; j < N; ++j) {
			numbers[j] = N;
		}
		ss.unserialize(&numbers[0], &numbers[i]);
		for (memory_size_type j = 0; j < N; ++j) {
			if (j < i && numbers[j] != j) {
				log_error() << "Incorrect deserialization #" << i << " in position " << j << std::endl;
				result = false;
			}
			if (j >= i && numbers[j] != N) {
				log_error() << "Deserialization #" << i << " changed an array index " << j << " out of bounds" << std::endl;
				result = false;
			}
			numbers[j] = N;
		}
	}
	if (!ss.can_read()) {
		log_error() << "Expected can_read()" << std::endl;
		result = false;
	}
	serializable_dummy d;
	ss.unserialize(d);
	if (ss.can_read()) {
		log_error() << "Expected !can_read()" << std::endl;
		result = false;
	}
	ss.close();
	}

	return result;
}

bool stream_dtor_test() {
	temp_file f;
	{
		serialization_writer ss;
		ss.open(f.path());
		ss.serialize(serializable_dummy());
	}
	{
		serialization_reader ss;
		ss.open(f.path());
		TEST_ENSURE(ss.can_read(), "writer did not write");
	}
	return true;
}

bool stream_reopen_test() {
	temp_file f;
	const int src = 42;
	int dest;
	{
		serialization_writer wr;
		wr.open(f);
		wr.serialize(src);
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f);
		if (!rd.can_read()) {
			log_error() << "Expected can_read()" << std::endl;
			return false;
		}
		rd.unserialize(dest);
		if (rd.can_read()) {
			log_error() << "Expected !can_read()" << std::endl;
			return false;
		}
		rd.close();
		rd.open(f);
		if (!rd.can_read()) {
			log_error() << "Expected can_read()" << std::endl;
			return false;
		}
		rd.close();
	}
	return true;
}

bool stream_reverse_test() {
	bool result = true;

	memory_size_type N = 2000;
	tpie::array<memory_size_type> numbers(N);
	for (memory_size_type i = 0; i < N; ++i) {
		numbers[i] = i;
	}

	temp_file f;
	{
	serialization_reverse_writer ss;
	ss.open(f.path());
	ss.serialize(serializable_dummy());
	for (memory_size_type i = 0; i < N; ++i) {
		ss.serialize(&numbers[0], &numbers[i]);
	}
	ss.serialize(serializable_dummy());
	ss.serialize(serializable_dummy());
	ss.close();
	}
	{
	serialization_reverse_reader ss;
	serializable_dummy d;
	ss.open(f.path());
	ss.unserialize(d);
	ss.unserialize(d);
	for (memory_size_type i = N; i--;) {
		if (!ss.can_read()) {
			log_error() << "Expected can_read()" << std::endl;
			result = false;
		}
		for (memory_size_type j = 0; j < N; ++j) {
			numbers[j] = N;
		}
		ss.unserialize(&numbers[0], &numbers[i]);
		for (memory_size_type j = 0; j < N; ++j) {
			if (j < i && numbers[j] != j) {
				log_error() << "Incorrect deserialization #" << i << " in position " << j << std::endl;
				log_error() << "Got " << numbers[j] << ", expected " << j << std::endl;
				result = false;
			}
			if (j >= i && numbers[j] != N) {
				log_error() << "Deserialization #" << i << " changed an array index " << j << " out of bounds" << std::endl;
				result = false;
			}
			numbers[j] = N;
		}
	}
	if (!ss.can_read()) {
		log_error() << "Expected can_read()" << std::endl;
		result = false;
	}
	ss.unserialize(d);
	if (ss.can_read()) {
		log_error() << "Expected !can_read()" << std::endl;
		result = false;
	}
	ss.close();
	}

	return result;
}

bool stream_temp_test() {
	stream_size_type tmpUsage1, tmpUsage2, tmpUsage3, tmpUsage4;
	tmpUsage1 = get_temp_file_usage();
	{
		tpie::temp_file f;
		{
			serialization_writer wr;
			wr.open(f);
			tmpUsage2 = get_temp_file_usage();
			wr.serialize(1);
			wr.serialize(2);
			wr.serialize(3);
			wr.close();
		}
		tmpUsage3 = get_temp_file_usage();
		{
			serialization_reader rd;
			int i;
			rd.open(f);
			rd.unserialize(i);
			rd.close();
		}
	}
	tmpUsage4 = get_temp_file_usage();
	log_info()
		<< "Usage before open:    " << tmpUsage1 << '\n'
		<< "Usage after open:     " << tmpUsage2 << '\n'
		<< "Usage after close:    " << tmpUsage3 << '\n'
		<< "Usage after destruct: " << tmpUsage4 << std::endl;
	return tmpUsage1 == tmpUsage2
		&& tmpUsage1 == tmpUsage4
		&& tmpUsage3 > tmpUsage2;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(safe_test, "safe")
		.test(unsafe_test, "unsafe")
		.test(testSer2, "serialization2")
		.test(stream_test, "stream")
		.test(stream_dtor_test, "stream_dtor")
		.test(stream_reopen_test, "stream_reopen")
		.test(stream_reverse_test, "stream_reverse")
		.test(stream_temp_test, "stream_temp")
		;
}
