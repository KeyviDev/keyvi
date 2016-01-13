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

#include <queue>
#include <tpie/compressed/thread.h>
#include <tpie/compressed/request.h>
#include <tpie/compressed/buffer.h>
#include <tpie/compressed/scheme.h>
#include <condition_variable>
namespace {

class block_header {
public:
	block_header()
		: m_payload(0)
	{
	}

	tpie::memory_size_type get_block_size() const {
		return static_cast<tpie::memory_size_type>(m_payload & BLOCK_SIZE_MASK);
	}

	// Precondition: 0 <= blockSize <= max_block_size()
	// Postcondition: get_block_size() == blockSize
	void set_block_size(tpie::memory_size_type blockSize) {
		m_payload &= ~BLOCK_SIZE_MASK;
		m_payload |= static_cast<tpie::uint32_t>(blockSize);
	}

	static tpie::memory_size_type max_block_size() {
		return BLOCK_SIZE_MAX;
	}

	tpie::compression_scheme::type get_compression_scheme() const {
		return static_cast<tpie::compression_scheme::type>((m_payload & COMPRESSION_MASK) >> BLOCK_SIZE_BITS);
	}

	void set_compression_scheme(tpie::compression_scheme::type scheme) {
		m_payload &= ~COMPRESSION_MASK;
		m_payload |= scheme << BLOCK_SIZE_BITS;
	}

	bool operator==(const block_header & other) const {
		return m_payload == other.m_payload;
	}

	bool operator!=(const block_header & other) const { return !(*this == other); }

private:
	static const tpie::uint32_t BLOCK_SIZE_BITS = 24;
	static const tpie::uint32_t BLOCK_SIZE_MASK = (1 << BLOCK_SIZE_BITS) - 1;
	static const tpie::memory_size_type BLOCK_SIZE_MAX =
		static_cast<tpie::memory_size_type>(1 << BLOCK_SIZE_BITS) - 1;
	static const tpie::uint32_t COMPRESSION_BITS = 8;
	static const tpie::uint32_t COMPRESSION_MASK = ((1 << COMPRESSION_BITS) - 1) << BLOCK_SIZE_BITS;

	tpie::uint32_t m_payload;
};

}

namespace tpie {

/*static*/ stream_size_type compressor_thread::subtract_block_header(stream_size_type dataOffset) {
	return dataOffset - sizeof(block_header);
}

class compressor_thread::impl {
public:
	impl()
		: m_done(false)
		, m_preferredCompression(compression_scheme::snappy)
	{
	}

	void stop(compressor_thread_lock & /*lock*/) {
		m_done = true;
		m_newRequest.notify_one();
	}

	bool request_valid(const compressor_request & r) {
		switch (r.kind()) {
			case compressor_request_kind::NONE:
				return false;
			case compressor_request_kind::READ:
			case compressor_request_kind::WRITE:
				return true;
		}
		tp_assert(false, "Unknown request type");
	}

	void run() {
		while (true) {
			compressor_thread_lock::lock_t lock(mutex());
			m_idle = false;
			while (!m_done && m_requests.empty()) {
				m_idle = true;
				m_newRequest.wait(lock);
			}
			if (m_done && m_requests.empty()) break;
			{
				compressor_request r = m_requests.front();
				m_requests.pop();
				lock.unlock();

				switch (r.kind()) {
					case compressor_request_kind::NONE:
						throw exception("Invalid request");
					case compressor_request_kind::READ:
						process_read_request(r.get_read_request());
						break;
					case compressor_request_kind::WRITE:
						process_write_request(r.get_write_request());
						break;
				}
			}
			lock.lock();
			m_requestDone.notify_all();
		}
	}

private:
	void checked_read(read_request & rr, stream_size_type readOffset, void * buf, memory_size_type count) {
		memory_size_type nRead = rr.file_accessor().read(readOffset, buf, count);
		if (nRead != count) {
			throw exception("read failed to read right amount");
		}
	}

