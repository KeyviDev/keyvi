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
#include <tpie/compressed/stream.h>
#include <tpie/file_stream.h>

template <tpie::compression_flags flags>
class tests {
public:

static bool basic_test(size_t n) {
	tpie::file_stream<size_t> s;
	s.open(0, tpie::access_sequential, flags);
	for (size_t i = 0; i < n; ++i) {
		if (s.size() != i) {
			tpie::log_error() << "size() == " << s.size()
				<< ", expected " << i << std::endl;
			return false;
		}
		s.write(i);
	}
	s.seek(0);
	for (size_t i = 0; i < n; ++i) {
		if (!s.can_read()) {
			tpie::log_error() << "!can_read @ " << i << " out of " << n << std::endl;
			return false;
		}
		if (s.size() != n) {
			tpie::log_error() << "size() == " << s.size()
				<< " at position " << i << ", expected " << n << std::endl;
			return false;
		}
		size_t r = s.read();
		if (r != i) {
			tpie::log_error() << "Read " << r << " at " << i << std::endl;
			return false;
		}
	}
	if (s.can_read()) {
		tpie::log_error() << "can_read @ end of stream" << std::endl;
		return false;
	}
	return true;
}

static bool read_seek_test(size_t seekPosition, size_t items) {
	if (seekPosition > items) {
		tpie::log_error() << "Invalid test parameters: " << seekPosition << " > " << items << std::endl;
		return false;
	}
	tpie::temp_file tf;

	tpie::file_stream<size_t> s;
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Open file" << std::endl;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Write some items" << std::endl;
	for (size_t i = 0; i < seekPosition; ++i) s.write(i);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Get position" << std::endl;
	tpie::stream_position pos1 = s.get_position();
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Write more items" << std::endl;
	for (size_t i = seekPosition; i < items; ++i) s.write(i);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Seek to 0" << std::endl;
	s.seek(0);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Read some items" << std::endl;
	for (size_t i = 0; i < seekPosition; ++i) s.read();
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Get position" << std::endl;
	tpie::stream_position pos2 = s.get_position();
	tpie::log_debug() << s.describe() << std::endl;

	if (pos1 != pos2) {
		tpie::log_error() << "Positions differ" << std::endl;
		return false;
	}

	tpie::log_debug() << "Read more items" << std::endl;
	for (size_t i = seekPosition; i < items; ++i) s.read();
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Set position" << std::endl;
	s.set_position(pos1);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Read single item" << std::endl;
	size_t d = s.read();
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Got " << d << ", expected "
		<< seekPosition << std::endl;
	s.seek(0);
	tpie::log_debug() << s.describe() << std::endl;
	if (d != seekPosition) return false;
	return true;
}

static bool read_back_seek_test() {
	tpie::temp_file tf;
	const size_t blockSize = 2*1024*1024 / sizeof(size_t);
	const size_t BLOCKS = 5;
	tpie::array<tpie::stream_position> positions(BLOCKS);
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		for (size_t block = 0; block < BLOCKS; ++block) {
			positions[block] = s.get_position();
			for (size_t i = 0; i < blockSize; ++i) s.write(i);
		}
	}
	tpie::stream_size_type r1 = tpie::get_bytes_read();
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		for (size_t block = 0; block < BLOCKS; ++block) {
			s.set_position(positions[block]);
			for (size_t i = 0; i < blockSize; ++i) s.read();
		}
	}
	tpie::stream_size_type r2 = tpie::get_bytes_read();
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		for (size_t block = 0; block < BLOCKS; ++block) {
			if (block+1 == BLOCKS)
				s.seek(0, tpie::file_stream_base::end);
			else
				s.set_position(positions[block+1]);
			for (size_t i = 0; i < blockSize; ++i) s.read_back();
		}
	}
	tpie::stream_size_type r3 = tpie::get_bytes_read();

	tpie::stream_size_type d1 = r2-r1;
	tpie::stream_size_type d2 = r3-r2;
	tpie::log_debug() << d1 << ' ' << d2 << std::endl;
	return d1 == d2;
}

