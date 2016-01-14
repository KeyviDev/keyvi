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

#include <tpie/array.h>
#include <tpie/array_view.h>
#include <tpie/bit_array.h>
#include <tpie/array.h>
#include <tpie/concepts.h>
#include <tpie/util.h>
using namespace tpie;

// Method coverage of tpie::array
//
// Method           Covered by unit test
//
// ctor(sz)         basic
// ctor(sz, el)     basic
// operator[]       basic
// at               TODO
// back             frontback
// begin            iterator
// empty            basic
// end              iterator
// find             iterator
// front            frontback
// get              TODO
// rbegin           iterator
// rend             iterator
// resize(sz)       basic
// resize(sz, el)   basic
// size             basic
// swap             TODO
// copy             copy

bool basic_test() {
	array<size_t> hat;
	//Resize
	hat.resize(52, 42);
	TEST_ENSURE(hat.size() == 52, "Wrong size"); 
	for (memory_size_type i=0; i < 52; ++i)
		TEST_ENSURE_EQUALITY(hat[i], 42, "Wrong value");
	
	//Get and set
	for (memory_size_type i=0; i < 52; ++i)
		hat[i] = (i * 104729) % 2251;
  
	const tpie::array<size_t> & hat2(hat);
	for (memory_size_type i=0; i < 52; ++i)
		TEST_ENSURE_EQUALITY(hat2[i], ((i * 104729) % 2251), "Wrong value");

	TEST_ENSURE(!hat.empty(), "Empty");
	hat.resize(0);
	TEST_ENSURE(hat.empty(), "Not empty");
	array<int> a(1,0),b(4,0),c(11,0);
	a[0] = b[0] = c[0] = 1;
	TEST_ENSURE(a[0] && b[0] && c[0], "Wrong value");
	a[0] = b[0] = c[0] = 0;
	TEST_ENSURE(!a[0] && !b[0] && !c[0], "Wrong value")
	return true;
}

class unique_ptr_test_class {
public:
	size_t & dc;
	size_t & cc;
	unique_ptr_test_class(size_t & cc_, size_t & dc_): dc(dc_), cc(cc_) {
		++cc;
	}
	~unique_ptr_test_class() {
		++dc;
	}
	size_t hat() {return 42;}
private:
	unique_ptr_test_class(const unique_ptr_test_class & o): dc(o.dc), cc(o.cc) {}
};


bool unique_ptr_test() {
	size_t s=1234;
	size_t cc=0;
	size_t dc=0;
	array<tpie::unique_ptr<unique_ptr_test_class> > a;
	a.resize(s);
	for(size_t i=0; i < s; ++i) 
		a[i].reset(tpie_new<unique_ptr_test_class, size_t &, size_t &>(cc, dc));
	TEST_ENSURE_EQUALITY(cc, s, "Wrong value");
	TEST_ENSURE_EQUALITY(dc, 0, "Wrong value");
	size_t x=0;
	for(size_t i=0; i < s; ++i) 
		x += a[i]->hat();
	TEST_ENSURE_EQUALITY(x, 42*s, "Wrong value");
	TEST_ENSURE_EQUALITY(cc, s, "Wrong value");
	TEST_ENSURE_EQUALITY(dc, 0, "Wrong value");
	for(size_t i=0; i < s; ++i) 
		a[i].reset(tpie_new<unique_ptr_test_class>(cc, dc));

	TEST_ENSURE_EQUALITY(cc, 2*s, "Wrong value");
	TEST_ENSURE_EQUALITY(dc, s, "Wrong value");
	a.resize(0);
	TEST_ENSURE_EQUALITY(cc, 2*s, "Wrong value");
	TEST_ENSURE_EQUALITY(dc, 2*s, "Wrong value");
	return true;
}

