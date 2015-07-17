// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2014, 2015, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file block_collection_cache Class to handle caching of the block collection
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_BLOCKS_BLOCK_COLLECTION_CACHE_H
#define _TPIE_BLOCKS_BLOCK_COLLECTION_CACHE_H

#include <tpie/tpie.h>
#include <tpie/tpie_assert.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/blocks/block.h>
#include <tpie/blocks/block_collection.h>
#include <list>
#include <map>

namespace tpie {

namespace blocks {

/**
 * \brief A class to manage writing and reading of block to disk. 
 * Blocks are stored in an internal cache with a static size.
 */
class block_collection_cache {
private:
	struct position_comparator {
		bool operator()(const block_handle & a, const block_handle & b) const {
			return a.position < b.position;
		}
	};

	typedef std::list<block_handle> block_list_t;

	struct block_information_t {
		block_information_t() {}

		block_information_t(block * pointer, block_list_t::iterator iterator, bool dirty)
			: pointer(pointer)
			, iterator(iterator)
			, dirty(dirty)
		{}

		block * pointer;
		block_list_t::iterator iterator;
		bool dirty;
	};

	typedef std::map<block_handle, block_information_t, position_comparator> block_map_t;
public:
	/**
	 * \brief Create a block collection
	 * \param fileName the file in which blocks are saved
	 * \param blockSize the size of blocks constructed
	 * \param writeable indicates whether the collection is writeable
	 * \param maxSize the size of the cache given in number of blocks
	 */
	block_collection_cache(std::string fileName, memory_size_type blockSize, memory_size_type maxSize, bool writeable);

	~block_collection_cache();


	/**
	 * \brief Allocates a new block
	 * \return the handle of the new block
	 */
	block_handle get_free_block();

	/**
	 * \brief frees a block
	 * \param handle the handle of the block to be freed
	 */
	void free_block(block_handle handle);

private:
	// make space for a new block in the cache
	void prepare_cache();

	void add_to_cache(block_handle handle, block * b, bool dirty);
public:
	/**
	 * \brief Reads the content of a block from disk
	 * \param handle the handle of the block to read
	 * \return a pointer to the block with the given handle
	 */
	block * read_block(block_handle handle);

	/**
	 * \brief Writes the content of a block to disk
	 * \param handle the handle of the block to write
	 * \pre the block has been previously read and not flushed from the cache
	 */
	void write_block(block_handle handle);

private:
	block_collection m_collection;
	block_list_t m_blockList;
	block_map_t m_blockMap;
	memory_size_type m_curSize;
	memory_size_type m_maxSize;
	memory_size_type m_blockSize;
};

} // blocks namespace

}  //  tpie namespace

#endif // _TPIE_BLOCKS_BLOCK_COLLECTION_CACHE_H
