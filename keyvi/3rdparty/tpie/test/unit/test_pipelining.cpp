// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2011, 2012, 2013 The TPIE development team
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
#include <tpie/pipelining/subpipeline.h>
#include <tpie/file_stream.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <tpie/sysinfo.h>
#include <tpie/pipelining/virtual.h>
#include <tpie/pipelining/serialization.h>
#include <tpie/progress_indicator_arrow.h>
#include <tpie/pipelining/helpers.h>
#include <tpie/pipelining/split.h>
#include <tpie/resource_manager.h>

using namespace tpie;
using namespace tpie::pipelining;

typedef uint64_t test_t;

template <typename dest_t>
struct multiply_t : public node {
	typedef test_t item_type;

	inline multiply_t(dest_t dest, uint64_t factor)
		: dest(std::move(dest))
		, factor(factor)
	{
		set_minimum_memory(17000000);
		add_push_destination(dest);
	}

	virtual void begin() override {
		node::begin();
		log_debug() << "multiply begin with memory " << this->get_available_memory() << std::endl;
	}

	void push(const test_t & item) {
		dest.push(factor*item);
	}

	dest_t dest;
	uint64_t factor;
};

typedef pipe_middle<factory<multiply_t, uint64_t> > multiply;

std::vector<test_t> inputvector;
std::vector<test_t> expectvector;
std::vector<test_t> outputvector;

void setup_test_vectors() {
	inputvector.resize(0); expectvector.resize(0); outputvector.resize(0);
	inputvector.resize(20); expectvector.resize(20);
	for (size_t i = 0; i < 20; ++i) {
		inputvector[i] = i;
		expectvector[i] = i*6;
	}
}

bool check_test_vectors() {
	if (outputvector != expectvector) {
		log_error() << "Output vector does not match expect vector\n"
			<< "Expected: " << std::flush;
		std::vector<test_t>::iterator expectit = expectvector.begin();
		while (expectit != expectvector.end()) {
			log_error()<< *expectit << ' ';
			++expectit;
		}
		log_error()<< '\n'
			<< "Output:   " << std::flush;
		std::vector<test_t>::iterator outputit = outputvector.begin();
		while (outputit != outputvector.end()) {
			log_error()<< *outputit << ' ';
			++outputit;
		}
		log_error()<< std::endl;
		return false;
	}
	return true;
}

bool vector_multiply_test() {
	pipeline p = input_vector(inputvector) | multiply(3) | multiply(2) | output_vector(outputvector);
	p.plot(log_info());
	p();
	return check_test_vectors();
}

bool file_stream_test(stream_size_type items) {
	tpie::temp_file input_file;
	tpie::temp_file output_file;
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		for (stream_size_type i = 0; i < items; ++i) {
			in.write(i);
		}
	}
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		file_stream<test_t> out;
		out.open(output_file.path());
		// p is actually an input_t<multiply_t<multiply_t<output_t<test_t> > > >
		pipeline p = (input(in) | multiply(3) | multiply(2) | output(out));
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open(output_file.path());
		for (stream_size_type i = 0; i < items; ++i) {
			if (i*6 != out.read()) return false;
		}
	}
	return true;
}

bool file_stream_pull_test() {
	tpie::temp_file input_file;
	tpie::temp_file output_file;
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		in.write(1);
		in.write(2);
		in.write(3);
	}
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		file_stream<test_t> out;
		out.open(output_file.path());
		pipeline p = (pull_input(in) | pull_output(out));
		p.get_node_map()->dump(log_info());
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open(output_file.path());
		if (1 != out.read()) return false;
		if (2 != out.read()) return false;
		if (3 != out.read()) return false;
	}
	return true;
}

bool file_stream_alt_push_test() {
	tpie::temp_file input_file;
	tpie::temp_file output_file;
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		in.write(1);
		in.write(2);
		in.write(3);
	}
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		file_stream<test_t> out;
		out.open(output_file.path());
		pipeline p = (input(in) | output(out));
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open(output_file.path());
		if (1 != out.read()) return false;
		if (2 != out.read()) return false;
		if (3 != out.read()) return false;
	}
	return true;
}

bool merge_test() {
	tpie::temp_file input_file;
	tpie::temp_file output_file;
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		pipeline p = input_vector(inputvector) | output(in);
		p.plot(log_info());
		p();
	}
	expectvector.resize(2*inputvector.size());
	for (int i = 0, j = 0, l = inputvector.size(); i < l; ++i) {
		expectvector[j++] = inputvector[i];
		expectvector[j++] = inputvector[i];
	}
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		file_stream<test_t> out;
		out.open(output_file.path());
		std::vector<test_t> inputvector2 = inputvector;
		pipeline p = input_vector(inputvector) | merge(pull_input(in)) | output(out);
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> in;
		in.open(output_file.path());
		pipeline p = input(in) | output_vector(outputvector);
		p.plot(log_info());
		p();
	}
	return check_test_vectors();
}

bool reverse_test() {
	pipeline p1 = input_vector(inputvector) | reverser() | output_vector(outputvector);
	p1.plot_full(log_info());
	p1();
	expectvector = inputvector;
	std::reverse(expectvector.begin(), expectvector.end());

	//reverser<test_t> r(inputvector.size());
	//pipeline p1 = input_vector(inputvector) | r.sink();
	//pipeline p2 = r.source() | output_vector(outputvector);
	//p1.plot(log_info());
	//p1();

	return check_test_vectors();
}

bool internal_reverse_test() {
	pipeline p1 = input_vector(inputvector) | reverser() | output_vector(outputvector);
	p1();
	expectvector = inputvector;
	std::reverse(expectvector.begin(), expectvector.end());

	return check_test_vectors();
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sums the two components of the input pair
///////////////////////////////////////////////////////////////////////////////

template <typename factory_type>
class add_pairs_type {
public:
	template <typename dest_t>
	class type : public tpie::pipelining::node {
	public:
		dest_t dest;
		typename factory_type::constructed_type pullSource;
		typedef uint64_t item_type;

		type(dest_t dest, factory_type && factory)
		: dest(std::move(dest))
		, pullSource(factory.construct())
		{
			add_push_destination(dest);
			add_pull_source(pullSource);
		}

		void push(const item_type &item) {
			dest.push(item + pullSource.pull());
		}
	};
};

template <typename pipe_type>
tpie::pipelining::pipe_middle<tpie::pipelining::tempfactory<add_pairs_type<typename pipe_type::factory_type>, typename pipe_type::factory_type &&> >
add_pairs(pipe_type && pipe) {
	return tpie::pipelining::tempfactory<add_pairs_type<typename pipe_type::factory_type>, typename pipe_type::factory_type &&>(std::move(pipe.factory));
}

///////////////////////////////////////////////////////////////////////////////
/// \brief templated test for the passive reversers
///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool templated_passive_reverse_test(size_t n) {
	std::vector<test_t> input;
	std::vector<test_t> output;

	for(size_t i = 0; i < n; ++i)
		input.push_back(i);

	T p_reverser;
	pipeline p = input_vector(input)
	| fork(p_reverser.input())
	| buffer()
	| add_pairs(p_reverser.output())
	| output_vector(output);

	p();

	TEST_ENSURE_EQUALITY(output.size(), n, "The passive reverser didn't output the correct number of elements");
	for(std::vector<test_t>::iterator i = output.begin(); i != output.end(); ++i)
		TEST_ENSURE_EQUALITY(*i, n-1, "The passive reverser did not output in the correct order");
	return true;
}

bool internal_passive_reverse_test(size_t n) {
	return templated_passive_reverse_test<passive_reverser<test_t> >(n);
}

bool passive_reverse_test(size_t n) {
	return templated_passive_reverse_test<internal_passive_reverser<test_t> >(n);
}

template <typename dest_t>
struct sequence_generator_type : public node {
	typedef size_t item_type;

	sequence_generator_type(dest_t dest, size_t elements, bool reverse)
		: dest(std::move(dest))
		, elements(elements)
		, reverse(reverse)
	{
		add_push_destination(dest);
	}