static bool read_back_seek_test_2() {
	tpie::temp_file tf;
	const size_t blockSize = 2*1024*1024 / sizeof(size_t);
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		for (size_t i = 0; i < blockSize; ++i) s.write(i);
	}
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		s.seek(0, tpie::file_stream_base::end);
		s.write(1);
		s.read_back();
		s.read_back();
	}
	return true;
}

static bool read_back_throw_test() {
	for (size_t x = 0; x < 3; ++x) {
		size_t items = x * 1000000;
		tpie::log_debug() << "Writing " << items << " items and reading them back" << std::endl;
		tpie::file_stream<size_t> s;
		s.open();
		for (size_t i = 0; i < items; ++i) s.write(i);
		for (size_t i = 0; i < items; ++i) s.read_back();
		bool threw = false;
		try {
			s.read_back();
		} catch (const tpie::end_of_stream_exception &) {
			threw = true;
		} catch (...) {
			tpie::log_error() << "Threw something that is not an end_of_stream_exception." << std::endl;
			return false;
		}
		if (!threw) {
			tpie::log_error() << "Did not throw an end_of_stream_exception." << std::endl;
			return false;
		}
	}
	return true;
}

static bool position_test_0(size_t n) {
	tpie::temp_file tf;

	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	tpie::log_debug() << "Recording array of positions" << std::endl;
	tpie::array<tpie::stream_position> positions(n);
	for (size_t i = 0; i < n; ++i) {
		positions[i] = s.get_position();
		s.write(i);
	}

	tpie::log_debug() << "Verifying array of positions" << std::endl;
	s.seek(0);
	for (size_t i = 0; i < n; ++i) {
		tpie::stream_position p = s.get_position();
		if (positions[i] != p) {
			tpie::log_error() << "Disagreement in position " << i << std::endl;
			return false;
		}
		s.read();
	}

	tpie::log_debug() << "Verifying items at fib(n) positions" << std::endl;
	{
		size_t i = 0, j = 1;
		while (i + j < n) {
			size_t k = i + j;
			s.set_position(positions[k]);
			if (s.read() != k) {
				tpie::log_error() << "Bad read in position " << k << std::endl;
				return false;
			}
			i = j;
			j = k;
		}
	}

	tpie::log_debug() << "Verifying items at 2^n and 2^n-1 positions" << std::endl;
	for (size_t i = 1; i < n; i = i+i) {
		s.set_position(positions[i]);
		if (s.read() != i) {
			tpie::log_error() << "Bad read in position " << i << std::endl;
			return false;
		}
		s.set_position(positions[i-1]);
		if (s.read() != i-1) {
			tpie::log_error() << "Bad read in position " << i-1 << std::endl;
			return false;
		}
	}

	return true;
}

static bool position_seek_test() {
	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	for (size_t i = 0; i < 5; ++i) s.write(i);
	s.seek(0, tpie::file_stream_base::end);
	tpie::stream_position p = s.get_position();
	for (size_t i = 5; i < 10; ++i) s.write(i);
	s.seek(0);
	for (size_t i = 0; i < 10; ++i) {
		if (s.read() != i) {
			tpie::log_error() << "Bad read in position " << i << std::endl;
			return false;
		}
	}
	s.set_position(p);
	size_t x = s.read();
	if (x != 5) {
		tpie::log_error() << "Bad read after set_position; got " << x << ", expected 5" << std::endl;
		return false;
	}
	return true;
}

#define TEST_ASSERT(cond) \
	do { \
		if (!(cond)) { \
			tpie::log_error() << "Test failed on line " << __LINE__ << ": " #cond << std::endl; \
			return false; \
		} \
	} while (0)

static bool position_test_1() {
	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);

	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	for (size_t i = 0; i < 2*blockSize; ++i) s.write(i);
	TEST_ASSERT(!s.can_read());
	TEST_ASSERT(s.size() == 2*blockSize);
	TEST_ASSERT(s.offset() == 2*blockSize);
	tpie::log_debug() << s.describe() << std::endl;
	s.set_position(s.get_position());
	tpie::log_debug() << s.describe() << std::endl;
	TEST_ASSERT(!s.can_read());
	TEST_ASSERT(s.size() == 2*blockSize);
	TEST_ASSERT(s.offset() == 2*blockSize);
	s.write(2*blockSize);
	TEST_ASSERT(!s.can_read());
	TEST_ASSERT(s.size() == 2*blockSize+1);
	TEST_ASSERT(s.offset() == 2*blockSize+1);
	return true;
}

