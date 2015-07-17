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

#include <tpie/tpie.h>
#include <tpie/sysinfo.h>
#include <tpie/tempname.h>
#include <tpie/serialization_stream.h>
#include <tpie/serialization_sorter.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>
#include "stat.h"
#include "testtime.h"

struct item {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;

	item & operator=(uint64_t v) {
		a = static_cast<uint32_t>(v);
		return *this;
	}

	std::pair<uint32_t, uint32_t> repr() const {
		const uint32_t mask = 0x55555555u;
		return std::make_pair(a & mask, a & ~mask);
	}

	bool operator<(const item & other) const {
		return repr() < other.repr();
	}
};

template <typename D>
void serialize(D & dst, const item & it) {
	dst.write(reinterpret_cast<const char *>(&it), sizeof(it));
}

template <typename S>
void unserialize(S & src, item & it) {
	src.read(reinterpret_cast<char *>(&it), sizeof(it));
}

struct parameters {
	size_t mb;
	size_t times;
	size_t memory;
};

template <typename child_t>
class speed_tester {
	child_t & self() { return *static_cast<child_t *>(this); }
public:
	size_t items;

	void go(parameters params) {
		items = params.mb*1024*1024 / sizeof(item);

		tpie::sysinfo info;
		std::cout << info;
		info.printinfo("MB", params.mb);
		info.printinfo("Samples", params.times);
		info.printinfo("sizeof(item)", sizeof(item));
		info.printinfo("items", items);
		self().init();
		std::vector<const char *> name;
		name.push_back("Write");
		name.push_back("Read");
		tpie::test::stat st(name);
		for (size_t i = 0; i < params.times; ++i) {
			tpie::temp_file temp;
			tpie::test::test_realtime_t t1;
			tpie::test::test_realtime_t t2;
			tpie::test::getTestRealtime(t1);
			self().write(temp.path());
			tpie::test::getTestRealtime(t2);
			st(tpie::test::testRealtimeDiff(t1, t2));

			tpie::test::getTestRealtime(t1);
			self().read(temp.path());
			tpie::test::getTestRealtime(t2);
			st(tpie::test::testRealtimeDiff(t1, t2));
		}
	}
};

struct forward_base {
	static const char * dir() { return "forward"; }
	static uint32_t expect(size_t i, size_t /*items*/) { return static_cast<uint32_t>(i); }
};

struct backward_base {
	static const char * dir() { return "reverse"; }
	static uint32_t expect(size_t i, size_t items) { return items-1-static_cast<uint32_t>(i); }
};

struct serialization_forward : public forward_base {
	typedef tpie::serialization_writer writer;
	typedef tpie::serialization_reader reader;
};

struct serialization_backward : public backward_base {
	typedef tpie::serialization_reverse_writer writer;
	typedef tpie::serialization_reverse_reader reader;
};

struct stream_forward : public forward_base {
	static item read(tpie::file_stream<item> & stream) { return stream.read(); }
	static void seek(tpie::file_stream<item> & /*stream*/) {}
};

struct stream_backward : public backward_base {
	static item read(tpie::file_stream<item> & stream) { return stream.read_back(); }
	static void seek(tpie::file_stream<item> & stream) { stream.seek(0, tpie::file_stream<item>::end); }
};

template <typename Traits>
class serialization_speed_tester : public speed_tester<serialization_speed_tester<Traits> > {
public:
	using speed_tester<serialization_speed_tester<Traits> >::items;

	void init() {
		std::cout << "Testing serialization " << Traits::dir() << " stream" << std::endl;
	}

	void write(std::string path) {
		typename Traits::writer wr;
		wr.open(path);
		item it;
		for (size_t j = 0; j < items; ++j) {
			it = j;
			wr.serialize(it);
		}
		wr.close();
	}

	void read(std::string path) {
		typename Traits::reader rd;
		rd.open(path);
		item it;
		for (size_t j = 0; j < items; ++j) {
			rd.unserialize(it);
			if (it.a != Traits::expect(j, items)) std::cout << "Wrong value read" << std::endl;
		}
		rd.close();
	}
};