	virtual void propagate() override {
		forward("items", static_cast<stream_size_type>(elements));
		set_steps(elements);
	}

	virtual void go() override {
		if (reverse) {
			for (size_t i = elements; i > 0; --i) {
				dest.push(i);
				step();
			}
		} else {
			for (size_t i = 1; i <= elements; ++i) {
				dest.push(i);
				step();
			}
		}
	}
private:
	dest_t dest;
	size_t elements;
	bool reverse;
};

typedef pipe_begin<factory<sequence_generator_type, size_t, bool> >
	sequence_generator;

struct sequence_verifier_type : public node {
	typedef size_t item_type;

	sequence_verifier_type(size_t elements, bool * result)
		: elements(elements)
		, expect(1)
		, result(result)
		, bad(false)
	{
		*result = false;
	}

	virtual void propagate() override {
		if (!can_fetch("items")) {
			log_error() << "Sorter did not forward number of items" << std::endl;
			bad = true;
		}
		*result = false;
	}

	inline void push(size_t element) {
		if (element != expect) {
			(bad ? log_debug() : log_error()) << "Got " << element << ", expected " << expect << std::endl;
			bad = true;
		}
		*result = false;
		++expect;
	}

	virtual void end() override {
		if (can_fetch("items")
			&& static_cast<stream_size_type>(elements) != fetch<stream_size_type>("items")) {

			log_error() << "Sorter did not send as many items as promised" << std::endl;
			bad = true;
		}
		*result = !bad;
	}

private:
	size_t elements;
	size_t expect;
	bool * result;
	bool bad;
};

typedef pipe_end<termfactory<sequence_verifier_type, size_t, bool *> >
	sequence_verifier;

bool sort_test(size_t elements) {
	bool result = false;
	pipeline p = sequence_generator(elements, true)
		| sort().name("Test")
		| sequence_verifier(elements, &result);
	p.plot(log_info());
	p();
	return result;
}

bool sort_test_trivial() {
	TEST_ENSURE(sort_test(0), "Cannot sort 0 elements");
	TEST_ENSURE(sort_test(1), "Cannot sort 1 element");
	TEST_ENSURE(sort_test(2), "Cannot sort 2 elements");
	return true;
}

bool sort_test_small() {
	return sort_test(20);
}

bool sort_test_large() {
	return sort_test(300*1024);
}

// This tests that pipe_middle | pipe_middle -> pipe_middle,
// and that pipe_middle | pipe_end -> pipe_end.
// The other tests already test that pipe_begin | pipe_middle -> pipe_middle,
// and that pipe_begin | pipe_end -> pipeline.
bool operator_test() {
	expectvector = inputvector;
	std::reverse(inputvector.begin(), inputvector.end());
	pipeline p = input_vector(inputvector) | ((sort() | sort()) | output_vector(outputvector));
	p.plot(log_info());
	p();
	return check_test_vectors();
}

bool uniq_test() {
	const size_t n = 5;
	inputvector.resize(n*(n+1)/2);
	expectvector.resize(n);
	size_t k = 0;
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j <= i; ++j) {
			inputvector[k++] = i;
		}
		expectvector[i] = i;
	}
	pipeline p = input_vector(inputvector) | pipeuniq() | output_vector(outputvector);
	p.plot(log_info());
	p();
	return check_test_vectors();
}

struct memtest {
	size_t totalMemory;
	size_t minMem1;
	size_t maxMem1;
	size_t minMem2;
	size_t maxMem2;
	double frac1;
	double frac2;

	size_t assigned1;
	size_t assigned2;
};

template <typename dest_t>
class memtest_1 : public node {
	dest_t dest;
	memtest & settings;

public:
	memtest_1(dest_t dest, memtest & settings)
		: dest(std::move(dest))
		, settings(settings)
	{
		add_push_destination(dest);
	}

	void prepare() override {
		set_minimum_memory(settings.minMem1);
		if (settings.maxMem1 > 0)
			set_maximum_memory(settings.maxMem1);
		set_memory_fraction(settings.frac1);
	}

	virtual void set_available_memory(memory_size_type m) override {
		node::set_available_memory(m);
		settings.assigned1 = m;
	}

	virtual void go() override {
	}
};

class memtest_2 : public node {
	memtest & settings;

public:
	memtest_2(memtest & settings)
		: settings(settings)
	{
	}

	void prepare() override {
		set_minimum_memory(settings.minMem2);
		if (settings.maxMem2 > 0)
			set_maximum_memory(settings.maxMem2);
		set_memory_fraction(settings.frac2);
	}

	virtual void set_available_memory(memory_size_type m) override {
		node::set_available_memory(m);
		settings.assigned2 = m;
	}
};

bool memory_test(memtest settings) {
	if (settings.minMem1 + settings.minMem2 > settings.totalMemory) {
		throw tpie::exception("Memory requirements too high");
	}

	const memory_size_type NO_MEM = std::numeric_limits<memory_size_type>::max();
	settings.assigned1 = settings.assigned2 = NO_MEM;

	progress_indicator_null pi;

	pipeline p =
		make_pipe_begin<memtest_1, memtest &>(settings)
		| make_pipe_end<memtest_2, memtest &>(settings);
	p(0, pi, settings.totalMemory, TPIE_FSI);

	log_debug() << "totalMemory " << settings.totalMemory << '\n'
	            << "minMem1     " << settings.minMem1 << '\n'
	            << "maxMem1     " << settings.maxMem1 << '\n'
	            << "minMem2     " << settings.minMem2 << '\n'
	            << "maxMem2     " << settings.maxMem2 << '\n'
	            << "frac1       " << settings.frac1 << '\n'
	            << "frac2       " << settings.frac2 << '\n'
	            << "assigned1   " << settings.assigned1 << '\n'
	            << "assigned2   " << settings.assigned2 << std::endl;

	if (settings.assigned1 == NO_MEM || settings.assigned2 == NO_MEM) {
		log_error() << "No memory assigned" << std::endl;
		return false;
	}

	if (settings.assigned1 + settings.assigned2 > settings.totalMemory) {
		log_error() << "Too much memory assigned" << std::endl;
		return false;
	}

	if (settings.assigned1 < settings.minMem1 || settings.assigned2 < settings.minMem2) {
		log_error() << "Too little memory assigned" << std::endl;
		return false;
	}

	if ((settings.maxMem1 != 0 && settings.assigned1 > settings.maxMem1)
		|| (settings.maxMem2 != 0 && settings.assigned2 > settings.maxMem2)) {
		log_error() << "Too much memory assigned" << std::endl;
		return false;
	}

	const double EPS = 1e-9;
	const size_t min1 = settings.minMem1;
	const size_t max1 = (settings.maxMem1 == 0) ? settings.totalMemory : settings.maxMem1;
	const size_t min2 = settings.minMem2;
	const size_t max2 = (settings.maxMem2 == 0) ? settings.totalMemory : settings.maxMem2;
	const size_t m1 = settings.assigned1;
	const size_t m2 = settings.assigned2;
	const double f1 = settings.frac1;
	const double f2 = settings.frac2;
	if ((min1 < m1 && m1 < max1) && (min2 < m2 && m2 < max2)
		&& std::abs(m1 * f2 - m2 * f1) > EPS)
	{
		log_error() << "Fractions not honored" << std::endl;
		return false;
	}

	return true;
}

void memory_test_shorthand(teststream & ts, size_t totalMemory, size_t minMem1, size_t maxMem1, size_t minMem2, size_t maxMem2, double frac1, double frac2) {
	ts << "(" << totalMemory << ", " << minMem1 << ", " << maxMem1 << ", " << minMem2 << ", " << maxMem2 << ", " << frac1 << ", " << frac2 << ")";
	memtest settings;
	settings.totalMemory = totalMemory;
	settings.minMem1 = minMem1;
	settings.maxMem1 = maxMem1;
	settings.minMem2 = minMem2;
	settings.maxMem2 = maxMem2;
	settings.frac1 = frac1;
	settings.frac2 = frac2;
	ts << result(memory_test(settings));
}

