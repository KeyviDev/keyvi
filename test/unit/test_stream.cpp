// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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

// This program tests sequential reads and writes of 8 MB of 64-bit int items,
// sequential read and write of 8 MB of 64-bit int arrays,
// random seeking in 8 MB followed by either a read or a write.

#include "common.h"

#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <tpie/tpie_log.h>
#include <tpie/progress_indicator_arrow.h>

#include <tpie/tpie.h>

#include <tpie/array.h>
#include <tpie/file_stream.h>
#include <tpie/compressed/stream.h>
#include <tpie/util.h>

using tpie::uint64_t;

inline uint64_t ITEM(size_t i) {return i*98927 % 104639;}
static const size_t TESTSIZE = 8*1024*1024;
static const size_t ITEMS = TESTSIZE/sizeof(uint64_t);
static const size_t ARRAYSIZE = 512;
static const size_t ARRAYS = TESTSIZE/(ARRAYSIZE*sizeof(uint64_t));

struct movable_file_stream {
	tpie::unique_ptr<tpie::uncompressed_stream<uint64_t> > fs;
	movable_file_stream() {fs.reset(tpie::tpie_new<tpie::uncompressed_stream<uint64_t> >());}
	movable_file_stream(tpie::uncompressed_stream<uint64_t> & with) {
		fs.reset(tpie::tpie_new<tpie::uncompressed_stream<uint64_t> >());
		fs->swap(with);
	}
	movable_file_stream(const movable_file_stream & other) {
		fs.reset(tpie::tpie_new<tpie::uncompressed_stream<uint64_t> >());
		fs->swap(*other.fs);
	}
	movable_file_stream & operator=(const movable_file_stream & other) {
		fs->swap(*other.fs);
		return *this;
	}
};

bool swap_test();

template <typename T>
struct file_colon_colon_stream {
	tpie::file<T> m_file;
	tpie::unique_ptr<typename tpie::file<T>::stream> m_stream;
	typedef typename tpie::file<T>::stream stream_type;

	inline ~file_colon_colon_stream() {
		m_stream.reset();
	}

	tpie::file<T> & file() {
		return m_file;
	}

	typename tpie::file<T>::stream & stream() {
		if (m_stream.get() == 0) m_stream.reset(tpie::tpie_new<typename tpie::file<T>::stream>(m_file));
		return *m_stream;
	}

	inline void close_stream() {
		m_stream.reset();
	}

	void open(std::string fileName) { file().open(fileName); }
	void open(tpie::temp_file & tf) { file().open(tf); }
	void open(tpie::temp_file & tf, tpie::access_type a) { file().open(tf, a); }
	void open(tpie::temp_file & tf, tpie::access_type a, tpie::memory_size_type uds) { file().open(tf, a, uds); }
};

template <typename T>
struct file_stream {
	tpie::uncompressed_stream<T> m_fs;
	typedef tpie::uncompressed_stream<T> stream_type;

	tpie::uncompressed_stream<T> & file() {
		return m_fs;
	}

	tpie::uncompressed_stream<T> & stream() {
		return m_fs;
	}

	inline void close_stream() {
		m_fs.seek(0);
	}

	void open(std::string fileName) { file().open(fileName); }
	void open(tpie::temp_file & tf) { file().open(tf); }
	void open(tpie::temp_file & tf, tpie::access_type a) { file().open(tf, a); }
	void open(tpie::temp_file & tf, tpie::access_type a, tpie::memory_size_type uds) { file().open(tf, a, uds); }
};

template <typename T>
struct compressed_stream {
	tpie::file_stream<T> m_fs;
	typedef tpie::file_stream<T> stream_type;

	tpie::file_stream<T> & file() {
		return m_fs;
	}

	tpie::file_stream<T> & stream() {
		return m_fs;
	}

	inline void close_stream() {
		m_fs.seek(0);
	}