static bool position_test_2() {
	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	size_t blockSize = 2*1024*1024 / sizeof(size_t);

	tpie::log_debug() << "Write blockSize + 5 items" << std::endl;
	for (size_t i = 0; i < blockSize + 5; ++i) s.write(i);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Get position and write 2 items" << std::endl;
	tpie::stream_position pos1 = s.get_position();
	TEST_ASSERT(!s.can_read());
	for (size_t i = blockSize + 5; i < blockSize + 7; ++i) s.write(i);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Get position, check can_read and set position" << std::endl;
	tpie::stream_position pos2 = s.get_position();
	TEST_ASSERT(!s.can_read());
	s.set_position(pos1);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Check can_read" << std::endl;
	TEST_ASSERT(s.can_read());
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Check read" << std::endl;
	TEST_ASSERT(s.read() == blockSize + 5);
	tpie::log_debug() << s.describe() << std::endl;
	TEST_ASSERT(s.can_read());

	tpie::log_debug() << "Check read" << std::endl;
	TEST_ASSERT(s.read() == blockSize + 6);
	TEST_ASSERT(!s.can_read());
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Check get_position" << std::endl;
	TEST_ASSERT(s.get_position() == pos2);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Check write" << std::endl;
	s.write(blockSize + 7);
	tpie::log_debug() << s.describe() << std::endl;

	return true;
}

static bool position_test_3(size_t n) {
	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);

	size_t i = 0;
	size_t j = 1;
	size_t k = 1;
	s.write(0);
	while (k < n) {
		TEST_ASSERT(s.offset() == k);
		s.seek(0);
		TEST_ASSERT(s.offset() == 0);
		s.read();
		TEST_ASSERT(s.offset() == 1);
		if (k % 2 == 0) {
			tpie::log_debug() << "Seek to end" << std::endl;
			s.seek(0, tpie::file_stream_base::end);
		} else {
			tpie::log_debug() << "Scan to end" << std::endl;
			for (size_t n = 1; n < k; ++n) {
				TEST_ASSERT(s.read() == n);
			}
		}
		TEST_ASSERT(s.offset() == k);
		size_t target = i+j;
		i = j;
		j = target;
		tpie::log_debug() << "[" << k << ", " << target << ")" << std::endl;
		while (k < target) s.write(k++);
	}
	return true;
}

static bool reopen_test_1(size_t n) {
	tpie::temp_file tf;

	size_t i = 0;
	size_t j = 1;
	size_t k = 0;

	while (k < n) {
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);

		TEST_ASSERT(s.offset() == 0);
		TEST_ASSERT(s.size() == k);

		if (k % 2 == 0) {
			tpie::log_debug() << "Seek to end" << std::endl;
			s.seek(0, tpie::file_stream_base::end);
		} else {
			tpie::log_debug() << "Scan to end" << std::endl;
			for (size_t n = 0; n < k; ++n) {
				TEST_ASSERT(s.read() == n);
			}
		}

		TEST_ASSERT(s.offset() == k);
		TEST_ASSERT(!s.can_read());

		size_t target = i+j;
		i = j;
		j = target;
		tpie::log_debug() << "[" << k << ", " << target << ")" << std::endl;
		while (k < target) s.write(k++);
	}
	return true;
}