bool basic_bool_test() {
	tpie::bit_array hat;
  
	//Resize
	hat.resize(52, 1);
	TEST_ENSURE(hat.size() == 52, "Wrong size");
	for (size_type i=0; i < 52; ++i)
		TEST_ENSURE(hat[i] == true, "Wrong value");
  
	//Get and set
	return true;
	for (size_type i=0; i < 52; ++i)
		hat[i] = static_cast<bool>(((i * 104729)>>3) % 2);
  
	const tpie::bit_array & hat2(hat);
	for (size_type i=0; i < 52; ++i)
		TEST_ENSURE_EQUALITY(hat2[i], static_cast<bool>(((i * 104729)>>3) % 2), "Wrong value");

	TEST_ENSURE(!hat.empty(), "Empty");
	hat.resize(0);
	TEST_ENSURE(hat.empty(), "Not empty");
	bit_array a(1,0),b(4,0),c(11,0);
	a[0] = b[0] = c[0] = true;
	TEST_ENSURE(a[0] && b[0] && c[0], "Wrong value");
	a[0] = b[0] = c[0] = false;
	TEST_ENSURE(!a[0] && !b[0] && !c[0], "Wrong value");

	return true;
}


bool iterator_test() {
	array<size_t> hat;
	hat.resize(52);

	for (size_type i=0; i < 52; ++i)
		hat[i] = (i * 104729) % 2251;
	{
		array<size_t>::const_iterator i=hat.begin();
		for (size_t j=0; j < 52; ++j) {
			TEST_ENSURE(i != hat.end(), "Should not be end");
			TEST_ENSURE_EQUALITY(*i,((j * 104729) % 2251), "Wrong value");
			++i;
		}
		TEST_ENSURE(i == hat.end(), "Should be end");
	}
	{
		for (size_t j=0; j < 52; ++j) {
			array<size_t>::iterator i=hat.find(j/2)+(j-j/2);
			TEST_ENSURE(i != hat.end(), "Should not be end");
			TEST_ENSURE_EQUALITY(*i, ((j * 104729) % 2251), "Wrong value");
		}
	}
	{
		array<size_t>::reverse_iterator i=hat.rbegin();
		for (size_t j=0; j < 52; ++j) {
			TEST_ENSURE(i != hat.rend(), "Should not be rend");
			TEST_ENSURE_EQUALITY(*i, (((51-j) * 104729) % 2251), "Wrong value"); 
			++i;
		}
		TEST_ENSURE(i == hat.rend(), "Should be rend");
	}

	std::sort(hat.begin(), hat.end());

	{
		// verify order
		// find two elements in the reverse order where one is less than the other
		array<size_t>::reverse_iterator i=std::adjacent_find(hat.rbegin(), hat.rend(), std::less<size_t>());
		TEST_ENSURE(i == hat.rend(), "Should not exist");
	}
	return true;
}

bool iterator_bool_test() {
	bit_array hat;
	hat.resize(52);

	for (size_type i=0; i < 52; ++i)
		hat[i] = static_cast<bool>(((i * 104729)>>7) % 2);
	{
		bit_array::const_iterator i=hat.begin();
		for (int j=0; j < 52; ++j) {
			TEST_ENSURE(i != hat.end(), "End too soon");
			TEST_ENSURE_EQUALITY(*i, static_cast<bool>(((j * 104729)>>7) % 2), "Wrong value");
			++i;
		}
		TEST_ENSURE(i == hat.end(), "End expected");
	}
	{
		bit_array::reverse_iterator i=hat.rbegin();
		for (int j=51; j >= 0; --j) {
			TEST_ENSURE(i != hat.rend(), "End too soon");
			TEST_ENSURE_EQUALITY(*i, static_cast<bool>(((j * 104729)>>7) % 2), "Wrong value");
			++i;
		}
		TEST_ENSURE(i == hat.rend(), "Rend expected");
	}
  	std::sort(hat.begin(), hat.end());
	return true;
}

class array_memory_test: public memory_test {
public:
	array<int> a;
	virtual void alloc() {a.resize(1024*1024*32);}
	virtual void free() {a.resize(0);}
	virtual size_type claimed_size() {
		return static_cast<size_type>(array<int>::memory_usage(1024*1024*32));
	}
};

class array_bool_memory_test: public memory_test {
public:
	bit_array a;
	virtual void alloc() {a.resize(123456);}
	virtual void free() {a.resize(0);}
	virtual size_type claimed_size() {return static_cast<size_type>(bit_array::memory_usage(123456));}
};

bool copyempty() {
	array<char> a(0);
	array<char> b(0);
	array<char> temp = a;
	a = b;
	b = temp;
	return true;
}

bool arrayarray() {
	array<array<int> > a;
	array<int> prototype(1);
	a.resize(1, prototype);
	a.resize(0);
	return true;
}

