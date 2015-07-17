// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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

#ifndef __TPIE_PIPELINING_H__
#define __TPIE_PIPELINING_H__

/**
 * \namespace tpie::pipelining TPIE pipelining framework.
 * \author Mathias Rav
 */

// Core framework
#include <tpie/pipelining/exception.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipeline.h>
#include <tpie/pipelining/pair_factory.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/virtual.h>

// Library
#include <tpie/pipelining/buffer.h>
#include <tpie/pipelining/internal_buffer.h>
#include <tpie/pipelining/file_stream.h>
#include <tpie/pipelining/helpers.h>
#include <tpie/pipelining/join.h>
#include <tpie/pipelining/merge.h>
#include <tpie/pipelining/node_map_dump.h>
#include <tpie/pipelining/numeric.h>
#include <tpie/pipelining/reverse.h>
#include <tpie/pipelining/serialization.h>
#include <tpie/pipelining/sort.h>
#include <tpie/pipelining/serialization_sort.h>
#include <tpie/pipelining/std_glue.h>
#include <tpie/pipelining/stdio.h>
#include <tpie/pipelining/uniq.h>
#include <tpie/pipelining/parallel.h>
#include <tpie/pipelining/map.h>

#endif