void memory_test_multi(teststream & ts) {
	//                        total   min1   max1   min2   max2  frac1  frac2
	memory_test_shorthand(ts,  2000,     0,     0,     0,     0,   1.0,   1.0);
	memory_test_shorthand(ts,  2000,   800,     0,   800,     0,   1.0,   1.0);
	memory_test_shorthand(ts,  4000,  1000,     0,  1000,     0,   0.0,   0.0);
	memory_test_shorthand(ts,  2000,     0,     0,     0,     0,   0.0,   1.0);
	memory_test_shorthand(ts,  2000,   500,     0,     0,     0,   0.0,   1.0);
	memory_test_shorthand(ts,  2000,   500,   700,     0,     0,   1.0,   1.0);
	memory_test_shorthand(ts,  2000,     0,   700,     0,   500,   1.0,   1.0);
	memory_test_shorthand(ts,  2000,     0,  2000,     0,  2000,   1.0,   1.0);
	memory_test_shorthand(ts,  2000,   200,  2000,     0,  2000,   1.0,   1.0);
}

bool fork_test() {
	expectvector = inputvector;
	pipeline p = input_vector(inputvector).name("Input vector") | fork(output_vector(outputvector)) | null_sink<test_t>();
	p();
	return check_test_vectors();
}

template <typename dest_t>
struct buffer_node_t : public node {
	typedef typename dest_t::item_type item_type;

	inline buffer_node_t(dest_t dest)
		: dest(std::move(dest))
	{
		add_dependency(dest);
	}

	inline void push(const item_type & item) {
		dest.push(item);
	}

	dest_t dest;
};

typedef pipe_middle<factory<buffer_node_t> > buffer_node;

struct merger_memory : public memory_test {
	typedef int test_t;
	typedef plain_store::specific<test_t> specific_store_t;

	size_t n;
	array<file_stream<test_t> > inputs;
	merger<specific_store_t, std::less<test_t> > m;

	inline merger_memory(size_t n)
		: n(n)
		, m(std::less<test_t>(), specific_store_t())
	{
		inputs.resize(n);
		for (size_t i = 0; i < n; ++i) {
			inputs[i].open();
			inputs[i].write(static_cast<test_t>(n-i));
			inputs[i].seek(0);
		}
	}

	virtual void alloc() {
		m.reset(inputs, 1);
	}

	virtual void free() {
		m.reset();
	}

	virtual void use() {
		test_t prev = m.pull();
		for (size_t i = 1; i < n; ++i) {
			test_t it = m.pull();
			if (prev > it) {
				log_error() << "Merger returns items out of order in memory test" << std::endl;
			}
			prev = it;
		}
	}

	virtual size_type claimed_size() {
		return static_cast<size_type>(merger<specific_store_t, std::less<test_t> >::memory_usage(n));
	}
};

bool merger_memory_test(size_t n) {
	merger_memory m(n);
	return m();
}

struct my_item {
	my_item() : v1(42), v2(9001) {}
	short v1;
	int v2;
};

template <typename dest_t>
struct FF1 : public node {
	dest_t dest;
	FF1(dest_t dest) : dest(std::move(dest)) {
		add_push_destination(dest);
	}
	virtual void propagate() override {
		my_item i;
		i.v1 = 1;
		forward("my_item", i);
	}
	virtual void go() override {
	}
};

template <typename dest_t>
struct FF2 : public node {
	dest_t dest;
	FF2(dest_t dest) : dest(std::move(dest)) {
		add_push_destination(dest);
	}
};

bool fetch_forward_result;

struct FF3 : public node {
	FF3() {
	}
	virtual void propagate() override {
		if (!can_fetch("my_item")) {
			log_error() << "Cannot fetch my_item" << std::endl;
			fetch_forward_result = false;
			return;
		}
		my_item i = fetch<my_item>("my_item");
		if (i.v1 != 1) {
			log_error() << "Wrong answer" << std::endl;
			fetch_forward_result = false;
			return;
		}
	}
};

bool fetch_forward_test() {
	fetch_forward_result = true;
	pipeline p = make_pipe_begin<FF1>()
		| make_pipe_middle<FF2>()
		| make_pipe_end<FF3>()
		;
	p.plot(log_info());
	p.forward<int>("test", 42);
	p();
	if (!fetch_forward_result) return false;
	if (p.fetch<int>("test") != 42) {
		log_error() << "Something went wrong" << std::endl;
		return false;
	}
	return true;
}

template <typename dest_t>
struct FFB1 : public node {
	dest_t dest;
	FFB1(dest_t dest) : dest(std::move(dest)) {}

	virtual void propagate() override {
		forward("my_item", 42, 2);
	}

	virtual void go() override {
	}
};

bool bound_fetch_forward_result;

template <typename dest_t>
struct FFB2 : public node {
	dest_t dest;
	FFB2(dest_t dest) : dest(std::move(dest)) {}

	virtual void propagate() override {
		if (!can_fetch("my_item")) {
			log_error() << "Cannot fetch my_item" << std::endl;
			bound_fetch_forward_result = false;
			return;
		}
		int i = fetch<int>("my_item");
		if (i != 42) {
			log_error() << "Wrong answer" << std::endl;
			bound_fetch_forward_result = false;
			return;
		}
	}
};

struct FFB3 : public node {
	virtual void propagate() override {
		if (can_fetch("my_item")) {
			log_error() << "Should not be able to fetch my_item" << std::endl;
			bound_fetch_forward_result = false;
			return;
		}
	}
};

bool bound_fetch_forward_test() {
	bound_fetch_forward_result = true;
	pipeline p = make_pipe_begin<FFB1>()
		| make_pipe_middle<FFB2>()
		| make_pipe_middle<FFB2>()
		| make_pipe_end<FFB3>()
		;
	p.plot(log_info());
	p.forward<int>("test", 7);
	p();
	if (!bound_fetch_forward_result) return false;
	if (p.fetch<int>("test") != 7) {
		log_error() << "Something went wrong" << std::endl;
		return false;
	}
	return true;
}

bool forward_unique_ptr_result = true;

template <typename dest_t>
struct FUP1 : public node {
	dest_t dest;
	FUP1(dest_t dest) : dest(std::move(dest)) {}

	virtual void propagate() override {
		forward("item", std::unique_ptr<int>(new int(293)));
	}

	virtual void go() override {
	}
};

struct FUP2 : public node {
	virtual void propagate() override {
		if (!can_fetch("item")) {
			log_error() << "Cannot fetch item" << std::endl;
			forward_unique_ptr_result = false;
			return;
		}
		auto &p = fetch<std::unique_ptr<int>>("item");
		if (*p != 293) {
			log_error() << "Expected 293, not " << *p << std::endl;
			forward_unique_ptr_result = false;
			return;
		}
	}
};

bool forward_unique_ptr_test() {
	std::unique_ptr<int> ptr(new int(1337));
	pipeline p = make_pipe_begin<FUP1>()
		| make_pipe_end<FUP2>();
	p.plot(log_info());
	p.forward("ptr", std::move(ptr));
	p();
	if (!forward_unique_ptr_result) return false;
	if (!p.can_fetch("ptr")) {
		log_error() << "Cannot fetch ptr" << std::endl;
		return false;
	}
	auto &ptr2 = p.fetch<std::unique_ptr<int>>("ptr");
	if (*ptr2 != 1337) {
		log_error() << "Expected 1337, not " << *ptr2 << std::endl;
		return false;
	}
	return true;
}

bool forward_multiple_pipelines_test() {
	passive_sorter<int> ps;
	pipeline p = input_vector(std::vector<int>{3, 2, 1}) | ps.input();
	p.forward("test", 8);
	pipeline p_ = input_vector(std::vector<int>{5, 6, 7}) | add_pairs(ps.output()) | null_sink<int>();
	p();
	int val = p_.fetch<int>("test");
	return val == 8;
}

bool pipe_base_forward_result = true;

struct PBF_base : public node {
	int n;
	PBF_base(int n) : n(n) {}

