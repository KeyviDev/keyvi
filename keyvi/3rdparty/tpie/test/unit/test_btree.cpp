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
#include <tpie/btree.h>
#include <tpie/tempname.h>
#include <algorithm>
#include <set>
#include <map>
#include <numeric>
using namespace tpie;
using namespace std;

template <typename ... TT>
class TA {};

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

template<typename ... TT, typename ... A>
bool basic_test(TA<TT...>, A && ... a) {
	btree<int, TT...> tree(std::forward<A>(a)...);
	set<int> tree2;
	
	std::vector<int> x(12); //34);
	std::iota(x.begin(), x.end(), 0);
	std::random_shuffle(x.begin(), x.end());

	for (size_t v: x) {
		tree.insert(v);
		tree2.insert(v);
		TEST_ENSURE(compare(tree, tree2), "Compare failed on during insert stage.");
		TEST_ENSURE_EQUALITY(tree2.size(), tree.size(), "The tree has the wrong size during insert stage.");
	}

	std::random_shuffle(x.begin(), x.end());
	for (size_t v: x) {
		tree.erase(v);
		tree2.erase(v);
		TEST_ENSURE(compare(tree, tree2), "Compare failed on during erase stage.");
		TEST_ENSURE_EQUALITY(tree2.size(), tree.size(), "The tree has the wrong size during erase stage.");
	}

	return true;
}

template<typename ... TT, typename ... A>
bool iterator_test(TA<TT...>, A && ... a) {
	btree<int, TT...> tree(std::forward<A>(a)...);
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

	auto b1 = tree.begin();
	set<int>::iterator b2 = tree2.begin();
	auto e1 = tree.end();
	set<int>::iterator e2 = tree2.end();
	auto i1 = b1;
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
	friend std::ostream & operator<<(std::ostream & o, const uncomparable & u) {
		return o << u.value;
	}
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
	uncomparable<int> operator()(const item & i) const {return i.key;}
};

