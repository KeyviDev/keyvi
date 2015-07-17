// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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
#ifndef __TPIE_PIPELINING_MAINTAIN_ORDER_TYPE_H__
#define __TPIE_PIPELINING_MAINTAIN_ORDER_TYPE_H__

///////////////////////////////////////////////////////////////////////////////
/// \file maintain_order_type.h  Whether to maintain order in parallel or not.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

namespace pipelining {

/** Type describing whether to maintain the order of items in parallel. */
enum maintain_order_type {
	/** Do not maintain order; push items as soon as a worker has processed
	 * them. */
	arbitrary_order = false,
	/** Maintain order; push items in the same order that a single thread would
	 * have. */
	maintain_order = true
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_MAINTAIN_ORDER_TYPE_H__
