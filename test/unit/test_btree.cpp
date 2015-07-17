// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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
#include <tpie/tpie.h>
#include <tpie/btree/internal_store.h>
#include <tpie/btree/external_store.h>
#include <tpie/btree/btree.h>
#include <tpie/tempname.h>
#include <algorithm>
#include <set>
#include <map>
using namespace tpie;
using namespace std;

template <typename node_t, typename iter_t> 
bool compare(node_t & n, iter_t & i, iter_t end) {
	if (n.is_leaf()) {
		for (size_t j=0; j < n.count(); ++j) {
			if (i == end) return false;
			if (n.value(j) != *i) return false;
			++i;
		}
	} else {
		for (size_t j=0; j < n.count(); ++j) {
			n.child(j);
			if (!compare(n, i, end)) return false;
			n.parent();
		}
	}
	return true;
}

template <typename tree_t, typename set_t>
bool compare(tree_t & t, set_t & s) {
	if (t.empty()) return s.empty();
	typename tree_t::node_type node=t.root();
	typename set_t::iterator iter=s.begin();
	return compare(node, iter, s.end()) && iter == s.end();
}

template<typename store_constructor>
bool basic_test(store_constructor constructor) {
	btree<typename store_constructor::build_type> tree(constructor.construct());
	set<int> tree2;
	
	std::vector<int> x;
    for (int i=0; i < 1234; ++i) {
        x.push_back(i);
	}
	std::random_shuffle(x.begin(), x.end());
	
	for (size_t i=0; i < x.size(); ++i) {
		tree.insert(x[i]);
		tree2.insert(x[i]);
		TEST_ENSURE(compare(tree, tree2), "Compare failed on during insert stage.");
		TEST_ENSURE_EQUALITY(tree2.size(), tree.size(), "The tree has the wrong size during insert stage.");
	}

	std::random_shuffle(x.begin(), x.end());
	
	for (size_t i=0; i < x.size(); ++i) {
		tree.erase(x[i]);
		tree2.erase(x[i]);
		TEST_ENSURE(compare(tree, tree2), "Compare failed on during erase stage.");
		TEST_ENSURE_EQUALITY(tree2.size(), tree.size(), "The tree has the wrong size during erase stage.");
	}

	return true;
}

template<typename store_constructor>
bool iterator_test(store_constructor constructor) {
	btree<typename store_constructor::build_type> tree(constructor.construct());
	set<int> tree2;
	
	std::vector<int> x;
    for (int i=0; i < 1234; ++i) {
        x.push_back(i);
	}
	std::random_shuffle(x.begin(), x.end());
	
	for (size_t i=0; i < x.size(); ++i) {
		tree.insert(x[i]);
		tree2.insert(x[i]);
		if (tree.size() != tree2.size()) return false;
	}

	typename btree<typename store_constructor::build_type>::iterator b1 = tree.begin();
	set<int>::iterator b2 = tree2.begin();
	typename btree<typename store_constructor::build_type>::iterator e1 = tree.end();
	set<int>::iterator e2 = tree2.end();
	typename btree<typename store_constructor::build_type>::iterator i1 = b1;
	set<int>::iterator i2 = b2;
	
	while (true) {
		if ((i1 == e1) != (i2 == e2)) return false;
		if (i2 == e2) break;
		if (*i1 != *i2) return false;
		++i1;
		++i2;
	}
	
	while (true) {
		--i1;
		--i2;
		if (*i1 != *i2) return false;
		if ( (i1 == b1) != (i2 == b2)) return false;
		if ( (i1 == b1) ) break;
	}

	return true;
}

template <typename T>
class uncomparable {
public:
	T value;
private:
	bool operator <(const T & other) const;
	bool operator <=(const T & other) const;
	bool operator >(const T & other) const;
	bool operator >=(const T & other) const;
	bool operator ==(const T & other) const;
	bool operator !=(const T & other) const;
};

template <typename comp_t>
struct comparator {
	typedef typename comp_t::first_argument_type value_type;
	typedef uncomparable<value_type> first_argument_type;
	typedef uncomparable<value_type> second_argument_type;
	typedef bool result_type;

	comparator(comp_t c): c(c) {}

	bool operator()(const first_argument_type & a, const first_argument_type & b) const {
		return c(a.value, b.value);
	}
	
	comp_t c;
};

struct item {
	uncomparable<int> key;
	int value;
};

struct key_extract {
	typedef uncomparable<int> value_type;
	value_type operator()(const item & i) const {return i.key;}
};

template<typename store_constructor>
bool key_and_comparator_test(store_constructor constructor) {
	std::greater<int> c1;
	comparator<std::greater<int> > comp(c1);
	btree<typename store_constructor::build_type, comparator<std::greater<int> > > tree(constructor.construct(), comp);
	std::map<uncomparable<int>, int, comparator<std::greater<int> > > tree2(comp);
	
	std::vector<item> x;
    for (int i=0; i < 1234; ++i) {
		item it;
		it.key.value = i;
        x.push_back(it);
	}
	std::random_shuffle(x.begin(), x.end());
    for (size_t i=0; i < x.size(); ++i)
		x[i].value = i;
	std::random_shuffle(x.begin(), x.end());

	for (size_t i=0; i < x.size(); ++i) {
		tree.insert(x[i]);
		tree2[x[i].key] = x[i].value;
	}

	typename btree<typename store_constructor::build_type>::iterator e1 = tree.end();
	typename btree<typename store_constructor::build_type>::iterator i1 = tree.begin();
	map<uncomparable<int>, int>::iterator i2 = tree2.begin();
	while (i1 != e1) {
		if (i1->key.value != i2->first.value ||
			i1->value != i2->second) return false;
		++i1;
		++i2;
	}
	
	std::random_shuffle(x.begin(), x.end());
	for (size_t i=0; i < x.size(); ++i) {
		tree.erase(x[i].key);
	}
	return tree.empty();
}