	void process_read_request(read_request & rr) {
		stat_timer t(3); // Time reading
		const bool useCompression = rr.file_accessor().get_compressed();
		const bool backward = rr.get_read_direction() == read_direction::backward;
		tp_assert(!(backward && !useCompression), "backward && !useCompression");

		stream_size_type readOffset = rr.read_offset();
		if (!useCompression) {
			memory_size_type blockSize = rr.buffer()->size();
			if (blockSize > rr.buffer()->capacity()) {
				throw stream_exception("Internal error; blockSize > buffer capacity");
			}
			rr.file_accessor().read(readOffset, rr.buffer()->get(), blockSize);
			rr.buffer()->set_size(blockSize);
			compressor_thread_lock::lock_t lock(mutex());
			// Notify that reading has completed.
			rr.set_next_block_offset(1111111111111111111ull);
			rr.buffer()->transition_state(compressor_buffer_state::reading,
										  compressor_buffer_state::clean);
			return;
		}
		block_header blockHeader;
		block_header blockTrailer;
		memory_size_type blockSize;
		array<char> scratch;
		char * compressed;
		stream_size_type nextReadOffset;
		if (backward) {
			readOffset -= sizeof(blockTrailer);
			checked_read(rr, readOffset, &blockTrailer, sizeof(blockTrailer));
			blockSize = blockTrailer.get_block_size();
			if (blockSize == 0) {
				throw exception("Block size was unexpectedly zero");
			}
			scratch.resize(sizeof(blockHeader) + blockSize);
			readOffset -= scratch.size();
			checked_read(rr, readOffset, scratch.get(), scratch.size());
			compressed = scratch.get() + sizeof(blockHeader);
			memcpy(&blockHeader,
				   reinterpret_cast<block_header *>(scratch.get()),
				   sizeof(blockHeader));
			nextReadOffset = readOffset;
		} else {
			checked_read(rr, readOffset, &blockHeader, sizeof(blockHeader));
			blockSize = blockHeader.get_block_size();
			if (blockSize == 0) {
				throw exception("Block size was unexpectedly zero");
			}
			scratch.resize(blockSize + sizeof(blockTrailer));
			checked_read(rr, readOffset + sizeof(blockHeader), scratch.get(), scratch.size());
			compressed = scratch.get();
			memcpy(&blockTrailer,
				   reinterpret_cast<block_header *>(scratch.get() + scratch.size()) - 1,
				   sizeof(blockTrailer));
			nextReadOffset = readOffset + sizeof(blockHeader) + scratch.size();
		}
		if (blockHeader != blockTrailer) {
			throw exception("Block trailer is different from the block header");
		}

		const compression_scheme & compressionScheme =
			get_compression_scheme(blockHeader.get_compression_scheme());
		size_t uncompressedLength = compressionScheme.uncompressed_length(compressed, blockSize);
		if (uncompressedLength > rr.buffer()->capacity())
			throw exception("uncompressedLength exceeds the buffer capacity");
		compressionScheme.uncompress(rr.buffer()->get(), compressed, blockSize);

		compressor_thread_lock::lock_t lock(mutex());
		rr.buffer()->transition_state(compressor_buffer_state::reading,
									  compressor_buffer_state::clean);
		rr.buffer()->set_size(uncompressedLength);
		rr.buffer()->set_block_size(sizeof(blockHeader) + blockSize + sizeof(blockTrailer));
		rr.buffer()->set_read_offset(readOffset);
		rr.set_next_block_offset(nextReadOffset);
	}

