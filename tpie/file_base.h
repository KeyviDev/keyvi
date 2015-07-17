// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2011, 2012 The TPIE development team
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
/// \file file_base.h  Basic file and stream operations.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_FILE_BASE_H__
#define __TPIE_FILE_BASE_H__
#include <tpie/file_base_crtp.h>
#include <tpie/stream_crtp.h>
#include <tpie/exception.h>
#include <tpie/file_accessor/file_accessor.h>
#ifndef WIN32
#include <tpie/file_accessor/posix.h>
#else ////WIN32
#include <tpie/file_accessor/win32.h>
#endif //WIN32
#include <boost/intrusive/list.hpp>
#include <tpie/tempname.h>
#include <memory>
#include <tpie/memory.h>
#include <tpie/cache_hint.h>
#include <tpie/access_type.h>
#include <tpie/types.h>

namespace tpie {

class file_base: public file_base_crtp<file_base> {
	typedef file_base_crtp<file_base> p_t;
protected:
	///////////////////////////////////////////////////////////////////////////
	/// This is the type of our block buffers. We have one per file::stream
	/// distributed over two linked lists.
	///////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4200 )
#endif
	struct block_t : public boost::intrusive::list_base_hook<> {
		memory_size_type size;
		memory_size_type usage;
		stream_size_type number;
		bool dirty;
		char data[0];
	};
#ifdef WIN32
#pragma warning( pop )
#endif

	inline void update_size(stream_size_type size) {
		m_size = std::max(m_size, size);
		if (m_tempFile) 
			m_tempFile->update_recorded_size(m_fileAccessor->byte_size());
	}

public:
	inline stream_size_type size() const throw() {
		return file_size();
	}

	void close();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Stream in file. We support multiple streams per file.
	///////////////////////////////////////////////////////////////////////////
	class stream: public stream_crtp<stream> {
	private:
		typedef stream_crtp<stream> p_t;

		friend class stream_crtp<stream>;

		/** Associated file object. */
		file_base * m_file;

	protected:
		block_t & get_block() {return *m_block;}
		const block_t & get_block() const {return *m_block;}
		inline file_base & get_file() {assert(m_file != 0); return *m_file;}
		inline const file_base & get_file() const {assert(m_file != 0); return *m_file;}

		void update_block_core();

		inline void update_vars() {}

		/** Current block. May be equal to &m_file->m_emptyBlock to indicate no
		 * current block. */
		block_t * m_block;

	public:
		///////////////////////////////////////////////////////////////////////
		/// \brief True if we are attached to a tpie::file.
		///////////////////////////////////////////////////////////////////////
		inline bool attached() const { return 0 != m_file; }

	protected:
		///////////////////////////////////////////////////////////////////////
		/// \brief Attach to the given tpie::file. If necessary, detach first.
		///////////////////////////////////////////////////////////////////////
		void attach_inner(file_base & f);

		///////////////////////////////////////////////////////////////////////
		/// \brief Detach from a tpie::file.
		///////////////////////////////////////////////////////////////////////
		void detach_inner();

	public:
		///////////////////////////////////////////////////////////////////////
		/// \brief Fetch number of items per block.
		///////////////////////////////////////////////////////////////////////
		inline memory_size_type block_items() const {return get_file().m_blockItems;}

	protected:
		///////////////////////////////////////////////////////////////////////
		/// Call whenever the current block buffer is modified. Since we
		/// support multiple streams per block, we must always keep
		/// m_block->size updated when m_block is the trailing block (or the
		/// only block) in the file. For the same reasons we keep m_file->m_size
		/// updated.
		///////////////////////////////////////////////////////////////////////
		inline void write_update() {
			m_block->dirty = true;
			m_block->size = std::max(m_block->size, m_index);
			get_file().update_size(static_cast<stream_size_type>(m_index)+m_blockStartIndex);
		}

	public:
		///////////////////////////////////////////////////////////////////////
		/// \brief Create a stream associated with the given file.
		/// \param file The file to associate with this stream.
		/// \param offset The file-level item offset to seek to.
		///////////////////////////////////////////////////////////////////////
		stream(file_base & file, stream_size_type offset=0);

		stream() : m_file(0) {}

	private:
		///////////////////////////////////////////////////////////////////////
		/// \brief Free the current block buffer, flushing it to the disk.
		///////////////////////////////////////////////////////////////////////
		void free();

	public:
		inline ~stream() {free();}


	protected:
		///////////////////////////////////////////////////////////////////////
		/// \brief Set up block buffers and offsets.
		///////////////////////////////////////////////////////////////////////
		inline void initialize() {
			if (m_block != &get_file().m_emptyBlock) get_file().free_block(m_block);
			p_t::initialize();
			m_block = &get_file().m_emptyBlock;
		}
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Truncate file to given size. May only be used when no streams
	/// are opened to this file.
	///////////////////////////////////////////////////////////////////////////
	void truncate(stream_size_type s) throw(stream_exception) {
		assert(m_open);
		if (!m_used.empty()) {
			throw io_exception("Tried to truncate a file with one or more open streams");
		}
		m_size = s;
		m_fileAccessor->truncate(s);
		if (m_tempFile)
			m_tempFile->update_recorded_size(m_fileAccessor->byte_size());
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief file_base destructor.
	///////////////////////////////////////////////////////////////////////////
	~file_base();


protected:
	file_base(memory_size_type item_size,
			  double blockFactor=1.0,
			  file_accessor::file_accessor * fileAccessor=NULL);

	void create_block();
	void delete_block();
	block_t * get_block(stream_size_type block);
	void free_block(block_t * block);


	static block_t m_emptyBlock;
	// TODO This should really be a hash map
	boost::intrusive::list<block_t> m_used;
	boost::intrusive::list<block_t> m_free;
};

} // namespace tpie

#endif //__TPIE_FILE_BASE_H__
