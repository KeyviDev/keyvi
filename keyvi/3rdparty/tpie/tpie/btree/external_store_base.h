// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The TPIE development team
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

#ifndef _TPIE_BTREE_EXTERNAL_STORE_BASE_H_
#define _TPIE_BTREE_EXTERNAL_STORE_BASE_H_

#include <tpie/portability.h>
#include <tpie/btree/base.h>
#include <tpie/tpie_assert.h>
#include <tpie/blocks/block_collection_cache.h>

#include <cstddef>

namespace tpie {
namespace bbits {

class external_store_base {
public:

	/**
	 * \brief Construct a new empty btree storage
	 */
	external_store_base(const std::string & path);

	~external_store_base();

protected:
	blocks::block_handle m_root;
	std::string m_path;
	size_t m_height;
	size_t m_size;
};

} //namespace bbits
} //namespace tpie
#endif /*_TPIE_BTREE_EXTERNAL_STORE_BASE_H_*/
