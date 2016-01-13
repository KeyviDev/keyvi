// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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

///////////////////////////////////////////////////////////////////////////////
/// \file block_collection Class to handle blocks of varying size on disk
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_BLOCKS_BLOCK_COLLECTION_H
#define _TPIE_BLOCKS_BLOCK_COLLECTION_H

#include <tpie/tpie.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/blocks/block.h>
#include <tpie/blocks/freespace_collection.h>

namespace tpie {

namespace blocks {

/**
 * \brief A class to manage writing and reading of block to disk.
 */
class block_collection {
public:
	/**
	 * \brief Create a block collection
	 * \param fileName the file in which blocks are saved
	 * \param blockSize the size of the blocks
	 * \param writeable indicates whether the collection is writeable
	 */
	block_collection(std::string fileName, memory_size_type blockSize, bool writeable);

	~block_collection();

	block_collection(const block_collection &) = delete;

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

	/**
	 * \brief Reads the content of a block from disk
	 * \param handle the handle of the block to read
	 * \param b the block to store the content in
	 */
	void read_block(block_handle handle, block & b);

	/**
	 * \brief Writes the content of a block to disk
	 * \param handle the handle of the block to write
	 * \param b the block type in which the content is stored
	 */
	void write_block(block_handle handle, const block & b);
private:
	bits::freespace_collection m_collection;
	tpie::file_accessor::raw_file_accessor m_accessor;

	bool m_writeable;
};

} // blocks namespace

}  //  tpie namespace

#endif // _TPIE_BLOCKS_BLOCK_COLLECTION_H
