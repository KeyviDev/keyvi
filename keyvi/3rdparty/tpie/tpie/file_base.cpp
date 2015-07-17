// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2010, 2012, The TPIE development team
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
#include <tpie/portability.h>
#include <tpie/exception.h>
#include <tpie/file_base.h>
#include <tpie/file_stream_base.h>
#include <tpie/memory.h>
#include <stdlib.h>
#include <tpie/file_base_crtp.inl>
#include <tpie/stream_crtp.inl>

namespace tpie {

file_base::file_base(memory_size_type itemSize,
					 double blockFactor,
					 file_accessor::file_accessor * fileAccessor):
	file_base_crtp<file_base>(itemSize, blockFactor, fileAccessor) {
	m_emptyBlock.size = 0;
	m_emptyBlock.number = std::numeric_limits<stream_size_type>::max();
}


void file_base::create_block() {
	// alloc heap block
	block_t * block = reinterpret_cast<block_t*>( tpie_new_array<char>(sizeof(block_t) + m_itemSize*m_blockItems) );

	// call ctor
	new (block) block_t();

	// push to intrusive list
	m_free.push_front(*block);
}

void file_base::delete_block() {
	// find first block
	assert(!m_free.empty());
	block_t * block = &m_free.front();

	// remove from intrusive list
	m_free.pop_front();

	// call dtor
	block->~block_t();

	// dealloc
	tpie_delete_array<char>(reinterpret_cast<char*>(block), sizeof(block_t) + m_itemSize*m_blockItems);
}


file_base::block_t * file_base::get_block(stream_size_type block) {
	block_t * b;
	get_block_check(block);

	// First, see if the block is already buffered
	boost::intrusive::list<block_t>::iterator i = m_used.begin();
	while (i != m_used.end() && i->number != block)
		++i;

	if (i == m_used.end()) {
		// block not buffered. populate a free buffer.
		assert(!m_free.empty());

		// fetch a free buffer
		b = &m_free.front();
		b->usage = 0;
		read_block(*b, block);

		b->dirty = false;
		b->number = block;

		// read went well. move buffer to m_used
		m_free.pop_front();
		m_used.push_front(*b);

	} else {
		// yes, the block is already buffered.
		b = &*i;
	}
	++b->usage;
	return b;
}

void file_base::free_block(block_t * block) {
	assert(block->usage > 0);
	--block->usage;
	if (block->usage > 0) return;

	if (block->dirty || !m_canRead) {
		assert(m_canWrite);
		m_fileAccessor->write_block(block->data, block->number, block->size);
	}

	boost::intrusive::list<block_t>::iterator i = m_used.iterator_to(*block);

	m_used.erase(i);

	m_free.push_front(*block);
}

void file_base::close() {
	assert(m_free.empty());
	assert(m_used.empty());
	p_t::close();
}

file_base::~file_base() {
	assert(m_free.empty());
	assert(m_used.empty());
	delete m_fileAccessor;
}

/*************************> file_base::stream <*******************************/
void file_base::stream::update_block_core() {
	if (m_block != &m_file->m_emptyBlock) {
		m_file->free_block(m_block);
		m_block = &m_file->m_emptyBlock; // necessary if get_block below throws
	}
	m_block = m_file->get_block(m_nextBlock);
}


file_base::stream::stream(file_base & f, stream_size_type offset):
	m_file(0) {
	attach_inner(f);
	if (m_file->m_open)
		seek(offset);
}

void file_base::stream::free() {
	if (m_block) {
		if (m_block != &m_file->m_emptyBlock) m_file->free_block(m_block);
		m_file->delete_block();
	}
	m_block = 0;
}

void file_base::stream::attach_inner(file_base & f) {
	detach_inner();
	m_file = &f;
	m_blockStartIndex = 0;
	m_block = &m_file->m_emptyBlock;
	m_file->create_block();
	initialize();
	seek(0);
}

void file_base::stream::detach_inner() {
	if (!attached()) return;
	free();
	m_file = 0;
}

file_base::block_t file_base::m_emptyBlock;

template class stream_crtp<file_base::stream>;
template class file_base_crtp<file_base>;

} //namespace tpie