	virtual void prepare() override {
		for (int i = 1; i <= 3; i++) {
			std::string item = "item" + std::to_string(i);
			bool fetchable = can_fetch(item);
			if (n < i) {
				if (fetchable) {
					log_error() << "Pipe segment " << n
								<< " could fetch item " << i << "." << std::endl;
					pipe_base_forward_result = false;
				}
			} else {
				if (!fetchable) {
					log_error() << "Pipe segment " << n
								<< " couldn't fetch item " << i << "." << std::endl;
					pipe_base_forward_result = false;
				}
				int value = fetch<int>(item);
				if (value != i) {
					log_error() << "Pipe segment " << n
								<< " fetched item " << i
								<< " with value " << value << "." << std::endl;
					pipe_base_forward_result = false;
				}
			}
		}
	}
};

template <typename dest_t>
struct PBF : public PBF_base {
	dest_t dest;
	PBF(dest_t dest, int n) : PBF_base(n), dest(std::move(dest)) {}

	virtual void go() override {
	}
};

bool pipe_base_forward_test() {
	pipeline p = make_pipe_begin<PBF>(1).forward("item1", 1)
		| make_pipe_middle<PBF>(2).forward("item2", 2)
		| make_pipe_end<PBF_base>(3).forward("item3", 3);
	p();

	return pipe_base_forward_result;
}

// Assume that dest_t::item_type is a reference type.
// Push a dereferenced zero pointer to the destination.
template <typename dest_t>
class push_zero_t : public node {
	dest_t dest;
public:
	typedef typename dest_t::item_type item_type;
private:
	// Type of pointer to dereference.
	typedef typename std::remove_reference<item_type>::type * ptr_type;
public:
	push_zero_t(dest_t dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
	}

	virtual void go() /*override*/ {
		dest.push(*static_cast<ptr_type>(0));
	}
};

typedef pipe_begin<factory<push_zero_t> > push_zero;

bool virtual_test() {
	pipeline p = virtual_chunk_begin<test_t>(input_vector(inputvector))
		| virtual_chunk<test_t, test_t>(multiply(3) | multiply(2))
		| virtual_chunk<test_t, test_t>()
		| virtual_chunk_end<test_t>(output_vector(outputvector));
	p.plot(log_info());
	p();
	return check_test_vectors();
}

bool virtual_fork_test() {
	pipeline p = virtual_chunk_begin<test_t>(input_vector(inputvector))
		| vfork(virtual_chunk_end<test_t>(output_vector(outputvector)))
		| virtual_chunk_end<test_t>(output_vector(outputvector));
	p.plot_full(log_info());
	p();
	expectvector.resize(inputvector.size() * 2);
	for (size_t i = 0; i < inputvector.size(); ++i) {
		expectvector[2*i] = expectvector[2*i+1] = inputvector[i];
	}
	return check_test_vectors();
}

struct prepare_result {
	prepare_result()
		: t(0)
	{
	}

	memory_size_type memWanted1;
	memory_size_type memWanted2;
	memory_size_type memWanted3;

	memory_size_type memGotten1;
	memory_size_type memGotten2;
	memory_size_type memGotten3;

	size_t t;
	size_t prep1;
	size_t prep2;
	size_t prep3;
	size_t propagate1;
	size_t propagate2;
	size_t propagate3;
	size_t begin1;
	size_t begin2;
	size_t begin3;
	size_t end1;
	size_t end2;
	size_t end3;
};

std::ostream & operator<<(std::ostream & os, const prepare_result & r) {
	return os
		<< "memWanted1: " << r.memWanted1 << '\n'
		<< "memWanted2: " << r.memWanted2 << '\n'
		<< "memWanted3: " << r.memWanted3 << "\n\n"
		<< "memGotten1: " << r.memGotten1 << '\n'
		<< "memGotten2: " << r.memGotten2 << '\n'
		<< "memGotten3: " << r.memGotten3 << "\n\n"
		<< "t:          " << r.t << '\n'
		<< "prep1:      " << r.prep1 << '\n'
		<< "prep2:      " << r.prep2 << '\n'
		<< "prep3:      " << r.prep3 << '\n'
		<< "begin1:     " << r.begin1 << '\n'
		<< "begin2:     " << r.begin2 << '\n'
		<< "begin3:     " << r.begin3 << '\n'
		<< "end1:       " << r.end1 << '\n'
		<< "end2:       " << r.end2 << '\n'
		<< "end3:       " << r.end3 << '\n'
		;
}

template <typename dest_t>
class prepare_begin_type : public node {
	dest_t dest;
	prepare_result & r;
public:
	typedef void * item_type;

	prepare_begin_type(dest_t dest, prepare_result & r)
		: dest(std::move(dest))
		, r(r)
	{
		add_push_destination(dest);
	}

	virtual void prepare() override {
		log_debug() << "Prepare 1" << std::endl;
		r.prep1 = r.t++;
		set_minimum_memory(r.memWanted1);
		forward("t", r.t);
	}

	virtual void propagate() override {
		log_debug() << "Propagate 1" << std::endl;
		r.propagate1 = r.t++;
		r.memGotten1 = get_available_memory();
		forward("t", r.t);
	}

	virtual void begin() override {
		log_debug() << "Begin 1" << std::endl;
		r.begin1 = r.t++;
	}

	virtual void go() override {
		// We don't test go()/push() in this unit test.
	}

	virtual void set_available_memory(memory_size_type mem) override {
		node::set_available_memory(mem);
		log_debug() << "Begin memory " << mem << std::endl;
	}

	virtual void end() override {
		r.end1 = r.t++;
	}
};

typedef pipe_begin<factory<prepare_begin_type, prepare_result &> > prepare_begin;

template <typename dest_t>
class prepare_middle_type : public node {
	dest_t dest;
	prepare_result & r;
public:
	typedef void * item_type;

	prepare_middle_type(dest_t dest, prepare_result & r)
		: dest(std::move(dest))
		, r(r)
	{
		add_push_destination(dest);
	}

	virtual void prepare() override {
		log_debug() << "Prepare 2" << std::endl;
		if (!can_fetch("t")) {
			log_error() << "Couldn't fetch time variable in middle::prepare" << std::endl;
		} else if (fetch<size_t>("t") != r.t) {
			log_error() << "Time is wrong" << std::endl;
		}
		r.prep2 = r.t++;
		set_minimum_memory(r.memWanted2);
		forward("t", r.t);
	}

	virtual void propagate() override {
		log_debug() << "Propagate 2" << std::endl;
		if (!can_fetch("t")) {
			log_error() << "Couldn't fetch time variable in middle::propagate" << std::endl;
		} else if (fetch<size_t>("t") != r.t) {
			log_error() << "Time is wrong" << std::endl;
		}
		r.propagate2 = r.t++;
		r.memGotten2 = get_available_memory();
		forward("t", r.t);
	}

	virtual void begin() override {
		log_debug() << "Begin 2" << std::endl;
		r.begin2 = r.t++;
	}

	virtual void end() override {
		r.end2 = r.t++;
	}
};

typedef pipe_middle<factory<prepare_middle_type, prepare_result &> > prepare_middle;

class prepare_end_type : public node {
	prepare_result & r;
public:
	typedef void * item_type;

	prepare_end_type(prepare_result & r)
		: r(r)
	{
	}

	virtual void prepare() override {
		log_debug() << "Prepare 3" << std::endl;
		if (!can_fetch("t")) {
			log_error() << "Couldn't fetch time variable in end::prepare" << std::endl;
		} else if (fetch<size_t>("t") != r.t) {
			log_error() << "Time is wrong" << std::endl;
		}
		r.prep3 = r.t++;
		set_minimum_memory(r.memWanted3);
	}

	virtual void propagate() override {
		log_debug() << "Propagate 3" << std::endl;
		if (!can_fetch("t")) {
			log_error() << "Couldn't fetch time variable in end::propagate" << std::endl;
		} else if (fetch<size_t>("t") != r.t) {
			log_error() << "Time is wrong" << std::endl;
		}
		r.propagate3 = r.t++;
		r.memGotten3 = get_available_memory();
	}

	virtual void begin() override {
		log_debug() << "Begin 3" << std::endl;
		r.begin3 = r.t++;
	}

	virtual void end() override {
		r.end3 = r.t++;
	}
};