template <typename T1, typename T2>
std::pair<T1, T2> add(std::pair<T1, T2> a, std::pair<T1, T2> b) {
	return std::pair<T1, T2>(a.first + b.first, a.second + b.second);
}

typedef std::pair<size_t, size_t> ss_augment;

ostream & operator<<(ostream & o, const ss_augment & a) {
	return o << "{" << a.first << ", " << a.second << "}";
}

struct ss_augmenter {
	template <typename N>
	ss_augment operator()(const N & node) {
		ss_augment ans(0,0);
		if (node.is_leaf()) {
			for (size_t i=0; i < node.count(); ++i) {
				ans.first++;
				ans.second += node.value(i);
			}
		} else {
			for (size_t i=0; i < node.count(); ++i)
				ans = add(ans, node.get_augmentation(i));
		}
		return ans;
	}
};


template <typename S>
ss_augment rank_sum(const btree_iterator<S> & i) {
	ss_augment ans(0,0);
	size_t idx=i.index();
	btree_node<S> n=i.is_leaf();
	while (true) {
		if (n.is_leaf()) {
			for (size_t i=0; i < idx; ++i) {
				ans.first++;
				ans.second += n.value(i);
			}
		}
		else {
			for (size_t i=0; i < idx; ++i) {
				ans = add(ans, n.get_augmentation(i));
			}
		}
		if (!n.has_parent()) break;
		idx = n.index();
		n.parent();
	}
	return ans;
}

template <typename S>
void print(btree_node<S> & n) {
	if (n.is_leaf()) {
		std::cout << "[";
		for (size_t i=0; i < n.count(); ++i) {
			if (i != 0) std::cout << ", ";
			std::cout << n.value(i);
		}
		std::cout << "]";
		return;
	}
	std::cout << "(";
	for (size_t i=0; i < n.count(); ++i) {
		if (i != 0) std::cout << ", ";
		std::cout << n.get_augmentation(i) << " | ";
		n.child(i);
		print(n);
		n.parent();
	}
	std::cout << ")";
}

template<typename store_constructor>
bool augment_test(store_constructor constructor) {
	std::less<int> c;
	ss_augmenter a;
	btree<typename store_constructor::build_type, std::less<int>, ss_augmenter> tree(constructor.construct(), c, a);
	std::vector<int> x;
    for (int i=0; i < 1234; ++i) x.push_back(i);
	std::random_shuffle(x.begin(), x.end());
	for (size_t i=0; i < x.size(); ++i) {
		tree.insert(x[i]);
		btree_node<typename store_constructor::build_type> n=tree.root();
	}

	std::sort(x.begin(), x.end());
	typename btree<typename store_constructor::build_type>::iterator i1 = tree.begin();
	size_t sum=0;
	for (size_t i=0; i < x.size(); ++i) {
		if (rank_sum(i1) != ss_augment(i, sum)) return false;
		sum += x[i];
		++i1;
	}

	size_t e=x.size()/2;
	std::random_shuffle(x.begin(), x.end());
	for (size_t i=e; i < x.size(); ++i)
		tree.erase(x[i]);
	x.resize(e);
	std::sort(x.begin(), x.end());

	i1 = tree.begin();
	sum=0;
	for (size_t i=0; i < x.size(); ++i) {
		if (rank_sum(i1) != ss_augment(i, sum)) return false;
		sum += x[i];
		++i1;
	}

	return true;
}

template <typename T>
struct internal_store_constructor {
	typedef T build_type;

	build_type construct() {
		return build_type();
	}
};

bool internal_basic_test() {
	return basic_test(internal_store_constructor<btree_internal_store<int> >());
}

bool internal_iterator_test() {
	return iterator_test(internal_store_constructor<btree_internal_store<int> >());
}

bool internal_key_and_comparator_test() {
	return key_and_comparator_test(internal_store_constructor<btree_internal_store<item, empty_augment, key_extract> >());
}

bool internal_augment_test() {
	return augment_test(internal_store_constructor<btree_internal_store<int, ss_augment> >());
}

template <typename T>
struct external_store_constructor {
	typedef T build_type;
	std::string path;

	external_store_constructor(std::string path) : path(path) {}

	build_type construct() {
		return build_type(path);
	}
};

bool external_basic_test() {
	temp_file tmp;
	return basic_test(external_store_constructor<btree_external_store<int> >(tmp.path()));
}

bool external_iterator_test() {
	temp_file tmp;
	return iterator_test(external_store_constructor<btree_external_store<int> >(tmp.path()));
}

bool external_key_and_comparator_test() {
	temp_file tmp;
	return key_and_comparator_test(external_store_constructor<btree_external_store<item, empty_augment, key_extract> >(tmp.path()));
}

bool external_augment_test() {
	temp_file tmp;
	return augment_test(external_store_constructor<btree_external_store<int, ss_augment> >(tmp.path()));
}


int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(internal_basic_test, "internal_basic")
		.test(internal_iterator_test, "internal_iterator")
		.test(internal_key_and_comparator_test, "internal_key_and_compare")
		.test(internal_augment_test, "internal_augment")
		.test(external_basic_test, "external_basic")
		.test(external_iterator_test, "external_iterator")
		.test(external_key_and_comparator_test, "external_key_and_compare")
		.test(external_augment_test, "external_augment");
}