	void open(std::string fileName) {
		file().open(fileName, tpie::access_read_write, 0, tpie::access_sequential, tpie::compression_none);
	}
	void open(tpie::temp_file & tf) {
		file().open(tf, tpie::access_read_write, 0, tpie::access_sequential, tpie::compression_none);
	}
	void open(tpie::temp_file & tf, tpie::access_type a) {
		file().open(tf, a, 0, tpie::access_sequential, tpie::compression_none);
	}
	void open(tpie::temp_file & tf, tpie::access_type a, tpie::memory_size_type uds) {
		file().open(tf, a, uds, tpie::access_sequential, tpie::compression_none);
	}
};

template <template <typename U> class Stream>
struct stream_tester {

static bool array_test() {
	try {
		tpie::temp_file tmp;
		Stream<uint64_t> fs;
		fs.open(tmp.path());
		tpie::memory_size_type items = tpie::file<uint64_t>::block_size(1.0)/sizeof(uint64_t) + 10;
		std::vector<uint64_t> data(items, 1);
		fs.stream().write(data.begin(), data.end());
		fs.stream().seek(0);
		fs.stream().read(data.begin(), data.end());
	} catch (std::exception & e) {
		tpie::log_error() << "Caught exception " << typeid(e).name() << "\ne.what(): " << e.what() << std::endl;
		return false;
	} catch (...) {
		tpie::log_error() << "Caught something other than an exception" << std::endl;
		return false;
	}
	return true;
}

static bool truncate_test() {
	tpie::stream_size_type initialTempFileUsage = tpie::get_temp_file_usage();
	typedef int test_t;
	tpie::temp_file tempFile;
	Stream<test_t> fs;
	fs.open(tempFile);
	for (size_t i = 0; i < 10000000; ++i)
		fs.stream().write(42);
	bool res = true;
	try {
		fs.stream().seek(0);
	} catch (tpie::io_exception) {
		tpie::log_error() << "We should be able to seek!" << std::endl;
		res = false;
	}
	fs.close_stream();
	tpie::stream_size_type tempFileUsage = tpie::get_temp_file_usage();
	if (tempFileUsage == initialTempFileUsage) {
		tpie::log_error() << "Temp file usage did not increase" << std::endl;
		res = false;
	}
	fs.file().truncate(42);
	if (tempFileUsage != initialTempFileUsage && tempFileUsage <= tpie::get_temp_file_usage()) {
		tpie::log_error() << "Temp file usage did not decrease from " << tempFileUsage << std::endl;
		res = false;
	}
	for (size_t i = 0; i < 42; ++i) {
		if (!fs.stream().can_read()) {
			tpie::log_error() << "Cannot read item " << i << std::endl;
			return false;
		}
		if (42 != fs.stream().read()) {
			tpie::log_error() << "Item " << i << " is wrong" << std::endl;
			return false;
		}
	}
	if (fs.stream().can_read()) {
		tpie::log_error() << "We should not be able to read after truncate!" << std::endl;
		res = false;
	}
	try {
		fs.stream().read();
		tpie::log_error() << "We should not be able to read after truncate!" << std::endl;
		return false;
	} catch (tpie::stream_exception) {
	}
	return res;
}

// test using truncate to extend the stream
static bool extend_test() {
	typedef int test_t;
	tpie::temp_file tmp;
	Stream<test_t> fs;
	fs.open(tmp.path());
	tpie::stream_size_type ante = 0;
	tpie::stream_size_type pred = 1;
	tpie::stream_size_type n = 1;
	while (n < 1000000) {
		fs.close_stream();
		fs.file().truncate(n);
		if (fs.file().size() != n) {
			tpie::log_error() << "Wrong stream length" << std::endl;
			return false;
		}
		tpie::stream_size_type readItems = 0;
		while (fs.stream().can_read()) {
			fs.stream().read();
			++readItems;
		}
		if (readItems != n) {
			tpie::log_error() << "Read wrong no. of items" << std::endl;
			return false;
		}
		ante = pred;
		pred = n;
		n = ante+pred;
	}
	return true;
}

static bool odd_block_test() {
	typedef std::array<char, 17> test_t;
	const size_t items = 500000;
	test_t initial_item;
	for (size_t i = 0; i < initial_item.size(); ++i) initial_item[i] = static_cast<char>(i+42);

	tpie::temp_file tmp;
	{
		Stream<test_t> fs;
		fs.open(tmp.path());
		test_t item = initial_item;
		for (size_t i = 0; i < items; ++i) {
			fs.stream().write(item);
			item[0]++;
		}
	}

	{
		Stream<test_t> fs;
		fs.open(tmp.path());

		test_t item = initial_item;
		for (size_t i = 0; i < items; ++i) {
			test_t got = fs.stream().read();
			if (got != item) {
				tpie::log_error() << "Item " << i << " is wrong" << std::endl;
				return false;
			}
			item[0]++;
		}
	}

	return true;
}

static bool backwards_test() {
	tpie::temp_file tmp;
	Stream<int> fs;

	fs.open(tmp);
	TEST_ENSURE(!fs.stream().can_read_back(), "can_read_back() after open()")

	fs.stream().write(1);
	TEST_ENSURE(fs.stream().can_read_back(), "can_read_back() after write()")

	fs.stream().seek(0);
	TEST_ENSURE(!fs.stream().can_read_back(), "can_read_back() after seek()")

	for (int i = 0; i < (1 << 25); ++i) fs.stream().write(i);
	TEST_ENSURE(fs.stream().can_read_back(), "can_read_back() after writing a block aligned chunk")

	fs.stream().read_back();
	TEST_ENSURE(fs.stream().can_read_back(), "can_read_back() after read_back()")

	fs.stream().seek(5);
	TEST_ENSURE(fs.stream().can_read_back(), "can_read_back() after seek(5)")

	return true;
}

struct stress_tester {
	const size_t chunkSize;
	tpie::stream_size_type actions;
	size_t maxSize;
	std::vector<int> elements;
	std::vector<bool> defined;
	std::vector<int> itemBuffer;
	Stream<int> fs;
	typename Stream<int>::stream_type & stream;
	std::mt19937 rng;
	std::uniform_int_distribution<> ddist;
	bool result;
	size_t size;
	size_t location;