typedef pipe_end<termfactory<prepare_end_type, prepare_result &> > prepare_end;

bool prepare_test() {
	prepare_result r;
	r.memWanted1 = 23;
	r.memWanted2 = 45;
	r.memWanted3 = 67;

	pipeline p = prepare_begin(r)
		| prepare_middle(r)
		| prepare_end(r);
	p();
	log_debug() << r << std::endl;
	TEST_ENSURE(r.prep1      == 0,  "Prep 1 time is wrong");
	TEST_ENSURE(r.prep2      == 1,  "Prep 2 time is wrong");
	TEST_ENSURE(r.prep3      == 2,  "Prep 3 time is wrong");
	TEST_ENSURE(r.propagate1 == 3,  "Propagate 1 time is wrong");
	TEST_ENSURE(r.propagate2 == 4,  "Propagate 2 time is wrong");
	TEST_ENSURE(r.propagate3 == 5,  "Propagate 3 time is wrong");
	TEST_ENSURE(r.begin3     == 6,  "Begin 3 time is wrong");
	TEST_ENSURE(r.begin2     == 7,  "Begin 2 time is wrong");
	TEST_ENSURE(r.begin1     == 8,  "Begin 1 time is wrong");
	TEST_ENSURE(r.end1       == 9,  "End 1 time is wrong");
	TEST_ENSURE(r.end2       == 10, "End 2 time is wrong");
	TEST_ENSURE(r.end3       == 11, "End 3 time is wrong");
	TEST_ENSURE(r.t          == 12, "Time is wrong after execution");

	TEST_ENSURE(r.memGotten1 == r.memWanted1, "Memory assigned to 1 is wrong");
	TEST_ENSURE(r.memGotten2 == r.memWanted2, "Memory assigned to 2 is wrong");
	TEST_ENSURE(r.memGotten3 == r.memWanted3, "Memory assigned to 3 is wrong");

	return true;
}

namespace end_time {

struct result {
	size_t t;

	size_t end1;
	size_t end2;

	friend std::ostream & operator<<(std::ostream & os, result & r) {
		return os
			<< "end1 = " << r.end1 << '\n'
			<< "end2 = " << r.end2 << '\n'
			<< "t    = " << r.t << '\n'
			<< std::endl;
	}
};

class begin_type : public node {
	result & r;

public:
	begin_type(result & r) : r(r) {
	}

	virtual void end() override {
		r.end1 = r.t++;
	}
};

typedef pullpipe_begin<termfactory<begin_type, result &> > begin;

template <typename dest_t>
class end_type : public node {
	result & r;
	dest_t dest;

public:
	end_type(dest_t dest, result & r) : r(r), dest(std::move(dest)) {
		add_pull_source(dest);
	}

	virtual void go() override {
	}

	virtual void end() override {
		r.end2 = r.t++;
	}
};

typedef pullpipe_end<factory<end_type, result &> > end;


bool test() {
	result r;
	r.t = 0;
	pipeline p = begin(r) | end(r);
	p.plot(log_info());
	p();
	log_debug() << r;
	TEST_ENSURE(r.end2 == 0, "End 2 time wrong");
	TEST_ENSURE(r.end1 == 1, "End 1 time wrong");
	TEST_ENSURE(r.t    == 2, "Time wrong");
	return true;
}

} // namespace end_time

bool pull_iterator_test() {
	outputvector.resize(inputvector.size());
	expectvector = inputvector;
	pipeline p =
		pull_input_iterator(inputvector.begin(), inputvector.end())
		| pull_output_iterator(outputvector.begin());
	p.plot(log_info());
	p();
	return check_test_vectors();
}

bool push_iterator_test() {
	outputvector.resize(inputvector.size());
	expectvector = inputvector;
	pipeline p =
		push_input_iterator(inputvector.begin(), inputvector.end())
		| push_output_iterator(outputvector.begin());
	p.plot(log_info());
	p();
	return check_test_vectors();
}

template <typename dest_t>
class multiplicative_inverter_type : public node {
	dest_t dest;
	const size_t p;

public:
	typedef size_t item_type;

	multiplicative_inverter_type(dest_t dest, size_t p)
		: dest(std::move(dest))
		, p(p)
	{
		add_push_destination(dest);
		set_steps(p);
	}

	void push(size_t n) {
		size_t i;
		for (i = 0; (i*n) % p != 1; ++i);
		dest.push(i);
		step();
	}
};

typedef pipe_middle<factory<multiplicative_inverter_type, size_t> > multiplicative_inverter;

bool parallel_test(size_t modulo) {
	bool result = false;
	pipeline p = sequence_generator(modulo-1, true)
		| parallel(multiplicative_inverter(modulo))
		| sort()
		| sequence_verifier(modulo-1, &result);
	p.plot(log_info());
	tpie::progress_indicator_arrow pi("Parallel", 1);
	p(modulo-1, pi, TPIE_FSI);
	return result;
}

bool parallel_ordered_test(size_t modulo) {
	bool result = false;
	pipeline p = sequence_generator(modulo-1, false)
		| parallel(multiplicative_inverter(modulo) | multiplicative_inverter(modulo), maintain_order)
		| sequence_verifier(modulo-1, &result);
	p.plot(log_info());
	tpie::progress_indicator_arrow pi("Parallel", 1);
	p(modulo-1, pi, TPIE_FSI);
	return result;
}

template <typename dest_t>
class Monotonic : public node {
	dest_t dest;
	test_t sum;
	test_t chunkSize;
public:
	typedef test_t item_type;
	Monotonic(dest_t dest, test_t sum, test_t chunkSize)
		: dest(std::move(dest))
		, sum(sum)
		, chunkSize(chunkSize)
	{
		add_push_destination(dest);
	}

	virtual void go() override {
		while (sum > chunkSize) {
			dest.push(chunkSize);
			sum -= chunkSize;
		}
		if (sum > 0) {
			dest.push(sum);
			sum = 0;
		}
	}
};

typedef pipe_begin<factory<Monotonic, test_t, test_t> > monotonic;

template <typename dest_t>
class Splitter : public node {
	dest_t dest;
public:
	typedef test_t item_type;
	Splitter(dest_t dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
	}

	void push(test_t item) {
		while (item > 0) {
			dest.push(1);
			--item;
		}
	}
};

typedef pipe_middle<factory<Splitter> > splitter;

class Summer : public node {
	test_t & result;
public:
	typedef test_t item_type;
	Summer(test_t & result)
		: result(result)
	{
	}

	void push(test_t item) {
		result += item;
	}
};

typedef pipe_end<termfactory<Summer, test_t &> > summer;

bool parallel_multiple_test() {
	test_t sumInput = 1000;
	test_t sumOutput = 0;
	pipeline p = monotonic(sumInput, 5) | parallel(splitter()) | summer(sumOutput);
	p.plot();
	p();
	if (sumInput != sumOutput) {
		log_error() << "Expected sum " << sumInput << ", got " << sumOutput << std::endl;
		return false;
	} else {
		return true;
	}
}

template <typename dest_t>
class buffering_accumulator_type : public node {
	dest_t dest;
	test_t inputs;

public:
	static const test_t bufferSize = 8;

	typedef test_t item_type;

	buffering_accumulator_type(dest_t dest)
		: dest(std::move(dest))
		, inputs(0)
	{
		add_push_destination(dest);
	}

	void push(test_t item) {
		inputs += item;
		if (inputs >= bufferSize) flush_buffer();
	}

	virtual void end() override {
		if (inputs > 0) flush_buffer();
	}

private:
	void flush_buffer() {
		while (inputs > 0) {
			dest.push(1);
			--inputs;
		}
	}
};

typedef pipe_middle<factory<buffering_accumulator_type> > buffering_accumulator;

bool parallel_own_buffer_test() {
	test_t sumInput = 64;
	test_t sumOutput = 0;
	pipeline p =
		monotonic(sumInput, 1)
		| parallel(buffering_accumulator(), arbitrary_order, 1, 2)
		| summer(sumOutput);
	p.plot();
	p();
	if (sumInput != sumOutput) {
		log_error() << "Expected sum " << sumInput << ", got " << sumOutput << std::endl;
		return false;
	} else {
		return true;
	}
}

