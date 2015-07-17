// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////
/// \file tpie/util.h Miscellaneous utility functions.
///////////////////////////////////////////////////////////////////////////


#include <boost/random.hpp>
#include <tpie/util.h>
#include <boost/filesystem.hpp>
#include <stdexcept>

namespace tpie {
  
static boost::mt19937 rng;

///////////////////////////////////////////////////////////////////////////
/// \brief Seed the tpie pseudo random number generator
/// \param seed The seed used to generate pseudo random numbers
///////////////////////////////////////////////////////////////////////////
void seed_random(uint32_t seed) {
    rng.seed(seed);
}

///////////////////////////////////////////////////////////////////////////
/// \brief Generate a new 32-bit pseudo random number
/// \returns The random number
///////////////////////////////////////////////////////////////////////////
uint32_t random() {
    return rng();
}

///////////////////////////////////////////////////////////////////////////
/// \brief Remove a file from the filesystem, if the file does not exist
/// the function returns witout doing anything.
/// \param path The path of the file to remove
///////////////////////////////////////////////////////////////////////////
void remove(const std::string & path) {
	if (!boost::filesystem::exists(path))
		return;

	if (!boost::filesystem::remove(path))
		throw std::runtime_error(std::string("Unable to delete ")+path);
}

///////////////////////////////////////////////////////////////////////////
/// \brief Checks if a file exists on the filesystem
/// \param path The file whoes existense is questionable
/// \returns true iff the file exists
///////////////////////////////////////////////////////////////////////////
bool file_exists(const std::string & path) {
	return boost::filesystem::exists(path);
}

}