	inline stress_tester(tpie::stream_size_type actions, size_t maxSize)
		: chunkSize(128*1024)
		, actions(actions)
		, maxSize(maxSize)
		, elements(maxSize, 0)
		, defined(maxSize, true)
		, itemBuffer(chunkSize)
		, stream(fs.stream())
		, ddist(0, 123456789)
		, result(true)
		, size(0)
		, location(0)
	{
	}

	enum actions {
		Read, Write,
		SeekEnd, Seek,
		ReadArray, WriteArray,
		Truncate,
		actionCount
	};

	template <typename T>
	bool stress_assert_eq(T got, T value, const char * exp) {
		if (value != got) {
			tpie::log_error() << "Expected " << exp << " == " << value << ", got " << got << '\n';
			return false;
		}
		return true;
	}

	bool go() {
		tpie::temp_file tmp;
		fs.open(tmp.path());
		std::uniform_int_distribution<> todo(0, actionCount - 1);
		const size_t stepEvery = 256;
		tpie::progress_indicator_arrow pi("Test", actions/stepEvery);
		pi.init(actions/stepEvery);
		for (tpie::stream_size_type action = 0; action < actions; ++action) {
			try {
				switch (todo(rng)) {
					case Read: read(); break;
					case Write: write(); break;
					case SeekEnd: seek_end(); break;
					case Seek: seek(); break;
					case ReadArray: read_array(); break;
					case WriteArray: write_array(); break;
					case Truncate: truncate(); break;
				}
			} catch (std::exception & e) {
				tpie::log_debug() << std::flush;
				tpie::log_error() << "Caught unexpected exception " << typeid(e).name() << " (" << e.what() << ')' << std::endl;
				result = false;
				break;
			} catch (...) {
				tpie::log_debug() << std::flush;
				tpie::log_error() << "Caught unexpected non-exception" << std::endl;
				result = false;
				break;
			}
			if (!stress_assert_eq(static_cast<size_t>(stream.size()), size, "stream.size()")) {
				result = false;
			}
			if (!stress_assert_eq(static_cast<size_t>(stream.offset()), location, "stream.offset()")) {
				result = false;
			}
			if (0 == action % stepEvery) pi.step();
		}
		pi.done();
		return result;
	}

