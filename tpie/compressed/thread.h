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

#ifndef TPIE_COMPRESSED_THREAD_H
#define TPIE_COMPRESSED_THREAD_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/thread.h  Interface to the compressor thread.
///////////////////////////////////////////////////////////////////////////////

#include <boost/thread.hpp>
#include <tpie/array.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/file_accessor/byte_stream_accessor.h>
#include <tpie/compressed/predeclare.h>
#include <tpie/compressed/scheme.h>

namespace tpie {

class compressor_thread {
	class impl;
	impl * pimpl;

public:
	typedef boost::shared_ptr<compressor_buffer> buffer_t;
	typedef file_accessor::byte_stream_accessor<default_raw_file_accessor> file_accessor_t;
	typedef boost::mutex mutex_t;

	static stream_size_type subtract_block_header(stream_size_type dataOffset);

	compressor_thread();
	~compressor_thread();

	mutex_t & mutex();

	// Locking: Caller must lock the thread (with a compressor_thread_lock).
	void request(compressor_request & r);

	void wait_for_request_done(compressor_thread_lock & l);

	void run();

	void stop(compressor_thread_lock & lock);

	void set_preferred_compression(compressor_thread_lock &, compression_scheme::type);
};

class compressor_thread_lock {
public:
	typedef boost::unique_lock<compressor_thread::mutex_t> lock_t;

	compressor_thread_lock(compressor_thread & c)
		: t1(ptime::now())
		, m_lock(c.mutex())
		, t2(ptime::now())
	{
	}

	~compressor_thread_lock() {
		ptime t3 = ptime::now();
		// Time blocked
		increment_user(0, ptime::seconds(t1, t2)*1000000);
		// Time held
		increment_user(1, ptime::seconds(t2, t3)*1000000);
	}

	lock_t & get_lock() {
		return m_lock;
	}

private:
	ptime t1;
	lock_t m_lock;
	ptime t2;
};

} // namespace tpie

#endif // TPIE_COMPRESSED_THREAD_H