static bool reopen_test_2() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);

	tpie::stream_position pos1a;
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		for (size_t i = 0; i < blockSize; ++i) s.write(i);
		TEST_ASSERT(s.size() == blockSize);
		pos1a = s.get_position();
	}
	tpie::stream_position pos1b;
	tpie::stream_position pos2b;
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		TEST_ASSERT(s.size() == blockSize);
		TEST_ASSERT(s.offset() == 0);
		for (size_t i = 0; i < blockSize; ++i) {
			TEST_ASSERT(i == s.read());
		}
		TEST_ASSERT(s.offset() == blockSize);
		pos1b = s.get_position();
		for (size_t i = blockSize; i < 2*blockSize; ++i) s.write(i);
		pos2b = s.get_position();
		TEST_ASSERT(s.offset() == 2*blockSize);
		TEST_ASSERT(s.size() == 2*blockSize);
	}
	TEST_ASSERT(pos1a == pos1b);
	tpie::stream_position pos2c;
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		TEST_ASSERT(s.size() == 2*blockSize);
		s.set_position(pos1b);
		TEST_ASSERT(s.offset() == blockSize);
		TEST_ASSERT(s.size() == 2*blockSize);
		for (size_t i = blockSize; i < 2*blockSize; ++i) {
			TEST_ASSERT(i == s.read());
		}
		pos2c = s.get_position();
	}
	TEST_ASSERT(pos2b == pos2c);
	return true;
}

static bool seek_test() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	TEST_ASSERT(s.offset() == 0);
	for (size_t i = 0; i < blockSize + 1; ++i) s.write(i);
	TEST_ASSERT(s.offset() == blockSize + 1);
	s.seek(0);
	TEST_ASSERT(s.offset() == 0);
	s.seek(0, tpie::file_stream_base::end);
	TEST_ASSERT(s.offset() == blockSize + 1);
	s.seek(0);
	TEST_ASSERT(s.offset() == 0);
	return true;
}

static bool seek_test_2() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	for (size_t i = 0; i < blockSize; ++i) s.write(i);
	s.seek(0);
	s.seek(0, tpie::file_stream_base::end);
	s.write(0);
	return true;
}

static bool position_test_4() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	for (size_t i = 0; i < blockSize; ++i) s.write(i);
	tpie::stream_position pos = s.get_position();
	s.write(blockSize);
	s.seek(0);
	s.set_position(pos);
	TEST_ASSERT(s.read() == blockSize);
	return true;
}

static bool truncate_test() {
	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	s.write(0);
	s.truncate(0);
	TEST_ASSERT(s.offset() == 0);
	TEST_ASSERT(s.size() == 0);
	TEST_ASSERT(!s.can_read());
	return true;
}

static bool truncate_test_2() {
	size_t a = 1000000;
	size_t b = 2000000;
	size_t c = 3000000;

	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);

	for (size_t i = 0; i < a; ++i) s.write(i);
	tpie::stream_position pos1 = s.get_position();
	for (size_t i = a; i < b; ++i) s.write(i);
	tpie::stream_position pos2 = s.get_position();
	for (size_t i = b; i < c; ++i) s.write(i);
	s.truncate(pos2);
	TEST_ASSERT(s.offset() == b);
	s.close();
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	TEST_ASSERT(s.offset() == 0);
	TEST_ASSERT(s.size() == b);
	s.truncate(pos1);
	TEST_ASSERT(s.offset() == 0);
	TEST_ASSERT(s.size() == a);
	return true;
}

static bool position_test_5() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	for (size_t i = 0; i < 2*blockSize; ++i) s.write(i);
	tpie::stream_position pos1 = s.get_position();
	s.seek(0);
	s.read();
	s.seek(0, tpie::file_stream_base::end);
	tpie::stream_position pos2 = s.get_position();
	TEST_ASSERT(pos1 == pos2);
	return true;
}

static bool position_test_6() {
	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	s.write(1);
	s.write(2);
	s.write(3);
	s.seek(0);
	s.read();
	s.seek(0);
	s.read();
	s.read();
	return true;
}

static bool position_test_7() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	for (size_t i = 0; i < blockSize; ++i) s.write(i);
	tpie::stream_position pos1 = s.get_position();
	tpie::stream_position pos2 = s.get_position();
	TEST_ASSERT(pos1 == pos2);
	s.close();
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	for (size_t i = 0; i < blockSize; ++i) s.read();
	tpie::stream_position pos3 = s.get_position();
	TEST_ASSERT(pos1 == pos3);
	s.close();
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	s.seek(0, tpie::file_stream_base::end);
	tpie::stream_position pos4 = s.get_position();
	TEST_ASSERT(pos1 == pos4);
	return true;
}