	void read() {
		if (size == location) {
			if (stream.can_read()) {
				tpie::log_error() << "Assertion !stream.can_read() failed" << std::endl;
				result = false;
			}
			return;
		}
		size_t maxItems = size - location;
		std::uniform_int_distribution<> d(1,std::min<size_t>(maxItems, chunkSize));
		size_t items = d(rng);
		tpie::log_debug() << "stream.read() * " << items << '\n';
		for (size_t i = 0; i < items; ++i) {
			if (defined[location]) {
				if (!stress_assert_eq(stream.read(), elements[location], "stream.read()")) {
					result = false;
				}
			} else {
				int readItem = stream.read();
				elements[location] = readItem;
				defined[location] = true;
				//tpie::log_debug() << "elements[" << location << "] = " << readItem <<
				//	"; defined[" << location << "] = true;\n";
			}
			++location;
		}
	}

	void read_array() {
		if (size == location) {
			if (stream.can_read()) {
				tpie::log_error() << "Assertion !stream.can_read() failed" << std::endl;
				result = false;
			}
			return;
		}
		size_t maxItems = size - location;
		std::uniform_int_distribution<> d(1,std::min<size_t>(maxItems, chunkSize));
		size_t items = d(rng);
		stream.read(itemBuffer.begin(), itemBuffer.begin() + items);
		tpie::log_debug() << "stream.read(itemBuffer.begin(), itemBuffer.begin() + " << items << ");\n";
		for (size_t i = 0; i < items; ++i) {
			if (defined[location]) {
				if (!stress_assert_eq(itemBuffer[i], elements[location], "itemBuffer[i]")) {
					result = false;
				}
			} else {
				int readItem = itemBuffer[i];
				elements[location] = readItem;
				defined[location] = true;
				//tpie::log_debug() << "elements[" << location << "] = " << readItem <<
				//	"; defined[" << location << "] = true;\n";
			}
			++location;
		}
	}

	void write() {
		if (location == maxSize) return;
		size_t maxItems = maxSize - location;
		std::uniform_int_distribution<> d(1,std::min<size_t>(maxItems, chunkSize));
		size_t items = d(rng);
		tpie::log_debug() << "stream.write() * " << items << '\n';
		for (size_t i = 0; i < items; ++i) {
			int writeItem = ddist(rng);
			elements[location] = writeItem;
			defined[location] = true;
			stream.write(writeItem);
			//tpie::log_debug() << "elements[" << location << "] = " << writeItem <<
			//	"; defined[" << location << "] = true;\n";
			++location;
		}
		size = std::max(size, location);
	}

	void write_array() {
		if (location == maxSize) return;
		size_t maxItems = maxSize - location;
		std::uniform_int_distribution<> d(1,std::min<size_t>(maxItems, chunkSize));
		size_t items = d(rng);
		tpie::log_debug() << "stream.write(itemBuffer.begin(), itemBuffer.begin() + " << items << ");\n";
		for (size_t i = 0; i < items; ++i) {
			int writeItem = ddist(rng);
			itemBuffer[i] = writeItem;
			elements[location] = writeItem;
			defined[location] = true;
			//tpie::log_debug() << "elements[" << location << "] = " <<
			//	"itemBuffer[" << i << "] = " << writeItem <<
			//	"; defined[" << location << "] = true;\n";
			++location;
		}
		stream.write(itemBuffer.begin(), itemBuffer.begin() + items);
		size = std::max(size, location);
	}

	void seek_end() {
		tpie::log_debug() << "stream.seek(" << size << "); // end\n";
		stream.seek(size);
		location = size;
	}

