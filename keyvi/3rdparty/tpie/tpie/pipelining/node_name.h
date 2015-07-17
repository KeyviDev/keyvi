// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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

#ifndef TPIE_PIPELINING_NODE_NAME_H
#define TPIE_PIPELINING_NODE_NAME_H

#include <string>

namespace tpie {

namespace pipelining {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief Extract the class name from a mangled typeid name.
///////////////////////////////////////////////////////////////////////////////
std::string extract_pipe_class_name(std::string mangled);

///////////////////////////////////////////////////////////////////////////////
/// \brief Extract a sensible default name for a node given its typeid name.
///////////////////////////////////////////////////////////////////////////////
std::string extract_pipe_name(std::string mangled);

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif // TPIE_PIPELINING_NODE_NAME_H