template <typename dest_t>
class noop_initiator_type : public node {
	dest_t dest;
public:
	noop_initiator_type(dest_t dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
	}

	virtual void go() override {
		// noop
	}
};

typedef pipe_begin<factory<noop_initiator_type> > noop_initiator;

template <typename dest_t>
class push_in_end_type : public node {
	dest_t dest;
public:
	typedef test_t item_type;

	push_in_end_type(dest_t dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
	}

	void push(item_type) {
	}

	virtual void end() override {
		for (test_t i = 0; i < 100; ++i) {
			dest.push(1);
		}
	}
};

typedef pipe_middle<factory<push_in_end_type> > push_in_end;

bool parallel_push_in_end_test() {
	test_t sumOutput = 0;
	pipeline p =
		noop_initiator()
		| parallel(push_in_end(), arbitrary_order, 1, 10)
		| summer(sumOutput);
	p.plot(log_info());
	p();
	if (sumOutput != 100) {
		log_error() << "Wrong result, expected 100, got " << sumOutput << std::endl;
		return false;
	}
	return true;
}

template <typename dest_t>
class step_begin_type : public node {
	dest_t dest;
	static const size_t items = 256*1024*1024;

public:
	typedef typename dest_t::item_type item_type;

	step_begin_type(dest_t dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
	}

	virtual void propagate() override {
		forward<stream_size_type>("items", items);
	}

	virtual void go() override {
		for (size_t i = 0; i < items; ++i) {
			dest.push(item_type());
		}
	}
};

typedef pipe_begin<factory<step_begin_type> > step_begin;

template <typename dest_t>
class step_middle_type : public node {
	dest_t dest;

public:
	typedef typename dest_t::item_type item_type;

	step_middle_type(dest_t dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
	}

	virtual void propagate() override {
		if (!can_fetch("items")) throw tpie::exception("Cannot fetch items");
		set_steps(fetch<stream_size_type>("items"));
	}

	void push(item_type i) {
		step();
		dest.push(i);
	}
};

typedef pipe_middle<factory<step_middle_type> > step_middle;

class step_end_type : public node {
public:
	typedef size_t item_type;

	void push(item_type) {
	}
};

typedef pipe_end<termfactory<step_end_type> > step_end;
bool parallel_step_test() {
	pipeline p = step_begin() | parallel(step_middle()) | step_end();
	progress_indicator_arrow pi("Test", 0);
	p(get_memory_manager().available(), pi, TPIE_FSI);
	return true;
}

class virtual_cref_item_type_test_helper {
public:
	template <typename In, typename Expect>
	class t {
		typedef typename tpie::pipelining::bits::maybe_add_const_ref<In>::type Out;
	public:
		typedef typename std::enable_if<std::is_same<Out, Expect>::value, int>::type u;
	};
};

bool virtual_cref_item_type_test() {
	typedef virtual_cref_item_type_test_helper t;
	t::t<int, const int &>::u t1 = 1;
	t::t<int *, int *>::u t2 = 2;
	t::t<int &, int &>::u t3 = 3;
	t::t<const int *, const int *>::u t4 = 4;
	t::t<const int &, const int &>::u t5 = 5;
	return t1 + t2 + t3 + t4 + t5 > 0;
}

struct no_move_tag {};

class node_map_tester : public node {
	friend class node_map_tester_factory;

	std::unique_ptr<node_map_tester> dest;
public:

	node_map_tester() {
		set_name("Node map tester leaf");
	}

	node_map_tester(node_map_tester && copy, no_move_tag)
		: dest(new node_map_tester(std::move(copy)))
	{
		set_name("Node map tester non-leaf");
	}

	void add(std::vector<node_map_tester *> & v) {
		v.push_back(this);
		if (dest.get()) dest->add(v);
	}

	virtual void go() override {
		// Nothing to do.
	}
};

class node_map_tester_factory : public factory_base {
	size_t nodes;
	std::string edges;

public:
	typedef node_map_tester constructed_type;

	node_map_tester_factory(size_t nodes, const std::string & edges)
		: nodes(nodes)
		, edges(edges)
	{
		if (edges.size() != nodes*nodes) throw std::invalid_argument("edges has wrong size");
	}

	node_map_tester construct() {
		std::vector<node_map_tester *> nodes;
		node_map_tester node;
		this->init_node(node);
		for (size_t i = 1; i < this->nodes; ++i) {
			node_map_tester n2(std::move(node), no_move_tag());
			node = std::move(n2);
			this->init_node(node);
		}
		node.add(nodes);
		for (size_t i = 0, idx = 0; i < this->nodes; ++i) {
			for (size_t j = 0; j < this->nodes; ++j, ++idx) {
				switch (edges[idx]) {
					case '.':
						break;
					case '>':
						log_debug() << i << " pushes to " << j << std::endl;
						nodes[i]->add_push_destination(*nodes[j]);
						break;
					case '<':
						log_debug() << i << " pulls from " << j << std::endl;
						nodes[i]->add_pull_source(*nodes[j]);
						break;
					case '-':
						nodes[i]->add_dependency(*nodes[j]);
						break;
					default:
						throw std::invalid_argument("Bad char");
				}
			}
		}
		return node;
	}
};

bool node_map_test(size_t nodes, bool acyclic, const std::string & edges) {
	node_map_tester_factory fact(nodes, edges);
	pipeline p =
		tpie::pipelining::bits::pipeline_impl<node_map_tester_factory>(fact);
	p.plot(log_info());
	try {
		p();
	} catch (const exception &) {
		return !acyclic;
	}
	return acyclic;
}

void node_map_multi_test(teststream & ts) {
	ts << "push_basic" << result
		(node_map_test
		 (4, true,
		  ".>.."
		  "..>."
		  "...>"
		  "...."));
	ts << "pull_basic" << result
		(node_map_test
		 (4, true,
		  "...."
		  "<..."
		  ".<.."
		  "..<."));
	ts << "phase_basic" << result
		(node_map_test
		 (3, true,
		  "..."
		  "-.."
		  ".-."));
	ts << "self_push" << result
		(node_map_test
		 (1, false,
		  ">"));

	ts << "actor_cycle" << result
		(node_map_test
		 (2, false,
		  ".>"
		  "<."));
	ts << "item_cycle" << result
		(node_map_test
		 (3, false,
		  ".><"
		  "..>"
		  "..."));
}

bool join_test() {
	std::vector<int> i(10);
	std::vector<int> o;

	for (int j = 0; j < 10; ++j) {
		i[j] = j;
	}

	join<int> j;
	pipeline p1 = input_vector(i) | j.sink();
	pipeline p2 = input_vector(i) | j.sink();
	pipeline p3 = j.source() | output_vector(o);

	p3.plot(log_info());
	p3();

	if (o.size() != 20) {
		log_error() << "Wrong output size " << o.size() << " expected 20" << std::endl;
		return false;
	}

	for (int i = 0; i < 20; ++i) {
		if (o[i] == i % 10) continue;
		log_error() << "Wrong output item got " << o[i] << " expected " << i%10 << std::endl;
		return false;
	}
	return true;
}

bool split_test() {
	std::vector<int> i(10);
	std::vector<int> o1, o2;

	for (int j = 0; j < 10; ++j) {
		i[j] = j;
	}

	split<int> j;
	pipeline p1 = input_vector(i) | j.sink();
	pipeline p2 = j.source() | output_vector(o1);
	pipeline p3 = j.source() | output_vector(o2);

	p3.plot(log_info());
	p3();

	if (o1.size() != 10 || o2.size() != 10) {
		log_error() << "Wrong output size " << o1.size() << " " << o2.size() << " expected 10" << std::endl;
		return false;
	}

	for (int i = 0; i < 10; ++i) {
		if (o1[i] == i % 10 && o2[i] == i % 10) continue;
		log_error() << "Wrong output item got " << o1[i] << " " << o2[i] << " expected " << i%10 << std::endl;
		return false;
	}
	return true;
}