	void seek() {
		std::uniform_int_distribution<> d(0, size);
		size_t loc = d(rng);
		tpie::log_debug() << "stream.seek(" << loc << "); // end\n";
		stream.seek(loc);
		location = loc;
	}

	void truncate() {
		std::uniform_int_distribution<> d(std::max(0, (int)size-(int)chunkSize), std::min(size+chunkSize, maxSize));
		size_t ns=d(rng);
		tpie::log_debug() << "fs.file().truncate(" << ns << "); // was " << size << '\n';
		stream.seek(0);
		location = 0;
		fs.close_stream();
		fs.file().truncate(ns);
		for (size_t i = size; i < ns; ++i) {
			defined[i] = false;
		}
		location = 0;
		size = ns;
	}
};

static bool stress_test(tpie::stream_size_type actions, size_t maxSize) {
	return stress_tester(actions, maxSize).go();
}

static bool user_data_test() {
	tpie::temp_file tmp;
	typedef Stream<int> Fs;
	const int init[] = {2,4,6};
	{
		Fs fs;
		fs.open(tmp, tpie::access_write, 8*sizeof(int));
		if (fs.file().user_data_size() != 0) {
			tpie::log_error() << "Wrong user data size after opening for creation" << std::endl;
			return false;
		}
		if (fs.file().max_user_data_size() != 8*sizeof(int)) {
			tpie::log_error() << "Wrong max user data size after opening for creation" << std::endl;
			return false;
		}
		bool except = false;
		try {
			const int data[] = {2,4,6,8,10,12,14,16,18,20};
			fs.file().write_user_data(data, 10*sizeof(int));
		} catch (tpie::stream_exception &) {
			except = true;
		}
		if (!except) {
			tpie::log_error() << "Expected stream_exception" << std::endl;
			return false;
		}
	}
	{
		Fs fs;
		fs.open(tmp, tpie::access_write, 8*sizeof(int));
		if (fs.file().user_data_size() != 0) {
			tpie::log_error() << "Wrong user data size after opening for writing" << std::endl;
			return false;
		}
		if (fs.file().max_user_data_size() != 8*sizeof(int)) {
			tpie::log_error() << "Wrong max user data size after opening for writing" << std::endl;
			return false;
		}
		fs.file().write_user_data(init, 3*sizeof(int));
		if (fs.file().user_data_size() != 3*sizeof(int)) {
			tpie::log_error() << "Wrong user data size after write" << std::endl;
			return false;
		}
	}
	{
		Fs fs;
		fs.open(tmp, tpie::access_read);
		if (fs.file().user_data_size() != 3*sizeof(int)) {
			tpie::log_error() << "Wrong user data size after opening for reading" << std::endl;
			return false;
		}
		if (fs.file().max_user_data_size() != 8*sizeof(int)) {
			tpie::log_error() << "Wrong max user data size after opening for reading" << std::endl;
			return false;
		}
		int data[9];
		data[0] = data[1] = data[2] = 42; // marker
		tpie::memory_size_type cnt = fs.file().read_user_data(data, 2*sizeof(int));
		if (cnt != 2*sizeof(int)) {
			tpie::log_error() << "read_user_data read wrong amount" << std::endl;
			return false;
		}
		if (data[0] != init[0] || data[1] != init[1] || data[2] != 42) {
			tpie::log_error() << "read_user_data read something wrong" << std::endl;
			return false;
		}
		cnt = fs.file().read_user_data(data, 9*sizeof(int));
		if (cnt != 3*sizeof(int)) {
			tpie::log_error() << "read_user_data read wrong amount" << std::endl;
			return false;
		}
	}
	{
		Fs fs;
		bool except = false;
		try {
			fs.open(tmp, tpie::access_read_write, 10*sizeof(int));
		} catch (tpie::invalid_file_exception &) {
			except = true;
		}
		if (!except) {
			tpie::log_error() << "Expected invalid_file_exception, got nothing" << std::endl;
			return false;
		}
	}
	return true;
}

}; // template stream_tester

