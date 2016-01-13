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

#include "common.h"
#include <tpie/hash_map.h>
#include <tpie/tpie.h>
#include <map>
#include <random>
#include <unordered_map>
#include "test_timer.h"
#include <iomanip>
using namespace tpie;
using namespace std;

template <template <typename value_t, typename hash_t, typename equal_t, typename index_t> class table_t>
bool basic_test() {
	tpie::hash_map<int, char, tpie::hash<int>, std::equal_to<int>, size_t, table_t> q1(200);
	map<int, char> q2;
	std::default_random_engine prng(42);
	for(int i=0; i < 100; ++i) {
		int k = (prng()*2) % 250;
		char v = static_cast<char>(prng() % 265);
		q1[k] = v;
		q2[k] = v;
	}
	while (!q2.empty()) {
		if (q1.size() != q2.size()) {
			tpie::log_error() << "Size differs " << q1.size() << " " << q2.size() << std::endl;
			return false;
		}
		for (map<int, char>::iterator i=q2.begin(); i != q2.end(); ++i) {
			if (q1.find((*i).first) == q1.end()) {
				tpie::log_error() << "Element too much" << std::endl;
				return false;
			}
			if (q1[(*i).first] != (*i).second) {
				tpie::log_error() << "Value differs" << std::endl;
				return false;
			}
			if (q1.find((*i).first+1) != q1.end()) {
				tpie::log_error() << "Element too much" << std::endl;
				return false;
			}

		}
		int x=(*q2.begin()).first;
		q1.erase(x);
		q2.erase(x);
	}
	return true;
}

struct charm_gen {
	static inline size_t key(size_t i) {
		return (i*21467) % 0x7FFFFFFF;
	}
	static inline size_t value(size_t i) {
		return (i*41983)%128;
	}
	static inline size_t cnt() {return 1000000;}
};

struct identity_gen {
	static inline size_t key(size_t i) {
		return i;
	}
	static inline size_t value(size_t i) {
		return i%128;
	}
	static inline size_t cnt() {return 1000000;}
};

template <typename gen_t,
		  template <typename value_t, typename hash_t, typename equal_t, typename index_t> class table_t>
void test_speed() {
	test_timer insert_hash_map("insert hash_map");
	test_timer insert_unordered_map("insert unorderd_map");

	test_timer find_hash_map("find hash_map");
	test_timer find_unordered_map("find unordered_map");

	test_timer erase_hash_map("erase hash_map");
	test_timer erase_unordered_map("erase unordered_map");

	for(size_t t=0; t < 100; ++t) {
		insert_hash_map.start();
		tpie::hash_map<size_t, char, tpie::hash<size_t>, std::equal_to<size_t>, size_t, table_t> q1(gen_t::cnt());
		for(size_t i=0; i < gen_t::cnt();++i)
			q1[gen_t::key(i)] = static_cast<char>(gen_t::value(i));
		insert_hash_map.stop();

		insert_unordered_map.start();
		std::unordered_map<size_t, char> q2;
		for(size_t i=0; i < gen_t::cnt();++i)
			q2[gen_t::key(i)] = static_cast<char>(gen_t::value(i));
		insert_unordered_map.stop();

		size_t x=42;
		find_hash_map.start();
		for(size_t i=0; i < gen_t::cnt();++i)
			x ^= q1.find(gen_t::key(i))->second;
		find_hash_map.stop();

		find_unordered_map.start();
		for(size_t i=0; i < gen_t::cnt();++i)
			x ^= q2.find(gen_t::key(i))->second;
		find_unordered_map.stop();

		erase_hash_map.start();
		for(size_t i=0; i < gen_t::cnt();++i)
			q1.erase(gen_t::key(i));
		erase_hash_map.stop();

		erase_unordered_map.start();
		for(size_t i=0; i < gen_t::cnt();++i)
			q2.erase(gen_t::key(i));
		erase_unordered_map.stop();

		if (x + q1.size() + q2.size() != 42) tpie::log_info() << "Orly" << std::endl;
		tpie::log_info() << std::setw(3) << t  << "%\r" << std::flush;
	}
	insert_hash_map.output();
	insert_unordered_map.output();
	find_hash_map.output();
	find_unordered_map.output();
	erase_hash_map.output();
	erase_unordered_map.output();
}

bool iterator_test() {
	tpie::hash_map<int, char> m(20);
	vector< std::pair<int,char> > d;
	vector< std::pair<int,char> > r;
	
	d.push_back(make_pair(5,'c'));
	d.push_back(make_pair(7,'a'));
	d.push_back(make_pair(4,'k'));
	d.push_back(make_pair(9,'e'));
	d.push_back(make_pair(10,'x'));
	for(size_t i=0; i < d.size(); ++i) m.insert(d[i].first, d[i].second);
	for(tpie::hash_map<int, char>::iterator i=m.begin(); i != m.end(); ++i)
		r.push_back(*i);
	sort(d.begin(), d.end());
	sort(r.begin(), r.end());
	if (r != d) return false;
	return true;
}

class hashmap_memory_test: public memory_test {
public:
	tpie::hash_map<int, char> * a;
	virtual void alloc() {a = new tpie::hash_map<int, char>(123456);}
	virtual void free() {delete a;}
	virtual size_type claimed_size() {return static_cast<size_type>(tpie::hash_map<int, char>::memory_usage(123456));}
};

bool speed() {
	tpie::log_info() << "=====================> Linear Probing, Charm Dataset <========================" << std::endl;
	test_speed<charm_gen, linear_probing_hash_table>();
	tpie::log_info() << "========================> Chaining, Charm Dataset <===========================" << std::endl;
	test_speed<charm_gen, chaining_hash_table>();
	tpie::log_info() << "===================> Linear Probing, Identity Dataset <=======================" << std::endl;
	test_speed<identity_gen, linear_probing_hash_table>();
	tpie::log_info() << "=======================> Chaining, Identity Dataset <=========================" << std::endl;
	test_speed<identity_gen, chaining_hash_table>();
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic_test<chaining_hash_table>, "chaining")
		.test(basic_test<linear_probing_hash_table>, "linear_probing")
		.test(speed, "speed")
		.test(iterator_test, "iterators")
		.test(hashmap_memory_test(), "memory");
}