bool copy_ctor_test() {
	std::vector<int> i(10);
	std::vector<int> j;
	pipeline p1 = input_vector(i) | output_vector(j);
	pipeline p2 = p1;
	p2();
	return true;
}

struct datastructuretest {
	size_t totalMemory;
	size_t minMem1;
	size_t maxMem1;
	size_t minMem2;
	size_t maxMem2;
	double frac1;
	double frac2;

	size_t assigned1;
	size_t assigned2;
};

template <typename dest_t>
class datastructuretest_1 : public node {
	dest_t dest;
	datastructuretest & settings;

public:
	datastructuretest_1(dest_t dest, datastructuretest & settings)
		: dest(std::move(dest))
		, settings(settings)
	{
		add_push_destination(dest);
	}

	void prepare() {
		register_datastructure_usage("datastructure1", settings.frac1);
		register_datastructure_usage("datastructure2", settings.frac2);
		if (settings.maxMem1 > 0)
			set_datastructure_memory_limits("datastructure1", settings.minMem1, settings.maxMem1);
		else
			set_datastructure_memory_limits("datastructure1", settings.minMem1);

		if (settings.maxMem2 > 0)
			set_datastructure_memory_limits("datastructure2", settings.minMem2, settings.maxMem2);
		else
			set_datastructure_memory_limits("datastructure2", settings.minMem2);
	}

	 virtual void go() {

	 }
};

class datastructuretest_2 : public node {
	datastructuretest & settings;

public:
	datastructuretest_2(datastructuretest & settings)
		: settings(settings)
	{
	}

	void prepare() {
		register_datastructure_usage("datastructure1", settings.frac1);
		register_datastructure_usage("datastructure2", settings.frac2);
	}

	virtual void propagate() {
		settings.assigned1 = get_datastructure_memory("datastructure1");
		settings.assigned2 = get_datastructure_memory("datastructure2");
	}
};

bool datastructure_test(datastructuretest settings) {
	if (settings.minMem1 + settings.minMem2 > settings.totalMemory) {
		throw tpie::exception("Memory requirements too high");
	}

	const memory_size_type NO_MEM = std::numeric_limits<memory_size_type>::max();
	settings.assigned1 = settings.assigned2 = NO_MEM;

	progress_indicator_null pi;

	pipeline p =
		make_pipe_begin<datastructuretest_1, datastructuretest &>(settings)
		| make_pipe_end<datastructuretest_2, datastructuretest &>(settings);
	p(0, pi, settings.totalMemory, TPIE_FSI);

	log_debug() << "totalMemory " << settings.totalMemory << '\n'
	            << "minMem1     " << settings.minMem1 << '\n'
	            << "maxMem1     " << settings.maxMem1 << '\n'
	            << "minMem2     " << settings.minMem2 << '\n'
	            << "maxMem2     " << settings.maxMem2 << '\n'
	            << "frac1       " << settings.frac1 << '\n'
	            << "frac2       " << settings.frac2 << '\n'
	            << "assigned1   " << settings.assigned1 << '\n'
	            << "assigned2   " << settings.assigned2 << std::endl;

	if (settings.assigned1 == NO_MEM || settings.assigned2 == NO_MEM) {
		log_error() << "No memory assigned" << std::endl;
		return false;
	}

	if (settings.assigned1 + settings.assigned2 > settings.totalMemory) {
		log_error() << "Too much memory assigned" << std::endl;
		return false;
	}

	if (settings.assigned1 < settings.minMem1 || settings.assigned2 < settings.minMem2) {
		log_error() << "Too little memory assigned" << std::endl;
		return false;
	}

	if ((settings.maxMem1 != 0 && settings.assigned1 > settings.maxMem1)
		|| (settings.maxMem2 != 0 && settings.assigned2 > settings.maxMem2)) {
		log_error() << "Too much memory assigned" << std::endl;
		return false;
	}

	const double EPS = 1e-9;
	const size_t min1 = settings.minMem1;
	const size_t max1 = (settings.maxMem1 == 0) ? settings.totalMemory : settings.maxMem1;
	const size_t min2 = settings.minMem2;
	const size_t max2 = (settings.maxMem2 == 0) ? settings.totalMemory : settings.maxMem2;
	const size_t m1 = settings.assigned1;
	const size_t m2 = settings.assigned2;
	const double f1 = settings.frac1;
	const double f2 = settings.frac2;
	if ((min1 < m1 && m1 < max1) && (min2 < m2 && m2 < max2)
		&& std::abs(m1 * f2 - m2 * f1) > EPS)
	{
		log_error() << "Fractions not honored" << std::endl;
		return false;
	}

	return true;
}

void datastructure_test_shorthand(teststream & ts, size_t totalMemory, size_t minMem1, size_t maxMem1, size_t minMem2, size_t maxMem2, double frac1, double frac2) {
	ts << "(" << totalMemory << ", " << minMem1 << ", " << maxMem1 << ", " << minMem2 << ", " << maxMem2 << ", " << frac1 << ", " << frac2 << ")";
	datastructuretest settings;
	settings.totalMemory = totalMemory;
	settings.minMem1 = minMem1;
	settings.maxMem1 = maxMem1;
	settings.minMem2 = minMem2;
	settings.maxMem2 = maxMem2;
	settings.frac1 = frac1;
	settings.frac2 = frac2;
	ts << result(datastructure_test(settings));
}

void datastructure_test_multi(teststream & ts) {
	//                        total   min1   max1   min2   max2  frac1  frac2
	datastructure_test_shorthand(ts,  2000,     0,     0,     0,     0,   1.0,   1.0);
	datastructure_test_shorthand(ts,  2000,   800,     0,   800,     0,   1.0,   1.0);
	datastructure_test_shorthand(ts,  4000,  1000,     0,  1000,     0,   0.0,   0.0);
	datastructure_test_shorthand(ts,  2000,     0,     0,     0,     0,   0.0,   1.0);
	datastructure_test_shorthand(ts,  2000,   500,     0,     0,     0,   0.0,   1.0);
	datastructure_test_shorthand(ts,  2000,   500,   700,     0,     0,   1.0,   1.0);
	datastructure_test_shorthand(ts,  2000,     0,   700,     0,   500,   1.0,   1.0);
	datastructure_test_shorthand(ts,  2000,     0,  2000,     0,  2000,   1.0,   1.0);
	datastructure_test_shorthand(ts,  2000,   200,  2000,     0,  2000,   1.0,   1.0);
}

template <typename dest_t>
class flush_priority_test_node_t : public node {
public:
	typedef int item_type;

	flush_priority_test_node_t(dest_t dest, size_t flushPriority, size_t & returnedValue)
		: dest(std::move(dest))
		, returnedValue(returnedValue)
	{
		add_push_destination(dest);
		set_flush_priority(flushPriority);
	}

	virtual void propagate() override {
		returnedValue = get_flush_priority();
	}

	virtual void go() override {
		for(int i = 0; i < 100; ++i)
			dest.push(i);
	}
private:
	dest_t dest;
	size_t & returnedValue;
};

typedef pipe_begin<factory<flush_priority_test_node_t, size_t, size_t &> > flush_priority_test_node;

bool set_flush_priority_test() {
	for(size_t i = 0; i < 100; ++i) {
		size_t returnedValue;
		pipeline p = flush_priority_test_node(i, returnedValue) | null_sink<size_t>();

		p();

		TEST_ENSURE_EQUALITY(i, returnedValue, "The flush priority was not set to the correct value");
	}

	return true;
}

template <typename dest_t>
class reference_incrementer_type : public node {
	dest_t dest;
	size_t & counter;
	size_t & incremented_to;
public:
	typedef int item_type;

	reference_incrementer_type(dest_t dest, size_t flush_priority, size_t & counter, size_t & incremented_to, std::string name)
	: dest(std::move(dest))
	, counter(counter)
	, incremented_to(incremented_to)
	{
		add_push_destination(dest);
		set_flush_priority(flush_priority);
		set_name(name);
	}

	void propagate() {
		incremented_to = ++counter;
	}

	void push(int item) {
		dest.push(item);
	}
};