movable_file_stream openstream(tpie::temp_file & file) {
	tpie::uncompressed_stream<uint64_t> fs;
	fs.open(file.path());
	return fs;
}

bool swap_test() {
	// Write ITEMS items sequentially to the temporary file
	tpie::temp_file tmp;
	{
		movable_file_stream fs = openstream(tmp);

		tpie::uncompressed_stream<uint64_t> s;
		s.swap(*fs.fs);
		for(size_t i=0; i < ITEMS; ++i) s.write(ITEM(i));
	}

	// Sequential verify
	{
		movable_file_stream fs = openstream(tmp);
		tpie::uncompressed_stream<uint64_t> s;
		s.swap(*fs.fs);
		tpie::uncompressed_stream<uint64_t> t;
		for(size_t i=0; i < ITEMS; ++i) {
			uint64_t x = (i % 2) ? t.read() : s.read();
			if (x != ITEM(i)) {
				tpie::log_error() << "Expected element " << i << " = " << ITEM(i) << ", got " << x << std::endl;
				return false;
			}
			if (i % 3) s.swap(t);
			else t.swap(s);
		}
		s.swap(t);
	}

	// Write an ARRAYSIZE array ARRAYS times sequentially to the temporary file
	{
		tpie::uncompressed_stream<uint64_t> s;
		s.open(tmp.path());
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYSIZE; ++i) {
			x[i] = ITEM(i);
		}
		for(size_t i=0; i < ARRAYS; ++i) s.write(x + 0, x + ARRAYSIZE);
	}

	// Sequentially verify the arrays
	{
		tpie::uncompressed_stream<uint64_t> s;
		s.open(tmp.path());
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYS; ++i) {
			TPIE_OS_SIZE_T len = ARRAYSIZE;
			try {
				s.read(x + 0, x + len);
			} catch (tpie::end_of_stream_exception &) {
				tpie::log_error() << "read array threw unexpected end_of_stream_exception" << std::endl;
				return false;
			}
			for (size_t i=0; i < ARRAYSIZE; ++i) {
				if (x[i] != ITEM(i)) {
					tpie::log_error() << "Expected element " << i << " = " << ITEM(i) << ", got " << x[i] << std::endl;
					return false;
				}
			}
		}
	}

	// Random read/write of items
	{
		tpie::uncompressed_stream<uint64_t> s;
		s.open(tmp.path());
		tpie::array<uint64_t> data(ITEMS);
		for (size_t i=0; i < ITEMS; ++i) {
			data[i] = ITEM(i);
			s.write(data[i]);
		}
		for (size_t i=0; i < 10; ++i) {
			// Seek to random index
			size_t idx = ITEM(i) % ITEMS;
			s.seek(idx);

			if (i%2 == 0) {
				uint64_t read = s.read();
				if (read != data[idx]) {
					tpie::log_error() << "Expected element " << idx << " to be " << data[idx] << ", got " << read << std::endl;
					return false;
				}
			} else {
				uint64_t write = ITEM(ITEMS+i);
				data[idx] = write;
				s.write(write);
			}

			tpie::stream_offset_type newoff = s.offset();
			if (static_cast<size_t>(newoff) != idx+1) {
				tpie::log_error() << "Offset advanced to " << newoff << ", expected " << (idx+1) << std::endl;
				return false;
			}
		}
	}

	return true;
}

bool peek_skip_test_1() {
	tpie::file_stream<size_t> s;
	s.open();
	for (size_t i = 0; i < 1000000; ++i) s.write(i);
	s.seek(100);
	TEST_ENSURE(s.peek() == 100, "peek() wrong");
	s.seek(10000);
	TEST_ENSURE(s.peek() == 10000, "peek() wrong");
	s.skip();
	TEST_ENSURE(s.peek() == 10001, "peek() wrong");
	s.skip_back();
	TEST_ENSURE(s.peek() == 10000, "peek() wrong");
	s.seek(1000000);
	s.skip_back();
	TEST_ENSURE(s.peek() == 999999, "peek() wrong");

	bool threw = false;
	s.skip();
	try {
		s.peek();
	} catch (tpie::end_of_stream_exception &) {
		threw = true;
	}
	TEST_ENSURE(threw, "peek() did not throw");

	return true;
}

