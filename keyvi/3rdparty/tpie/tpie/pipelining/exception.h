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

#ifndef __TPIE_PIPELINING_EXCEPTION_H__
#define __TPIE_PIPELINING_EXCEPTION_H__

#include <tpie/exception.h>

namespace tpie {

class merge_sort_not_ready : public exception {
public:
	inline merge_sort_not_ready() : tpie::exception("Merge sort did not have memory assigned") {}
};

namespace pipelining {

class not_initiator_node : public tpie::exception {
public:
	inline not_initiator_node() : tpie::exception("Not an initiator node") {}
};

class no_initiator_node : public tpie::exception {
public:
	no_initiator_node() : tpie::exception("Phase has no initiator node") {}
};

class virtual_chunk_not_ready : public tpie::exception {
public:
	inline virtual_chunk_not_ready() : tpie::exception("Virtual receiver is missing a destination") {}
};

class virtual_chunk_missing_begin : public tpie::exception {
public:
	inline virtual_chunk_missing_begin() : tpie::exception("Virtual begin chunk contains no pipes") {}
};

class virtual_chunk_missing_middle : public tpie::exception {
public:
	inline virtual_chunk_missing_middle() : tpie::exception("Virtual middle chunk contains no pipes, and input type is not output type") {}
};

class virtual_chunk_missing_end : public tpie::exception {
public:
	inline virtual_chunk_missing_end() : tpie::exception("Virtual end chunk contains no pipes") {}
};

class non_authoritative_node_map : public tpie::exception {
public:
	non_authoritative_node_map() : tpie::exception("Non-authoritative node map") {}
};

class call_order_exception: public tpie::exception {
public:
	call_order_exception(std::string msg): tpie::exception(msg) {}
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_EXCEPTION_H__