bool frontback() {
	size_t sz = 9001;
	size_t base = 42;
	array<int> a(sz);
	for (size_t i = 0; i < sz; ++i) {
		a[i] = base+i;
	}
	TEST_ENSURE_EQUALITY(a.front(), static_cast<int>(base), "Wrong front");
	TEST_ENSURE_EQUALITY(a.back(), static_cast<int>(base+sz-1), "Wrong back");
	const array<int> & b = a;
	TEST_ENSURE_EQUALITY(b.front(), static_cast<int>(base), "Wrong front");
	TEST_ENSURE_EQUALITY(b.back(), static_cast<int>(base+sz-1), "Wrong back");
	return true;
}

bool swap_test() {
	{
		array<int> a(42, 42);
		array<int> b(84);
		a.swap(b);
	}
	return true;
}

template <typename T>
class test_allocator {
public:
	static size_t allocated;
	static size_t deallocated;
	static void reset() {
		allocated = 0;
		deallocated = 0;
	}

private:
    typedef tpie::allocator<T> a_t;
    a_t a;
public:
    typedef typename a_t::size_type size_type;
    typedef typename a_t::difference_type difference_type;
	typedef typename a_t::pointer pointer;
	typedef typename a_t::const_pointer const_pointer;
	typedef typename a_t::reference reference;
	typedef typename a_t::const_reference const_reference;
    typedef typename a_t::value_type value_type;

	test_allocator() throw() {}
	test_allocator(const test_allocator & a) throw() {unused(a);}
	template <typename T2>
	test_allocator(const test_allocator<T2> & a) throw() {unused(a);}

    template <class U> struct rebind {typedef test_allocator<U> other;};

    inline T * allocate(size_t size, const void * hint=0) {
		allocated += size;
		return a.allocate(size, hint);
    }

    inline void deallocate(T * p, size_t n) {
		if (p == 0) return;
		deallocated += n;
		a.deallocate(p, n);
    }

    inline size_t max_size() const {return a.max_size();}

#ifdef TPIE_CPP_RVALUE_REFERENCE
#ifdef TPIE_CPP_VARIADIC_TEMPLATES
	template <typename ...TT>
	inline void construct(T * p, TT &&...x) {a.construct(p, x...);}
#else
	template <typename TT>
	inline void construct(T * p, TT && val) {a.construct(p, val);}
#endif
#endif
	inline void construct(T * p) {
#ifdef WIN32
#pragma warning( push )
#pragma warning(disable: 4345)
#endif
		new(p) T();
#ifdef WIN32
#pragma warning( pop )
#endif
	}
    inline void construct(T * p, const T& val) {a.construct(p, val);}
    inline void destroy(T * p) {a.destroy(p);}
	inline pointer address(reference x) const {return &x;}
	inline const_pointer address(const_reference x) const {return &x;}
};

template <typename T> size_t test_allocator<T>::allocated;
template <typename T> size_t test_allocator<T>::deallocated;

bool allocator_test() {
	typedef size_t test_t;
	typedef test_allocator<test_t> alloc;
	typedef tpie::array<test_t, alloc> arr_t;
	alloc::reset();
	TEST_ENSURE_EQUALITY(alloc::allocated, 0, "Wrong value");
	TEST_ENSURE_EQUALITY(alloc::deallocated, 0, "Wrong value");
	size_t sz1 = 42;
	size_t sz2 = 420;
	size_t sz3 = 4200;
	arr_t arr(sz1);
	TEST_ENSURE_EQUALITY(alloc::allocated, sz1, "Wrong value after ctor(sz)");
	TEST_ENSURE_EQUALITY(alloc::deallocated, 0, "Wrong value after ctor(sz)");
	arr.resize(sz2);
	TEST_ENSURE_EQUALITY(alloc::allocated, sz2 + alloc::deallocated, "Wrong value after resize(sz)");
	arr.resize(sz1, 123);
	TEST_ENSURE_EQUALITY(alloc::allocated, sz1 + alloc::deallocated, "Wrong value after resize(sz, elm)");
	tpie::array<test_t> other(sz3);
	for (size_t i = 0; i < sz3; ++i) other[i] = i;
	arr = other;
	TEST_ENSURE_EQUALITY(alloc::allocated, sz3 + alloc::deallocated, "Wrong value after operator=");
	return true;
}
template <typename T>
class rangeit
: public boost::iterator_facade<
	rangeit<T>,
	T,
	boost::random_access_traversal_tag,
	const T &> {
	friend class boost::iterator_core_access;

	T v;

	const T & dereference() const { return v; }
	template <typename U>
	bool equal(const rangeit<U> & other) const { return v == other.v; }
	void increment() { ++v; }
	void decrement() { --v; }
	void advance(size_t n) { v += n; }
	ptrdiff_t distance_to(const rangeit & other) const { return other.v - v; }

public:
	rangeit(T v) : v(v) {}
};