typedef pipe_middle<factory<reference_incrementer_type, size_t, size_t &, size_t &, std::string> > reference_incrementer;

bool phase_priority_test() {
	size_t branchA;
	size_t branchB;
	size_t branchC;
	size_t counter = 0;
	std::vector<int> items;
	for(int i = 0; i < 100; ++i) items.push_back(i);

	pipeline p =
				input_vector(items)
				| fork(buffer() | reference_incrementer(1, counter, branchA, "Branch A(priority 1)") | null_sink<int>())
				| fork(buffer() |reference_incrementer(3, counter, branchC, "Branch C(priority 3)") | null_sink<int>())
				| buffer() | reference_incrementer(2, counter, branchB, "Branch B(priority 2)") | null_sink<size_t>();

	p.plot(log_debug());
	p();

	TEST_ENSURE_EQUALITY(1, branchA, "Branch A should be run first");
	TEST_ENSURE_EQUALITY(2, branchB, "Branch B should be run second");
	TEST_ENSURE_EQUALITY(3, branchC, "Branch C should be run last");
	return true;
}

template <typename dest_t>
class subpipe_tester_type: public node {
public:
	struct dest_pusher: public node {
		dest_pusher(dest_t & dest, int first): first(first), dest(dest) {}
		void push(int second) {
			dest.push(std::make_pair(first, second));
		}					  
		int first;
		dest_t & dest;
	};
		
	subpipeline<int> sp;
	int first;
	subpipe_tester_type(dest_t dest): dest(std::move(dest)) {
		set_memory_fraction(2);
	}

	void prepare() override {
		first = 1234;
	}
	
	void push(std::pair<int, int> i) {
		if (i.first != first) {
			if (first != 1234)
				sp.end();
			first = i.first;
			sp = sort() | pipe_end<termfactory<dest_pusher, dest_t &, int>>(dest, first);
			sp.begin(get_available_memory());
		}
		sp.push(i.second);
	}

	void end() override {
		if (first != 1234)
			sp.end();
	}
		
	dest_t dest;
};

typedef pipe_middle<factory<subpipe_tester_type> > subpipe_tester;

bool subpipeline_test() {
	constexpr int outer_size = 10;
	constexpr int inner_size = 3169; //Must be prime
	std::vector<std::pair<int, int> > items;
	for (int i=0; i < outer_size; ++i) {
		for (int j=0; j < inner_size; ++j)
			items.push_back(std::make_pair(i, (j*13) % inner_size));
	}

	std::vector<std::pair<int, int> > items2;
	
	pipeline p = input_vector(items) | subpipe_tester() | output_vector(items2);
	p();
	if (items2.size() != items.size()) return false;

	int cnt=0;
	for (int i=0; i < outer_size; ++i)
		for (int j=0; j < inner_size; ++j) 
			if (items2[cnt++] != std::make_pair(i, j)) return false;
		
	return true;
}

bool file_limit_sort_test() {
	int N = 1000000;
	int B = 10000;
	// Merge sort needs at least 3 open files for binary merge sort
	// + 2 open files to store stream_position objects for sorted runs.
	int F = 5;

	get_memory_manager().set_limit(5000000);

	set_block_size(B * sizeof(int));
	get_file_manager().set_limit(F);
	get_file_manager().set_enforcement(file_manager::ENFORCE_THROW);

	std::vector<int> items;
	for (int i = 0; i < N; i++) {
		items.push_back(N - i);
	}

	std::vector<int> items2;

	pipeline p = input_vector(items) | sort() | output_vector(items2);
	p();

	return true;
}

template<typename T>
virtual_chunk<int, int> passive_virtual_chunk() {
    T passive;
    return virtual_chunk<int, int>(fork(passive.input())
                                   | buffer()
                                   | merge(passive.output()));
}

template<typename T>
void passive_virtual_test(teststream & ts, const char * cname, const std::vector<int> & input, const std::vector<int> & expected_output) {
    ts << cname;

    auto vc = passive_virtual_chunk<T>();

    std::vector<int> output;
    pipeline p = virtual_chunk_begin<int>(input_vector(input))
                 | vc
                 | virtual_chunk_end<int>(output_vector<int>(output));
    p();

    // Output is interleaved with the input
    auto expected = expected_output;
    for (size_t i = 0; i < input.size(); i++) {
        expected.insert(expected.begin() + i * 2, input[i]);
    }

    ts << result(output == expected);
}

void passive_virtual_test_multi(teststream & ts) {
    std::vector<int> input    = {3, 4, 1, 2};
    std::vector<int> reversed = {2, 1, 4, 3};
    std::vector<int> sorted   = {1, 2, 3, 4};

#define TEST(T, expected) passive_virtual_test<T<int>>(ts, #T, input, expected)

    TEST(passive_sorter, sorted);
    TEST(passive_buffer, input);
    TEST(passive_reverser, reversed);
    TEST(serialization_passive_sorter, sorted);
    TEST(passive_serialization_buffer, input);
    TEST(passive_serialization_reverser, reversed);

#undef TEST
}

bool join_split_dealloc_test() {
    std::vector<int> v{1, 2, 3};
    pipeline p, p1, p2, p3;

    {
        join<int> j;
        split<int> s;

        p = input_vector(v) | s.sink();
        p1 = s.source() | j.sink();
        p2 = s.source() | j.sink();
        p3 = j.source() | null_sink<int>();
    }

    p();

    return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.setup(setup_test_vectors)
	.test(vector_multiply_test, "vector")
	.test(file_stream_test, "filestream", "n", static_cast<stream_size_type>(3))
	.test(file_stream_pull_test, "fspull")
		//.test(file_stream_alt_push_test, "fsaltpush")
	.test(merge_test, "merge")
	.test(reverse_test, "reverse")
	.test(internal_reverse_test, "internal_reverse")
	.test(passive_reverse_test, "passive_reverse", "n", static_cast<size_t>(50000))
	.test(internal_passive_reverse_test, "internal_passive_reverse", "n", static_cast<size_t>(50000))
	.test(sort_test_trivial, "sorttrivial")
	.test(sort_test_small, "sort")
	.test(sort_test_large, "sortbig")
	.test(operator_test, "operators")
	.test(uniq_test, "uniq")
	.multi_test(memory_test_multi, "memory")
	.test(fork_test, "fork")
	.test(merger_memory_test, "merger_memory", "n", static_cast<size_t>(10))
	.test(fetch_forward_test, "fetch_forward")
	.test(bound_fetch_forward_test, "bound_fetch_forward")
	.test(forward_unique_ptr_test, "forward_unique_ptr")
	.test(forward_multiple_pipelines_test, "forward_multiple_pipelines")
	.test(pipe_base_forward_test, "pipe_base_forward")
	.test(virtual_test, "virtual")
	.test(virtual_fork_test, "virtual_fork")
	.test(virtual_cref_item_type_test, "virtual_cref_item_type")
	.test(prepare_test, "prepare")
	.test(end_time::test, "end_time")
	.test(pull_iterator_test, "pull_iterator")
	.test(push_iterator_test, "push_iterator")
	.test(parallel_test, "parallel", "modulo", static_cast<size_t>(20011))
	.test(parallel_ordered_test, "parallel_ordered", "modulo", static_cast<size_t>(20011))
	.test(parallel_step_test, "parallel_step")
	.test(parallel_multiple_test, "parallel_multiple")
	.test(parallel_own_buffer_test, "parallel_own_buffer")
	.test(parallel_push_in_end_test, "parallel_push_in_end")
	.test(join_test, "join")
	.test(split_test, "split")
	.test(subpipeline_test, "subpipeline")
	.multi_test(node_map_multi_test, "node_map")
	.test(copy_ctor_test, "copy_ctor")
	.test(set_flush_priority_test, "set_flush_priority_test")
	.test(phase_priority_test, "phase_priority_test")
	.multi_test(datastructure_test_multi, "datastructures")
	.test(file_limit_sort_test, "file_limit_sort")
	.multi_test(passive_virtual_test_multi, "passive_virtual_management")
	.test(join_split_dealloc_test, "join_split_dealloc")
	;
}
