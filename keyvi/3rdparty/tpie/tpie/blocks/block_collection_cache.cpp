// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2015, The TPIE development team
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

#include <tpie/blocks/block_collection_cache.h>

namespace tpie {

namespace blocks {

block_collection_cache::block_collection_cache(std::string fileName, memory_size_type blockSize, memory_size_type maxSize, bool writeable)
	: m_collection(fileName, blockSize, writeable)
	, m_curSize(0)
	, m_maxSize(maxSize)
	, m_blockSize(blockSize)
{}

block_collection_cache::~block_collection_cache() {
	// write the content of the cache to disk
	block_map_t::iterator end = m_blockMap.end();

	for(block_map_t::iterator i = m_blockMap.begin(); i != end; ++i) {
		if(i->second.dirty) {
			m_collection.write_block(i->first, *i->second.pointer);
		}
		tpie_delete(i->second.pointer);
	}
}

block_handle block_collection_cache::get_free_block() {
	block_handle h = m_collection.get_free_block();
	block * cache_b = tpie_new<block>(m_blockSize);
	add_to_cache(h, cache_b, true);
	return h;
}

void block_collection_cache::free_block(block_handle handle) {
	tp_assert(handle.size == m_blockSize, "the size of the handle is not correct")
	tp_assert(m_curSize > 0, "the current size of the cache is 0");

	block_map_t::iterator i = m_blockMap.find(handle);

	if(i != m_blockMap.end()) {
		m_blockList.erase(i->second.iterator);
		tpie_delete(i->second.pointer);
		--m_curSize;
		m_blockMap.erase(i);
	}

	m_collection.free_block(handle);
}

void block_collection_cache::prepare_cache() {
	if(m_curSize < m_maxSize)
		return;

	// write the last accessed block to disk
	block_handle handle = m_blockList.front();
	m_blockList.pop_front();

	block_map_t::iterator i = m_blockMap.find(handle);
	if(i->second.dirty)
		m_collection.write_block(i->first, *(i->second.pointer));
	tpie_delete(i->second.pointer);
	--m_curSize;
	m_blockMap.erase(i);
}

void block_collection_cache::add_to_cache(block_handle handle, block * b, bool dirty) {
	m_blockList.push_back(handle);
	block_list_t::iterator list_pos = m_blockList.end();
	--list_pos;

	m_blockMap[handle] = block_information_t(b, list_pos, dirty);
	++m_curSize;
}

block * block_collection_cache::read_block(block_handle handle) {
	block_map_t::iterator i = m_blockMap.find(handle);

	if(i != m_blockMap.end()) { // the block is already in the cache
		// update the block list to reflect that the block was accessed
		m_blockList.erase(i->second.iterator);
		m_blockList.push_back(i->first);

		block_list_t::iterator j = m_blockList.end();
		--j;

		i->second.iterator = j;

		// return the block content from cache
		return i->second.pointer;
	}

	// the block isn't in the cache
	prepare_cache(); // make space in the cache for the new block

	block * cache_b = tpie_new<block>();
	m_collection.read_block(handle, *cache_b);
	add_to_cache(handle, cache_b, false);

	return cache_b;
}

void block_collection_cache::write_block(block_handle handle) {
	block_map_t::iterator i = m_blockMap.find(handle);

	tp_assert(i != m_blockMap.end(), "the given handle does not exist in the cache.");

	m_blockList.erase(i->second.iterator);
	m_blockList.push_back(handle);

	block_list_t::iterator j = m_blockList.end();
	--j;

	i->second.iterator = j;
	i->second.dirty = true;
}

} // namespace blocks
} // namespace tpie
