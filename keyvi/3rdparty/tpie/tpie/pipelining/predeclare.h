// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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
/// \file pipelining/predeclare.h  Predeclarations of some pipelining classes.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_PREDECLARE_H__
#define __TPIE_PIPELINING_PREDECLARE_H__

namespace tpie {

namespace pipelining {

class node;
class node_token;
class not_initiator_node;

namespace bits {
	class node_map;
	class runtime;
	class memory_runtime;
	class datastructure_runtime;
	class pipeline_base;
}

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PREDECLARE_H__