	void process_write_request(write_request & wr) {
		stat_timer t(4); // Time writing
		size_t inputLength = wr.buffer()->size();
		if (!wr.file_accessor().get_compressed()) {
			// Uncompressed case
			wr.file_accessor().write(wr.write_offset(), wr.buffer()->get(), wr.buffer()->size());
			compressor_thread_lock::lock_t lock(mutex());
			wr.buffer()->transition_state(compressor_buffer_state::writing,
										  compressor_buffer_state::clean);
			wr.update_recorded_size();
			return;
		}
		// Compressed case
		const bool adaptiveCompression =
			wr.file_accessor().get_compression_flags() != compression_all;
		block_header blockHeader;
		block_header & blockTrailer = blockHeader;
		compression_scheme::type schemeType = m_preferredCompression;
		if (adaptiveCompression && !m_idle) {
			schemeType = compression_scheme::none;
		}
		if (schemeType == compression_scheme::snappy)
			increment_user(7, 1);
		if (schemeType == compression_scheme::none)
			increment_user(8, 1);
		const compression_scheme & compressionScheme = get_compression_scheme(schemeType);
		const memory_size_type maxBlockSize = compressionScheme.max_compressed_length(inputLength);
		if (maxBlockSize > blockHeader.max_block_size())
			throw exception("process_write_request: MaxCompressedLength > max_block_size");
		array<char> scratch(sizeof(blockHeader) + maxBlockSize + sizeof(blockTrailer));
		memory_size_type blockSize;
		compressionScheme.compress(scratch.get() + sizeof(blockHeader),
								   reinterpret_cast<const char *>(wr.buffer()->get()),
								   inputLength,
								   &blockSize);
		blockHeader.set_block_size(blockSize);
		blockHeader.set_compression_scheme(schemeType);
		memcpy(scratch.get(), &blockHeader, sizeof(blockHeader));
		memcpy(scratch.get() + sizeof(blockHeader) + blockSize, &blockTrailer, sizeof(blockTrailer));
		const memory_size_type writeSize = sizeof(blockHeader) + blockSize + sizeof(blockTrailer);
		if (!wr.should_append()) {
			//log_debug() << "Truncate to " << wr.write_offset() << std::endl;
			wr.file_accessor().truncate_bytes(wr.write_offset());
			//log_debug() << "File size is now " << wr.file_accessor().file_size() << std::endl;
		}
		{
			compressor_thread_lock::lock_t lock(mutex());
			wr.buffer()->transition_state(compressor_buffer_state::writing,
										  compressor_buffer_state::clean);
			wr.buffer()->set_block_size(writeSize);
			wr.buffer()->set_read_offset(wr.file_accessor().file_size());
			const stream_size_type offset = wr.file_accessor().file_size();
			wr.set_block_info(offset, writeSize);
			const stream_size_type newSize = offset + writeSize;
			wr.update_recorded_size(newSize);
		}
		wr.file_accessor().append(scratch.get(), writeSize);
	}

public:
	mutex_t & mutex() {
		return m_mutex;
	}

	void request(const compressor_request & r) {
		tp_assert(request_valid(r), "Invalid request");

		m_requests.push(r);
		m_requests.back().get_request_base().initiate_request();
		m_newRequest.notify_one();
	}

	void wait_for_request_done(compressor_thread_lock & l) {
		// Time waiting
		stat_timer t(2);
		m_requestDone.wait(l.get_lock());
	}

	void set_preferred_compression(compressor_thread_lock &, compression_scheme::type scheme) {
		m_preferredCompression = scheme;
	}

private:
	mutex_t m_mutex;
	std::queue<compressor_request> m_requests;
	std::condition_variable m_newRequest;
	std::condition_variable m_requestDone;
	bool m_done;
	compression_scheme::type m_preferredCompression;

	// Whether the thread was idle prior to handling the current request.
	bool m_idle;
};

} // namespace tpie

namespace {

tpie::compressor_thread the_compressor_thread;
std::thread the_compressor_thread_handle;
bool compressor_thread_already_finished = false;

void run_the_compressor_thread() {
	the_compressor_thread.run();
}

} // unnamed namespace

namespace tpie {

compressor_thread & the_compressor_thread() {
	return ::the_compressor_thread;
}

void init_compressor() {
	if (the_compressor_thread_handle.get_id() != std::thread::id()) {
		log_debug() << "Attempted to initiate compressor thread twice" << std::endl;
		return;
	}
	std::thread t(run_the_compressor_thread);
	the_compressor_thread_handle.swap(t);
	compressor_thread_already_finished = false;
}

void finish_compressor() {
	if (the_compressor_thread_handle.get_id() == std::thread::id()) {
		if (compressor_thread_already_finished) {
			log_debug() << "Compressor thread already finished" << std::endl;
		} else {
			log_debug() << "Attempted to finish compressor thread that was never initiated" << std::endl;
		}
		return;
	}
	{
		compressor_thread_lock lock(the_compressor_thread());
		the_compressor_thread().stop(lock);
	}
	the_compressor_thread_handle.join();
	std::thread t;
	the_compressor_thread_handle.swap(t);
	compressor_thread_already_finished = true;
}

compressor_thread::compressor_thread()
	: pimpl(new impl)
{
}

compressor_thread::~compressor_thread() {
	delete pimpl;
}

compressor_thread::mutex_t & compressor_thread::mutex() {
	return pimpl->mutex();
}

void compressor_thread::request(compressor_request & r) {
	pimpl->request(r);
}

void compressor_thread::run() {
	pimpl->run();
}

void compressor_thread::wait_for_request_done(compressor_thread_lock & l) {
	pimpl->wait_for_request_done(l);
}

void compressor_thread::stop(compressor_thread_lock & lock) {
	pimpl->stop(lock);
}

void compressor_thread::set_preferred_compression(compressor_thread_lock & lock, compression_scheme::type scheme) {
	pimpl->set_preferred_compression(lock, scheme);
}

}
