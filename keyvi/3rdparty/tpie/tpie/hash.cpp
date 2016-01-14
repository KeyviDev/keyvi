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

#include <random>
#include <tpie/tpie.h>
#include <tpie/hash.h>

using namespace tpie::hash_bits;

namespace tpie {

namespace hash_bits {

size_t hash_codes[sizeof(size_t)][256];

} // namespace hash_bits

void init_hash() {
	std::mt19937 rng(9001);
	std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

	for(size_t i = 0; i < sizeof(size_t); ++i)
		for(size_t j = 0; j < 256; ++j)
			hash_codes[i][j] = dist(rng);
}

} // namespace tpie