static bool position_test_8() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		for (size_t i = 0; i < blockSize; ++i) s.write(i);
	}
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		s.seek(0, tpie::file_stream_base::end);
		s.get_position();
	}
	return true;
}

static bool position_test_9() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	for (size_t i = 0; i < 2*blockSize; ++i) s.write(i);
	s.close();
	tf.free();
	{
		tpie::file_stream<size_t> s2;
		s2.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		for (size_t i = 0; i < blockSize; ++i) s2.write(i);
	}
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
	s.seek(0, tpie::file_stream_base::end);
	s.get_position();
	return true;
}

static bool uncompressed_test(size_t n) {
	tpie::temp_file tf;
	{
		tpie::uncompressed_stream<size_t> s;
		s.open(tf);
		for (size_t i = 0; i < n; ++i) s.write(~i);
	}
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		TEST_ASSERT(s.is_open());
		TEST_ASSERT(s.can_read());
		for (size_t i = 0; i < n; ++i) {
			TEST_ASSERT(s.can_read());
			size_t x = s.read();
			TEST_ASSERT(x == ~i);
		}
		TEST_ASSERT(!s.can_read());
	}
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);
		TEST_ASSERT(s.is_open());
		TEST_ASSERT(s.can_read());
		size_t i = 0;
		while (i < n) {
			TEST_ASSERT(s.can_read());
			size_t x = s.read();
			TEST_ASSERT(x == ~i);
			++i;
			if (i != n) {
				TEST_ASSERT(s.can_read());
				s.write(i);
				++i;
			}
		}
		TEST_ASSERT(!s.can_read());
	}
	{
		tpie::uncompressed_stream<size_t> s;
		s.open(tf);
		TEST_ASSERT(s.is_open());
		TEST_ASSERT(s.can_read());
		size_t i = 0;
		while (i < n) {
			TEST_ASSERT(s.can_read());
			size_t x = s.read();
			TEST_ASSERT(x == ~i);
			++i;
			if (i != n) {
				TEST_ASSERT(s.can_read());
				x = s.read();
				TEST_ASSERT(x == i);
				++i;
			}
		}
		TEST_ASSERT(!s.can_read());
	}
	return true;
}

static bool uncompressed_new_test(size_t n) {
	tpie::temp_file tf;
	{
		tpie::file_stream<size_t> s;
		s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, tpie::compression_none);
		for (size_t i = 0; i < n; ++i) s.write(i);
	}
	{
		tpie::uncompressed_stream<size_t> s;
		s.open(tf);
		for (size_t i = 0; i < n; ++i) {
			TEST_ASSERT(s.read() == i);
		}
	}
	return true;
}

static bool backwards_test(size_t n) {
	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, flags);

	size_t x = 0;
	size_t y = 1;

	size_t offs = 0;
	while (true) {
		while (s.can_read()) {
			TEST_ASSERT(s.offset() == offs);
			size_t r = s.read();
			if (r != offs) {
				tpie::log_error() << "Got " << r << ", expected " << offs << std::endl;
				TEST_ASSERT(r == offs);
			}
			++offs;
		}
		tpie::log_debug() << "Write forward until " << y << std::endl;
		while ((offs < y) && (offs < n)) {
			TEST_ASSERT(s.offset() == offs);
			s.write(offs++);
		}
		if (offs == n) break;
		tpie::log_debug() << "Read backwards until " << x << std::endl;
		while (offs > x) {
			TEST_ASSERT(s.offset() == offs);
			size_t r = s.read_back();
			--offs;
			if (r != offs) {
				tpie::log_error() << "Got " << r << ", expected " << offs << std::endl;
				TEST_ASSERT(r == offs);
			}
		}
		std::swap(x, y);
		y += x;
	}

	return true;
}

};

