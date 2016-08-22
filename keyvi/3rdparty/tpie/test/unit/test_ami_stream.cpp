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
#include <random>
#include <tpie/tpie.h>

#include <tpie/array.h>
#include <tpie/stream.h>
#include <tpie/util.h>
#include <vector>
#include <tpie/progress_indicator_arrow.h>
#include <tpie/types.h>

using tpie::uint64_t;

inline uint64_t ITEM(size_t i) {return i*98927 % 104639;}
static const size_t TESTSIZE = 8*1024*1024;
static const size_t ITEMS = TESTSIZE/sizeof(uint64_t);
static const size_t ARRAYSIZE = 512;
static const size_t ARRAYS = TESTSIZE/(ARRAYSIZE*sizeof(uint64_t));

bool basic() {

	tpie::temp_file tmp;

	// Write ITEMS items sequentially to the temporary file
	{
		tpie::ami::stream<uint64_t> s(tmp.path(), tpie::ami::WRITE_STREAM);
		for(size_t i=0; i < ITEMS; ++i) s.write_item(ITEM(i));
	}

	// Sequential verify
	{
		tpie::ami::stream<uint64_t> s(tmp.path(), tpie::ami::READ_STREAM);
		uint64_t *x = 0;
		for(size_t i=0; i < ITEMS; ++i) {
			s.read_item(&x);
			if (*x != ITEM(i)) {
				tpie::log_error() << "Expected element " << i << " = " << ITEM(i) << ", got " << *x << std::endl;
				return false;
			}
		}
	}

	// Write an ARRAYSIZE array ARRAYS times sequentially to the temporary file
	{
		tpie::ami::stream<uint64_t> s(tmp.path(), tpie::ami::WRITE_STREAM);
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYSIZE; ++i) {
			x[i] = ITEM(i);
		}
		for(size_t i=0; i < ARRAYS; ++i) s.write_array(x, ARRAYSIZE);
	}

	// Sequentially verify the arrays
	{
		tpie::ami::stream<uint64_t> s(tmp.path(), tpie::ami::READ_STREAM);
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYS; ++i) {
			TPIE_OS_SIZE_T len = ARRAYSIZE;
			s.read_array(x, len);
			if (len != ARRAYSIZE) {
				tpie::log_error() <<  "read_array only read " << len << " elements, expected " << ARRAYSIZE << std::endl;
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
		tpie::ami::stream<uint64_t> s(tmp.path(), tpie::ami::WRITE_STREAM);
		tpie::array<uint64_t> data(ITEMS);
		for (size_t i=0; i < ITEMS; ++i) {
			data[i] = ITEM(i);
			s.write_item(data[i]);
		}
		for (size_t i=0; i < 10; ++i) {
			// Seek to random index
			size_t idx = ITEM(i) % ITEMS;
			s.seek(idx);

			if (i%2 == 0) {
				uint64_t *read = nullptr;
				s.read_item(&read);
				if (*read != data[idx]) {
					tpie::log_error() << "Expected element " << idx << " to be " << data[idx] << ", got " << *read << std::endl;
					return false;
				}
			} else {
				uint64_t write = ITEM(ITEMS+i);
				data[idx] = write;
				s.write_item(write);
			}

			tpie::stream_offset_type newoff = s.tell();
			if (newoff != tpie::stream_offset_type(idx+1) ) {
				tpie::log_error() << "Offset advanced to " << newoff << ", expected " << (idx+1) << std::endl;
				return false;
			}
		}
	}
	return true;
}

bool truncate_test() {
	try {
		tpie::ami::stream<int> stream;
		stream.truncate(134138027);
		stream.seek(0);
		stream.write_item(42);
		stream.truncate(134199667);
		stream.seek(325595);
		int * it;
		stream.read_item(&it);
	} catch(std::runtime_error e) {
		std::cout << "EXCEPTION " << e.what() << " " << typeid(e).name()<< std::endl;
		return false;
	}
	return true;
}

