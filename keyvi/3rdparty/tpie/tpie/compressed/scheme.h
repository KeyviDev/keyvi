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

#ifndef TPIE_COMPRESSED_SCHEME_H
#define TPIE_COMPRESSED_SCHEME_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/scheme.h  Compression scheme virtual interface.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Possible values for the \c compressionFlags parameter to
/// \c stream::open.
///////////////////////////////////////////////////////////////////////////////
enum compression_flags {
	/** No written blocks should be compressed.
	 * If a new stream is opened with compression_none,
	 * it will support seek(n) and truncate(n) for arbitrary n. */
	compression_none = 0,
	/** Compress some blocks
	 * according to available resources (time, memory). */
	compression_normal = 1,
	/** Compress all blocks according to the preferred compression scheme
	 * which can be set using
	 * tpie::the_compressor_thread().set_preferred_compression(). */
	compression_all = 2
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Abstract virtual base class for each compression scheme.
///////////////////////////////////////////////////////////////////////////////
class compression_scheme {
public:
	enum type {
		none = 0,
		snappy = 1
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief  An upper bound on the size of a compressed block corresponding
	/// to an uncompressed input of size \c srcSize.
	///
	/// By the pigeonhole principle, the upper bound for a given \c srcSize
	/// must be greater than or equal to \c srcSize.
	///////////////////////////////////////////////////////////////////////////
	virtual size_t max_compressed_length(size_t srcSize) const = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compress data from \c src into \c dest, returning its size in
	/// \c destSize.
	///////////////////////////////////////////////////////////////////////////
	virtual void compress(char * dest, const char * src, size_t srcSize, size_t * destSize) const = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get the uncompressed size of the compressed block at \c src.
	///////////////////////////////////////////////////////////////////////////
	virtual size_t uncompressed_length(const char * src, size_t srcSize) const = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Uncompress a compressed block at \c src into \c dest.
	///////////////////////////////////////////////////////////////////////////
	virtual void uncompress(char * dest, const char * src, size_t srcSize) const = 0;

protected:
	~compression_scheme() {}
};

const compression_scheme & get_compression_scheme_none();
const compression_scheme & get_compression_scheme_snappy();

inline const compression_scheme & get_compression_scheme(compression_scheme::type t) {
	switch (t) {
		case compression_scheme::none:
			return get_compression_scheme_none();
		case compression_scheme::snappy:
			return get_compression_scheme_snappy();
	}
	return get_compression_scheme_none();
}

}

#endif // TPIE_COMPRESSED_SCHEME_H