static bool backwards_file_stream_test(size_t n) {
	tpie::temp_file tf;
	tpie::uncompressed_stream<size_t> s;
	s.open(tf);

	size_t x = 0;
	size_t y = 1;

	size_t offs = 0;
	while (true) {
		while (s.can_read()) {
			TEST_ASSERT(s.offset() == offs);
			size_t r = s.read();
			if (r != offs) {
				tpie::log_error() << "Got " << r << ", expected " << offs << std::endl;
				TEST_ASSERT(r == offs);
			}
			++offs;
		}
		tpie::log_debug() << "Write forward until " << y << std::endl;
		while ((offs < y) && (offs < n)) {
			TEST_ASSERT(s.offset() == offs);
			s.write(offs++);
		}
		if (offs == n) break;
		tpie::log_debug() << "Read backwards until " << x << std::endl;
		while (offs > x) {
			TEST_ASSERT(s.offset() == offs);
			size_t r = s.read_back();
			--offs;
			if (r != offs) {
				tpie::log_error() << "Got " << r << ", expected " << offs << std::endl;
				TEST_ASSERT(r == offs);
			}
		}
		std::swap(x, y);
		y += x;
	}

	return true;
}

struct odd_block_size_item {
	int a;
	int padding[6];
};

static bool odd_block_size_test() {
	typedef odd_block_size_item item_type;
	tpie::temp_file tf;
	item_type item;
	item.a = 0;
	std::fill(item.padding, item.padding + 6, 0x55555555);
	{
		tpie::uncompressed_stream<item_type> s;
		tpie::log_debug() << s.block_size() << std::endl;
		s.open(tf);
		tpie::log_debug() << s.block_size() << std::endl;
		for (size_t i = 0; i < 1024*1024; ++i) s.write(item), item.a++;
		s.close();
	}
	item.a = 0;
	{
		tpie::file_stream<item_type> s;
		tpie::log_debug() << s.block_size() << std::endl;
		s.open(tf);
		tpie::log_debug() << s.block_size() << std::endl;
		for (size_t i = 0; i < 1024*1024; ++i) {
			item_type in = s.read();
			if (in.a != item.a++) return false;
		}
		s.close();
	}
	return true;
}

static bool write_peek_test(size_t n) {
	tpie::temp_file tf;
	tpie::file_stream<size_t> s;
	s.open(tf, tpie::access_read_write, 0, tpie::access_sequential, tpie::compression_none);

	for (size_t i = 0; i < n; ++i) {
		s.write(i);
	}

	s.seek(0);

	for (size_t i = 0; i < n; ++i) {
		size_t x = s.peek();
		s.write(~x);
	}

	return true;
}

void create_file(const std::string & path) {
	tpie::file_stream<int> fs;
	fs.open(path, tpie::open::write_only);
	fs.write(42);
	fs.close();
}

/* bool read_only_test() {
	{
		bool caughtException = false;
		tpie::temp_file tf;
		create_file(tf.path()); // temp_file doesn't actually create the file

		tpie::file_stream<int> fs;
		fs.open(tf.path(), tpie::open::read_only);

		try {
			fs.write(42);
			fs.close();
		} catch (tpie::io_exception e) {
			caughtException = true;
		}

		if (!caughtException) {
			tpie::log_error() << "Didn't throw on write() for named file" << std::endl;
			return false;
		}
	}
	{
		tpie::temp_file tf;
		create_file(tf.path()); // temp_file doesn't actually create the file
		bool caughtException = false;
		tpie::file_stream<int> fs;
		fs.open(tf, tpie::open::read_only);
		try {
			fs.write(42);
		} catch (tpie::io_exception) {
			caughtException = true;
		}
		if (!caughtException) {
			tpie::log_error() << "Didn't throw on write() for temp file" << std::endl;
			return false;
		}
	}
	return true;
} */

bool write_only_test() {
	bool success = true;
	{
		tpie::temp_file tf;
		{
			tpie::file_stream<int> fs;
			fs.open(tf);
			fs.write(42);
		}
		tpie::file_stream<int> fs;
		fs.open(tf.path(), tpie::open::write_only);
		if (fs.size() != 0) {
			tpie::log_error() << "Didn't truncate named file" << std::endl;
			success = false;
		}
	}
	{
		tpie::temp_file tf;
		{
			tpie::file_stream<int> fs;
			fs.open(tf);
			fs.write(42);
		}
		tpie::file_stream<int> fs;
		fs.open(tf, tpie::open::write_only);
		if (fs.size() != 0) {
			tpie::log_error() << "Didn't truncate temp file" << std::endl;
			success = false;
		}
	}
	return success;
}