typedef serialization_speed_tester<serialization_forward> serialization_forward_speed_tester;
typedef serialization_speed_tester<serialization_backward> serialization_backward_speed_tester;

template <typename Traits>
class stream_speed_tester : public speed_tester<stream_speed_tester<Traits> > {
public:
	using speed_tester<stream_speed_tester<Traits> >::items;

	void init() {
		std::cout << "Testing " << Traits::dir() << " file_stream" << std::endl;
	}

	void write(std::string path) {
		tpie::file_stream<item> wr;
		wr.open(path, tpie::access_write);
		item it;
		for (size_t j = 0; j < items; ++j) {
			it.a = j;
			wr.write(it);
		}
		wr.close();
	}

	void read(std::string path) {
		tpie::file_stream<item> rd;
		rd.open(path, tpie::access_read);
		item it;
		Traits::seek(rd);
		for (size_t j = 0; j < items; ++j) {
			it = Traits::read(rd);
			if (it.a != Traits::expect(j, items)) std::cout << "Wrong value read" << std::endl;
		}
		rd.close();
	}
};

void usage(char ** argv) {
	std::cout << "Usage: " << argv[0] << " [--mb <mb>] [--times <times>] <serialization_forward|serialization_backward|stream_forward|stream_backward>" << std::endl;
}

size_t number(std::string arg) {
	std::stringstream ss(arg);
	size_t res;
	ss >> res;
	return res;
}

template <typename Algorithm>
class sort_tester {
	typedef typename Algorithm::item_type item_type;
	Algorithm a;
	tpie::stream_size_type items;

public:
	sort_tester(parameters params)
		: a(params)
		, items(params.mb * (1024 * 1024 / sizeof(item_type)))
	{
	}

	void go() {
		a.begin(items);
		item_type x;
		for (tpie::stream_size_type i = 0; i < items; ++i) {
			x = (i + 91493)*104729;
			a.push(x);
		}
		a.end();
		x = a.pull();
		for (tpie::stream_size_type i = 1; i < items; ++i) {
			item_type y = a.pull();
			if (y < x) {
				std::cout << "Not sorted" << std::endl;
				return;
			}
			x = y;
		}
	}
};

class serialization_sorter {
public:
	typedef item item_type;

private:
	tpie::serialization_sorter<item, std::less<item> > sorter;

public:
	serialization_sorter(parameters params)
		: sorter(params.memory)
	{
	}

	void begin(tpie::stream_size_type /*items*/) {
		sorter.begin();
	}

	void push(item i) {
		sorter.push(i);
	}

	void end() {
		sorter.end();
	}

	item pull() {
		return sorter.pull();
	}
};

class tpie_sorter {
public:
	typedef item item_type;

private:
	tpie::file_stream<item> data;
	tpie::temp_file f;

public:
	tpie_sorter(parameters /*params*/) {
	}

	void begin(tpie::stream_size_type /*items*/) {
		data.open(f);
	}

	void push(item i) {
		data.write(i);
	}

	void end() {
		tpie::sort(data, data);
		data.seek(0);
	}

	item pull() {
		return data.read();
	}
};

int main(int argc, char ** argv) {
	tpie::tpie_init();
	parameters params;
	params.mb = 20480;
	params.times = 5;
	params.memory = 200*1024*1024;
	std::string type;
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--mb") {
			++i;
			params.mb = number(argv[i]);
		} else if (arg == "--times") {
			++i;
			params.times = number(argv[i]);
		} else {
			type = arg;
			break;
		}
	}
	tpie::get_memory_manager().set_limit(params.memory
										 + tpie::get_memory_manager().used());
	if (type == "serialization_forward") serialization_forward_speed_tester().go(params);
	else if (type == "serialization_backward") serialization_backward_speed_tester().go(params);
	else if (type == "stream_forward") stream_speed_tester<stream_forward>().go(params);
	else if (type == "stream_backward") stream_speed_tester<stream_backward>().go(params);
	else if (type == "serialization_sort") sort_tester<serialization_sorter>(params).go();
	else if (type == "tpie_sort") sort_tester<tpie_sorter>(params).go();
	else usage(argv);
	tpie::tpie_finish();
	return 0;
}
