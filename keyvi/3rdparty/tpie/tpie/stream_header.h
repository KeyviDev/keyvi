// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2011, 2012, The TPIE development team
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
/// \file stream_header.h  Header of streams.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_STREAM_HEADER_H__
#define __TPIE_STREAM_HEADER_H__
#include <tpie/util.h>
#include <tpie/types.h>

namespace tpie {

struct stream_header_t {
	static const uint64_t magicConst = 0x521cbe927dd6056all;
	static const uint64_t versionConst = 4;

	uint64_t magic;
	uint64_t version;
	uint64_t itemSize;
	uint64_t blockSize;
	uint64_t userDataSize;
	uint64_t maxUserDataSize;
	uint64_t size;
	uint64_t flags;
	uint64_t lastBlockReadOffset;

	static const uint64_t cleanCloseMask = 0x1;
	static const uint64_t compressedMask = 0x2;

	bool get_clean_close() const { return flags & cleanCloseMask; }
	void set_clean_close(bool b) { if (b) flags |= cleanCloseMask; else flags &= ~cleanCloseMask; }

	bool get_compressed() const { return flags & compressedMask; }
	void set_compressed(bool b) { if (b) flags |= compressedMask; else flags &= ~compressedMask; }
};

}
#endif //__TPIE_STREAM_HEADER_H__