bool peek_skip_test_2() {
	tpie::file_stream<size_t> s;
	s.open();
	for (size_t i = 0; i < 1000000; ++i) s.write(i);
	s.seek(10000);
	TEST_ENSURE(s.peek() == 10000, "peek() wrong");
	s.truncate(10000);
	bool threw = false;
	try {
		s.peek();
	} catch (tpie::end_of_stream_exception &) {
		threw = true;
	}
	TEST_ENSURE(threw, "peek() did not throw");

	return true;
}

bool reopen() {
	tpie::temp_file tf;

	tpie::file_stream<int> s;
	s.open(tf);
	for (size_t i = 0; i < 1000000; ++i) s.write(i);
	TEST_ENSURE(s.is_open(), "is_open wrong");
	s.close();

	TEST_ENSURE(!s.is_open(), "is_open wrong");

	s.open(tf);
	TEST_ENSURE(s.is_open(), "is_open wrong");
	TEST_ENSURE(s.peek() == 0, "peek() wrong");
	TEST_ENSURE(s.read() == 0, "read() wrong");
	s.seek(10000);
	TEST_ENSURE(s.peek() == 10000, "peek() wrong");
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(stream_tester<file_stream>::array_test, "array")
		.test(stream_tester<file_colon_colon_stream>::array_test, "array_file")
		.test(stream_tester<compressed_stream>::array_test, "array_compressed")
		.test(swap_test, "basic")
		.test(stream_tester<file_stream>::odd_block_test, "odd")
		.test(stream_tester<file_colon_colon_stream>::odd_block_test, "odd_file")
		.test(stream_tester<compressed_stream>::odd_block_test, "odd_compressed")
		.test(stream_tester<file_stream>::truncate_test, "truncate")
		.test(stream_tester<file_colon_colon_stream>::truncate_test, "truncate_file")
		.test(stream_tester<compressed_stream>::truncate_test, "truncate_compressed")
		.test(reopen, "reopen")
		.test(stream_tester<file_stream>::extend_test, "extend")
		.test(stream_tester<file_colon_colon_stream>::extend_test, "extend_file")
		.test(stream_tester<compressed_stream>::extend_test, "extend_compressed")
		.test(stream_tester<file_stream>::backwards_test, "backwards")
		.test(stream_tester<file_colon_colon_stream>::backwards_test, "backwards_file")
		.test(stream_tester<compressed_stream>::backwards_test, "backwards_compressed")
		.test(stream_tester<file_stream>::user_data_test, "user_data")
		.test(stream_tester<file_colon_colon_stream>::user_data_test, "user_data_file")
		.test(stream_tester<compressed_stream>::user_data_test, "user_data_compressed")
		.test(stream_tester<file_stream>::stress_test, "stress", "actions", static_cast<tpie::stream_size_type>(1024*1024*10), "maxsize", static_cast<size_t>(1024*1024*128))
		.test(stream_tester<file_colon_colon_stream>::stress_test, "stress_file", "actions", static_cast<tpie::stream_size_type>(1024*1024*10), "maxsize", static_cast<size_t>(1024*1024*128))
		.test(stream_tester<compressed_stream>::stress_test, "stress_compressed", "actions", static_cast<tpie::stream_size_type>(1024*1024*10), "maxsize", static_cast<size_t>(1024*1024*128))
		.test(stream_tester<file_stream>::user_data_test, "user_data")
		.test(stream_tester<file_colon_colon_stream>::user_data_test, "user_data_file")
		.test(peek_skip_test_1, "peek_skip_1")
		.test(peek_skip_test_2, "peek_skip_2")
		;
}