bool stack_test() {
	constexpr uint32_t block_items = 4;
	auto bof = tpie::file_stream<uint32_t>::calculate_block_factor(block_items);
	tpie::file_stream<uint32_t> fs_a(bof), fs_b(bof), fs_c(bof);
	fs_a.open(tpie::compression_none);
	fs_b.open(tpie::compression_none);
	fs_c.open(tpie::compression_all);

	constexpr uint32_t cnt = block_items * 2; 

	for (uint32_t i=0; i < cnt; ++i) {
		fs_a.write(i);
		fs_b.write(i);
		fs_c.write(i);
	}
	
	for (uint32_t i=cnt-1; i < cnt; --i) {
		uint32_t val;
		val = fs_a.read_back();
		if (i != val) {
			tpie::log_error() << "Bad value " << val << " instead of " << i << std::endl;
			return false;
		}
		val = fs_b.read_back();
		if (i != val) {
			tpie::log_error() << "Bad value " << val << " instead of " << i << std::endl;
			return false;
		}
		val = fs_c.read_back();
		if (i != val) {
			tpie::log_error() << "Bad value " << val << " instead of " << i << std::endl;
			return false;
		}
	}
		
	return true;
}

template <tpie::compression_flags flags>
tpie::tests & add_tests(tpie::tests & t, std::string suffix) {
	typedef tests<flags> T;
	return t
		.test(T::basic_test, "basic" + suffix, "n", static_cast<size_t>(1000))
		.test(T::seek_test, "seek" + suffix)
		.test(T::seek_test_2, "seek_2" + suffix)
		.test(T::reopen_test_1, "reopen_1" + suffix, "n", static_cast<size_t>(1 << 21))
		.test(T::reopen_test_2, "reopen_2" + suffix)
		.test(T::read_seek_test, "read_seek" + suffix, "m", static_cast<size_t>(1 << 10), "n", static_cast<size_t>(1 << 15))
		.test(T::read_back_seek_test, "read_back_seek" + suffix)
		.test(T::read_back_seek_test_2, "read_back_seek_2" + suffix)
		.test(T::read_back_throw_test, "read_back_throw" + suffix)
		.test(T::truncate_test, "truncate" + suffix)
		.test(T::truncate_test_2, "truncate_2" + suffix)
		.test(T::position_test_0, "position_0" + suffix, "n", static_cast<size_t>(1 << 19))
		.test(T::position_test_1, "position_1" + suffix)
		.test(T::position_test_2, "position_2" + suffix)
		.test(T::position_test_3, "position_3" + suffix, "n", static_cast<size_t>(1 << 21))
		.test(T::position_test_4, "position_4" + suffix)
		.test(T::position_test_5, "position_5" + suffix)
		.test(T::position_test_6, "position_6" + suffix)
		.test(T::position_test_7, "position_7" + suffix)
		.test(T::position_test_8, "position_8" + suffix)
		.test(T::position_test_9, "position_9" + suffix)
		.test(T::position_seek_test, "position_seek" + suffix)
		.test(T::uncompressed_test, "uncompressed" + suffix, "n", static_cast<size_t>(1000000))
		.test(T::uncompressed_new_test, "uncompressed_new" + suffix, "n", static_cast<size_t>(1000000))
		.test(T::backwards_test, "backwards" + suffix, "n", static_cast<size_t>(1 << 23))
		;
}

int main(int argc, char ** argv) {
	tpie::tests t(argc, argv);
	return add_tests<tpie::compression_none>
		(add_tests<tpie::compression_normal>
		 (t, ""), "_u")
		.test(backwards_file_stream_test, "backwards_fs", "n", static_cast<size_t>(1 << 23))
		.test(odd_block_size_test, "odd_block_size")
		.test(write_peek_test, "write_peek", "n", static_cast<size_t>(1 << 23))
		/* .test(read_only_test, "read_only") */
		.test(write_only_test, "write_only")
		.test(stack_test, "lockstep_reverse");
}