template<typename ... TT, typename ... A>
bool key_and_comparator_test(TA<TT...>, A && ... a) {
	std::greater<int> c1;
	comparator<std::greater<int> > comp(c1);
	btree<
		item,
		btree_key<key_extract>,
		btree_comp<comparator<std::greater<int> > >,
		TT...> tree(std::forward<A>(a)..., comp);
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

	auto e1 = tree.end();
	auto i1 = tree.begin();
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

template<typename ... TT, typename ... A>
bool augment_test(TA<TT...>, A && ... a) {
	default_comp c;
	ss_augmenter au;
	btree<int, btree_augment<ss_augmenter>, TT...> tree(std::forward<A>(a)..., c, au);
	std::vector<int> x;
    for (int i=0; i < 1234; ++i) x.push_back(i);
	std::random_shuffle(x.begin(), x.end());
	for (size_t i=0; i < x.size(); ++i) {
		tree.insert(x[i]);
		auto n=tree.root();
	}

	std::sort(x.begin(), x.end());
	auto i1 = tree.begin();
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

template<typename ... TT, typename ... A>
bool build_test(TA<TT...>, A && ... a) {
    default_comp c;
    ss_augmenter au;
    btree_builder<int, btree_augment<ss_augmenter>, TT...> builder(std::forward<A>(a)..., c, au);
	set<int> tree2;

	for (size_t i=0; i < 50000; ++i) {
		builder.push(i);
		tree2.insert(i);
	}

	auto tree = builder.build();
    TEST_ENSURE_EQUALITY(tree2.size(), tree.size(), "The tree has the wrong size");
    TEST_ENSURE(compare(tree, tree2), "Compare failed");

	return true;
}


template<typename ... TT, typename ... A>
bool unordered_test(TA<TT...>, A && ... a) {
	struct item {
		size_t v;
		bool operator==(const item & o) const noexcept {return v == o.v;}
		bool operator!=(const item & o) const noexcept {return v != o.v;}
	};
	
    btree_builder<item, btree_unordered, TT...> builder(std::forward<A>(a)...);
	std::vector<item> tree2;

	for (size_t i=0; i < 50000; ++i) {
		builder.push(item{i});
		tree2.push_back(item{i});
	}

	auto tree = builder.build();
    TEST_ENSURE_EQUALITY(tree2.size(), tree.size(), "The tree has the wrong size");
    TEST_ENSURE(compare(tree, tree2), "Compare failed");

	return true;
}

template<typename ... TT, typename ... A>
bool bound_test(TA<TT...>, A && ... a) {
	btree<int, TT...>  tree(std::forward<A>(a)...);
	set<int> tree2;

	std::vector<int> x;
    for (int i=0; i < 1234; ++i) {
        x.push_back(i);
	}
	std::random_shuffle(x.begin(), x.end());

	for (size_t i=0; i < x.size(); ++i) {
		tree.insert(x[i]);
		tree2.insert(x[i]);

		int r1 = (x.size() * 23) % x.size();
		int r2 = (x.size() * 61) % x.size();

		TEST_ENSURE((tree.lower_bound(r1) == tree.end()) == (tree2.lower_bound(r1) == tree2.end()), "Lower bound compare failed during insert stage");
		TEST_ENSURE(tree.lower_bound(r1) == tree.end() || *tree.lower_bound(r1) ==  *tree2.lower_bound(r1), "Lower bound compare failed during insert stage.");

		TEST_ENSURE((tree.upper_bound(r2) == tree.end()) == (tree2.upper_bound(r2) == tree2.end()), "Upper bound compare failed during insert stage");
		TEST_ENSURE(tree.upper_bound(r2) == tree.end() || *tree.upper_bound(r2) ==  *tree2.upper_bound(r2), "Upper bound compare failed during insert stage.");
	}

	std::random_shuffle(x.begin(), x.end());

	for (size_t i=0; i < x.size(); ++i) {
		tree.erase(x[i]);
		tree2.erase(x[i]);

		int r1 = (x.size() * 23) % x.size();
		int r2 = (x.size() * 61) % x.size();

		TEST_ENSURE((tree.lower_bound(r1) == tree.end()) == (tree2.lower_bound(r1) == tree2.end()), "Lower bound compare failed during insert stage");
		TEST_ENSURE(tree.lower_bound(r1) == tree.end() || *tree.lower_bound(r1) ==  *tree2.lower_bound(r1), "Lower bound compare failed during insert stage.");

		TEST_ENSURE((tree.upper_bound(r2) == tree.end()) == (tree2.upper_bound(r2) == tree2.end()), "Upper bound compare failed during insert stage");
		TEST_ENSURE(tree.upper_bound(r2) == tree.end() || *tree.upper_bound(r2) ==  *tree2.upper_bound(r2), "Upper bound compare failed during insert stage.");
	}

	return true;
}


bool internal_basic_test() {
	return basic_test(TA<btree_internal>());
}

bool internal_iterator_test() {
	return iterator_test(TA<btree_internal>());
}

bool internal_key_and_comparator_test() {
	return key_and_comparator_test(TA<btree_internal>());
}

bool internal_augment_test() {
	return augment_test(TA<btree_internal>());
}

bool internal_build_test() {
	return build_test(TA<btree_internal>());
}

bool internal_static_test() {
	return build_test(TA<btree_internal, btree_static>());
}

bool internal_unordered_test() {
	return unordered_test(TA<btree_internal>());
}

bool internal_bound_test() {
	return bound_test(TA<btree_internal>());
}

bool external_basic_test() {
	temp_file tmp;
	return basic_test(TA<btree_external>(), tmp.path());
}

bool external_iterator_test() {
	temp_file tmp;
	return iterator_test(TA<btree_external>(), tmp.path());
}

bool external_key_and_comparator_test() {
	temp_file tmp;
	return key_and_comparator_test(TA<btree_external>(), tmp.path());
}

bool external_augment_test() {
	temp_file tmp;
	return augment_test(TA<btree_external>(), tmp.path());
}

bool external_build_test() {
    temp_file tmp;
    return build_test(TA<btree_external>(), tmp.path());
}

bool external_bound_test() {
	temp_file tmp;
	return bound_test(TA<btree_external>(), tmp.path());
}

bool serialized_build_test() {
    temp_file tmp;
    return build_test(TA<btree_external, btree_serialized, btree_static>(), tmp.path());
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(internal_basic_test, "internal_basic")
		.test(internal_iterator_test, "internal_iterator")
		.test(internal_key_and_comparator_test, "internal_key_and_compare")
		.test(internal_augment_test, "internal_augment")
        .test(internal_build_test, "internal_build")
		.test(internal_static_test, "internal_static")
		.test(internal_unordered_test, "internal_unordered")
		.test(internal_bound_test, "internal_bound")
		.test(external_basic_test, "external_basic")
		.test(external_iterator_test, "external_iterator")
		.test(external_key_and_comparator_test, "external_key_and_compare")
		.test(external_augment_test, "external_augment")
        .test(external_build_test, "external_build")
		.test(external_bound_test, "external_bound")
		.test(serialized_build_test, "serialized_build");
}