int stress(size_t actions=1024*1024*10, size_t max_size=1024*1024*128) {
	try {
		tpie::progress_indicator_arrow pi("Test", actions);
		const size_t chunk_size=1024*128;
		std::vector<int> elements(max_size, 0);
		std::vector<bool> defined(max_size, true);
		std::vector<int> arr(chunk_size);
		size_t location=0;
		size_t size=0;
	
		std::mt19937 rng;
		std::uniform_int_distribution<> todo(0, 6);
		std::uniform_int_distribution<> ddist(0, 123456789);
		tpie::ami::stream<int> stream;
		pi.init(actions);
		for(size_t action=0; action < actions; ++action) {
			switch(todo(rng)) {
			case 0: //READ
			{
				size_t cnt=size-location;
				if (cnt > 0) {
					std::uniform_int_distribution<> d(1,std::min<size_t>(cnt, chunk_size));
					cnt=d(rng);
					std::cerr << location << " " << size << " read: " << cnt << std::endl;
					for (size_t i=0; i < cnt; ++i) {
						int * item;
						if (stream.read_item(&item) != tpie::ami::NO_ERROR) {
							std::cout << "Should be able to read" << std::endl;
							return false;
						}
						if (defined[location]) {
							if (elements[location] != *item) {
								std::cerr << "Found " << *item << " expected " << elements[location] << std::endl;
								return false;
							}
						} else {
							defined[location] = true;
							elements[location] = *item;
						}
						++location;
					}
				} else {
					int * item;
					if (stream.read_item(&item) == tpie::ami::NO_ERROR) {
						std::cerr << "Should not be able to read" << std::endl;
						return false;
					}
				}
				break;
			}
			case 1: //WRITE
			{
				std::uniform_int_distribution<> d(1,chunk_size);
				size_t cnt=std::min<size_t>(d(rng), max_size-location);
				std::cerr << location << " " << size << " write: " << cnt << std::endl;
				for (size_t i=0; i < cnt; ++i) {
					elements[location] = ddist(rng);
					defined[location] = true;
					stream.write_item(elements[location]);
					location++;
				}
				size = std::max(size, location);
				break;
			}
			case 2: //SEEK END
			{
				std::cerr << location << " " << size << " seek: " << size << std::endl;
				location = size;
				stream.seek(location);
				break;
			}
			case 3: //SEEK SOMEWHERE
			{
				std::uniform_int_distribution<> d(0, size);
				size_t l = d(rng);
				std::cerr << location << " " << size << " seek: " << l << std::endl;
				location = l;
				stream.seek(location);
				break;
			}
			case 4: //READ ARRAY
			{
				size_t cnt=size-location;
				if (cnt > 0) {
					std::uniform_int_distribution<> d(1,std::min<size_t>(cnt, chunk_size));
					cnt=d(rng);
					std::cerr << location << " " << size << " read array: " << cnt << std::endl;
					stream.read_array(&arr[0], cnt);
					for (size_t i=0; i < cnt; ++i) {
						if (defined[location]) {
							if (elements[location] != arr[i]) {
								std::cerr << "Found " << arr[i] << " expected " << elements[location] << std::endl;
								return false;
							}
						} else {
							defined[location] = true;
							elements[location] = arr[i];
						}
						++location;
					}
				}
			}
			case 5: //WRITE ARRAY
			{
				 std::uniform_int_distribution<> d(1,chunk_size);
				 size_t cnt=std::min<size_t>(d(rng), max_size-location);
				 std::cerr << location << " " << size << " write array: " << cnt << std::endl;
				 for (size_t i=0; i < cnt; ++i) {
					 arr[i] = elements[location] = ddist(rng);
					 defined[location] = true;
					 ++location;
				 }
				 stream.write_array(&arr[0], cnt);
				 size = std::max(size, location);
				 break;
			}
			case 6: //TRUNCATE 
			{
				std::uniform_int_distribution<> d(std::max(0, (int)size-(int)chunk_size), std::min(size+chunk_size, max_size));
				size_t ns=d(rng);
				std::cerr << location << " " << size << " truncate: " << ns << std::endl;	
				stream.truncate(ns);
				stream.seek(0);
				location=0;
				for (size_t i=size; i < ns; ++i)
					defined[i] = false;
				size=ns;
				break;
			}
			}
			//std::cout << location << " " << size << std::endl;
			if (stream.stream_len() != (tpie::stream_offset_type)size) {
				std::cerr << "Bad size" << std::endl;
				return false;
			}
			if (stream.tell() != (tpie::stream_offset_type)location) {
				std::cerr << "Bad offset" << std::endl;
				return false;
			}
			pi.step();
		}
		pi.done();
	} catch(std::runtime_error e) {
		std::cerr << "EXCEPTION " << e.what() << " " << typeid(e).name()<< std::endl;
		return false;
	}
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic, "basic")
		.test(stress, "stress", "actions", 1024*1024*10, "maxsize", 1024*1024*128)
		.test(truncate_test, "truncate");
}
