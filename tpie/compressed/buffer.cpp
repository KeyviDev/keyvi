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

#include <tpie/compressed/stream.h>
#include <tpie/compressed/buffer.h>

namespace tpie {

class stream_buffer_pool::impl {
public:
	typedef std::shared_ptr<compressor_buffer> buffer_t;

	const static memory_size_type EXTRA_BUFFERS = 2;

	impl() {
		m_extraBuffers.reserve(EXTRA_BUFFERS);
		for (size_t i = 0; i < EXTRA_BUFFERS; ++i) {
			m_extraBuffers.push_back(std::make_shared<compressor_buffer>(block_size()));
		}
	}

	buffer_t allocate_own_buffer() {
		return std::make_shared<compressor_buffer>(block_size());
	}

	void release_own_buffer(buffer_t & b) {
		tp_assert(b.unique(), "release_own_buffer: !b.unique");

		// swap it into oblivion
		buffer_t().swap(b);
	}

	bool can_take_shared_buffer() {
		return !m_extraBuffers.empty();
	}

	buffer_t take_shared_buffer() {
		tp_assert(!m_extraBuffers.empty(), "take_shared_buffer: No available shared buffers");

		buffer_t b;
		b.swap(m_extraBuffers.back());
		m_extraBuffers.pop_back();
		return b;
	}

	void release_shared_buffer(buffer_t & b) {
		tp_assert(b.unique(), "release_shared_buffer: !b.unique");
		tp_assert(!(m_extraBuffers.size() == EXTRA_BUFFERS), "release_shared_buffer: Too many available shared buffers");

		m_extraBuffers.push_back(buffer_t());
		m_extraBuffers.back().swap(b);
	}

private:
	memory_size_type block_size() {
		return compressed_stream_base::block_size(1.0);
	}

	std::vector<buffer_t> m_extraBuffers;
};

stream_buffer_pool::stream_buffer_pool()
	: pimpl(new impl())
{
}

stream_buffer_pool::~stream_buffer_pool() {
	delete pimpl;
}

stream_buffer_pool::buffer_t stream_buffer_pool::allocate_own_buffer() {
	return pimpl->allocate_own_buffer();
}

void stream_buffer_pool::release_own_buffer(buffer_t & b) {
	pimpl->release_own_buffer(b);
}

bool stream_buffer_pool::can_take_shared_buffer() {
	return pimpl->can_take_shared_buffer();
}

stream_buffer_pool::buffer_t stream_buffer_pool::take_shared_buffer() {
	return pimpl->take_shared_buffer();
}

void stream_buffer_pool::release_shared_buffer(buffer_t & b) {
	pimpl->release_shared_buffer(b);
}

} // namespace tpie

namespace {

tpie::stream_buffer_pool * the_stream_buffer_pool;

} // unnamed namespace

namespace tpie {

stream_buffer_pool & the_stream_buffer_pool() {
	tp_assert(::the_stream_buffer_pool, "the_stream_buffer_pool: Not initialized!");
	return *::the_stream_buffer_pool;
}

void init_stream_buffer_pool() {
	::the_stream_buffer_pool = new tpie::stream_buffer_pool();
}

void finish_stream_buffer_pool() {
	delete ::the_stream_buffer_pool;
	::the_stream_buffer_pool = 0;
}

} // namespace tpie