bool copy_test() {
	typedef tpie::array<int> A;
	typedef A::iterator I;
	typedef rangeit<int> R;
	A a(1234, 0xC0FFEE);

	I i1 = a.begin();

	// fill 10 array entries with the integers from 0 to 9
	I i2 = std::copy(R(0), R(10), i1);
	TEST_ENSURE(i1 == a.begin(), "Iterator changed");
	TEST_ENSURE(i2 == a.find(10), "Iterator did not advance");

	std::vector<int> output(10);

	// The following call to std::copy should be `simple` in the libstdc++ sense.
	// If compiled with libstdc++ and running in GDB, set a breakpoint at
	// std::__copy_move_a and verify that __simple == true.
	std::copy(i1, i2, output.begin());
	TEST_ENSURE(std::equal(i1, i2, output.begin()), "Did not copy correctly");
	TEST_ENSURE(std::equal(R(0), R(10), output.begin()), "Did not copy correctly");

	// fill 10 array entries with the integers from 20 to 29
	std::copy(R(20), R(30), output.begin());

	// The following call to std::copy should be `simple` in the libstdc++ sense.
	I i3 = std::copy(output.begin(), output.end(), i1);
	TEST_ENSURE(i2 == i3, "Output iterator did not advance properly");

	A b(1234, 0xC0FFEE);
	// The following call to std::copy should be `simple` in the libstdc++ sense.
	std::copy(a.begin(), a.find(30), b.begin());

	return true;
}

bool from_view_test() {
	size_t bufSz = 20;
	size_t viewOffset = 2;
	size_t viewSize = 10;
	array<size_t> orig(bufSz);
	for (size_t i = 0; i < bufSz; ++i) orig[i] = i;
	array<size_t> copied(array_view<size_t>(orig, viewOffset, viewOffset + viewSize));
	if (copied.size() != viewSize) return false;
	for (size_t i = 0; i < viewSize; ++i) {
		if (copied[i] != orig[i + viewOffset]) return false;
	}
	return true;
}

bool assign_test() {
	tpie::array<size_t> a1(5*1024*1024);
	a1[1000000] = 42;
	tpie::array<size_t> a2;
	a2 = a1;

	// If the default operator= is taken, this should modify a1 as well as a2.
	a2[1000000] = 24;

	// If the default operator= is taken, prevent a double free.
	tpie::array<size_t> a3;
	a2 = a3;
	return a1[1000000] == 42;
}

int main(int argc, char **argv) {
	BOOST_CONCEPT_ASSERT((linear_memory_structure_concept<array<int> >));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<array<int>::const_iterator>));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<array<int>::const_reverse_iterator>));
	BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<array<int>::iterator>));
	BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<array<int>::reverse_iterator>));
	BOOST_CONCEPT_ASSERT((linear_memory_structure_concept<bit_array >));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<bit_array::const_iterator>));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<bit_array::const_reverse_iterator>));

	return tpie::tests(argc, argv, 128)
		.test(basic_test, "basic")
		.test(iterator_test, "iterators")
		.test(unique_ptr_test, "unique_ptr")
		.test(array_memory_test(), "memory")
		.test(basic_bool_test, "bit_basic")
		.test(iterator_bool_test, "bit_iterators")
		.test(array_bool_memory_test(), "bit_memory")
		.test(copyempty, "copyempty")
		.test(arrayarray, "arrayarray")
		.test(frontback, "frontback")
		.test(swap_test, "swap")
		.test(allocator_test, "allocator")
		.test(copy_test, "copy")
		.test(from_view_test, "from_view")
		.test(assign_test, "assign")
		;
}
