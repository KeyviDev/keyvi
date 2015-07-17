// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino=(0 :
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

// block_collection usage test

#include <tpie/tpie.h>
#include "common.h"
#include <set>
#include <tpie/tempname.h>
#include <tpie/blocks/freespace_collection.h>

using namespace tpie;
using namespace tpie::blocks;
using namespace tpie::blocks::bits;

// return: whether the given position is in the interval covered by the block_handle
bool inside(block_handle handle, stream_size_type position) {
	return (handle.position <= position && position < handle.position + handle.size);
}

// return: whether the interval the handles cover overlap
bool overlaps(block_handle a, block_handle b) {
	return inside(a, b.position) || inside(b, a.position);
}

// random an integer in the range [min; max)
memory_size_type random(memory_size_type seed, memory_size_type min, memory_size_type max) {
	return (seed * 1009) % (max - min) + min;
}

int random_generator(int i) {
	return 10007 % i;
}

bool alloc_test(memory_size_type size, memory_size_type block_size) {
	typedef std::vector<block_handle> handles_t;
	temp_file file; 
	freespace_collection collection(file.path(), block_size);
	handles_t handles;

	for(memory_size_type i = 0; i < size; ++i) {
		block_handle handle = collection.alloc();
		TEST_ENSURE(handle.size == block_size, "The size of the returned handle is too small");

		for(handles_t::iterator j = handles.begin(); j != handles.end(); ++j) {
			if(overlaps(handle, *j)) {
				log_debug() << "Handle overlap "
							<< "[" << handle.position << ", " << handle.position + handle.size  << ") "
							<< "[" << j->position << ", " << j->position + j->size << ")" << std::endl;
			}

			TEST_ENSURE(!overlaps(handle, *j), "The returned handle overlaps with an existing handle");
		}

		handles.push_back(handle);
	}

	return true;
}

bool size_test(memory_size_type size, memory_size_type block_size) {
	typedef std::vector<block_handle> handles_t;
	typedef std::vector<memory_size_type> sizes_t;

	temp_file file;
	freespace_collection collection(file.path(), block_size);
	handles_t handles;
	sizes_t sizes;

	stream_size_type minimum_size = 0;

	for(memory_size_type i = 0; i < size; ++i) {
		TEST_ENSURE(collection.size() >= minimum_size, "The space used is too small.");
		
		minimum_size += block_size;

		block_handle handle = collection.alloc();
		handles.push_back(handle);
		sizes.push_back(block_size);
	}

	//  the two arrays are shuffled the same way
	std::random_shuffle(handles.begin(), handles.end(), random_generator);
	std::random_shuffle(sizes.begin(), sizes.end(), random_generator);

	for(memory_size_type i = 0; i < size; ++i) {
		TEST_ENSURE(collection.size() >= minimum_size, "The space used is too small.");
		collection.free(handles[i]);
		minimum_size -= sizes[i];
	}

	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(alloc_test, "alloc", "size", 1000, "block_size", 1024)
		.test(size_test, "size", "size", 1000, "block_size", 1024);
}