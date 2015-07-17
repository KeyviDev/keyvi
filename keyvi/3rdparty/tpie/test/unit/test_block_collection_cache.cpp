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

// block_collection_cache usage test

#include "common.h"
#include <tpie/tpie.h>
#include <tpie/blocks/block_collection_cache.h>
#include <tpie/tempname.h>
#include <vector>
#include <deque>
#include <algorithm>
#include <tpie/file_accessor/file_accessor.h>

using namespace tpie;
using namespace tpie::blocks;

const memory_size_type BLOCK_SIZE = 1024 * 5;

memory_size_type random(memory_size_type i) {
	return 179424673 * i + 15485863;
}

bool basic() {
	temp_file file;
	block_collection_cache collection(file.path(), BLOCK_SIZE, 5, true);
	
	std::vector<block_handle> blocks;

	// write 20 twenty blocks of random sizes
	for(char i = 0; i < 20; ++i) {
		block_handle handle = collection.get_free_block();

		TEST_ENSURE_EQUALITY(BLOCK_SIZE, handle.size, "The size of the returned block is not correct.");

		block * b = collection.read_block(handle);

		for(block::iterator j = b->begin(); j != b->end(); ++j)
			*j = i;

		collection.write_block(handle);
		blocks.push_back(handle);
	}

	log_debug() << "Finished writing blocks." << std::endl;

	// verify the content of the 20 blocks
	for(char i = 0; i < 20; ++i) {
		block_handle handle = blocks[i];

		block * b = collection.read_block(handle);

		TEST_ENSURE_EQUALITY(handle.size, b->size(), "The block size should be equal to the handle size");

		for(block::iterator j = b->begin(); j != b->end(); ++j)
			TEST_ENSURE_EQUALITY((int) *j, (int) i, "the content of the returned block is not correct");
	}

	log_debug() << "Finished reading blocks." << std::endl;
	return true;
}

bool erase() {
	typedef std::list<std::pair<block_handle, char> > block_list_t;
	
	temp_file file;
	block_collection_cache collection(file.path(), BLOCK_SIZE, 5, true);
	block_list_t blocks;

	// write 20 twenty blocks of random sizes
	for(char i = 0; i < 20; ++i) {
		block_handle handle = collection.get_free_block();

		TEST_ENSURE_EQUALITY(BLOCK_SIZE, handle.size, "The size of the returned block is not correct.");

		block * b = collection.read_block(handle);

		for(block::iterator j = b->begin(); j != b->end(); ++j)
			*j = i;

		collection.write_block(handle);
		blocks.push_back(std::make_pair(handle, i));
	}

	// repeat 40 times: free a block then allocate a block
	for(char i = 20; i < 40; ++i) {
		// free a block
		block_list_t::iterator j = blocks.begin();
		std::advance(j, random(i) % blocks.size());
		collection.free_block(j->first);
		blocks.erase(j);

		// allocate a new block
		block_handle handle = collection.get_free_block();

		TEST_ENSURE_EQUALITY(BLOCK_SIZE, handle.size, "The size of the returned block is not correct.");

		block * b = collection.read_block(handle);

		for(block::iterator j = b->begin(); j != b->end(); ++j)
			*j = i;

		collection.write_block(handle);
		blocks.push_back(std::make_pair(handle, i));
	}

	// verify the content of the 20 blocks
	for(block_list_t::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		block_handle handle = i->first;
		char content = i->second;

		block * b = collection.read_block(handle);

		TEST_ENSURE_EQUALITY(handle.size, b->size(), "The block size should be equal to the handle size");

		for(block::iterator j = b->begin(); j != b->end(); ++j)
			TEST_ENSURE_EQUALITY((int) *j, (int) content, "the content of the returned block is not correct"); // cast to int for human-readable human
	}

	return true;
}

bool overwrite() {
	typedef std::list<std::pair<block_handle, char> > block_list_t;

	temp_file file;
	block_collection_cache collection(file.path(), BLOCK_SIZE, 5, true);
	block_list_t blocks;

	// write 20 twenty blocks of random sizes
	for(char i = 0; i < 20; ++i) {
		block_handle handle = collection.get_free_block();

		TEST_ENSURE_EQUALITY(BLOCK_SIZE, handle.size, "The size of the returned block is not correct.");

		block * b = collection.read_block(handle);

		for(block::iterator j = b->begin(); j != b->end(); ++j)
			*j = i;

		collection.write_block(handle);
		blocks.push_back(std::make_pair(handle, i));
	}

	// repeat 20 times: overwrite a random block
	for(char i = 20; i < 40; ++i) {
		// select a random block handle from the list
		block_list_t::iterator j = blocks.begin();
		std::advance(j, random(i) % blocks.size());
		block_handle h = j->first;

		// overwrite the contents of the block
		block * b = collection.read_block(h);
		for(block::iterator j = b->begin(); j != b->end(); ++j)
			*j = i;
		collection.write_block(h);

		// update the block list
		blocks.erase(j);
		blocks.push_back(std::make_pair(h, i));
	}

	// verify the content of the 20 blocks
	for(block_list_t::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		block_handle handle = i->first;
		char content = i->second;

		block * b = collection.read_block(handle);

		TEST_ENSURE_EQUALITY(handle.size, b->size(), "The block size should be equal to the handle size");

		for(block::iterator j = b->begin(); j != b->end(); ++j)
			TEST_ENSURE_EQUALITY((int) *j, (int) content, "the content of the returned block is not correct"); // cast to int for human-readable human
	}
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic, "basic")
		.test(erase, "erase")
		.test(overwrite, "overwrite");
}
