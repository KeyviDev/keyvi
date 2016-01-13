// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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
#ifndef __TPIE_PRIME_H__
#define __TPIE_PRIME_H__
#include <tpie/array.h>
#include <tpie/util.h>

///////////////////////////////////////////////////////////////////////////////
/// \file prime.h
/// \brief Computations related to prime numbers.
///////////////////////////////////////////////////////////////////////////////
namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the prime number
/// database.
///////////////////////////////////////////////////////////////////////////////
void init_prime();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the prime number
/// database.
///////////////////////////////////////////////////////////////////////////////
void finish_prime();

///////////////////////////////////////////////////////////////////////////////
/// \brief Check if i is a prime.
///
/// \param i number to check, must be less then 4294967295.
/// \return true if and only if i is a prime number.
///////////////////////////////////////////////////////////////////////////////
bool is_prime(size_type i);

typedef uint64_t hash_type;

///////////////////////////////////////////////////////////////////////////////
/// \brief Calculate a fairly good string hash based on prime numbers.
///
/// \param s The string to hash.
/// \return The hash value.
///////////////////////////////////////////////////////////////////////////////
hash_type prime_hash(const std::string & s);

///////////////////////////////////////////////////////////////////////////////
/// \brief Get next prime.
/// \param i Subject to same restrictions as in is_prime.
/// \returns Least prime greater than or equal to i.
///////////////////////////////////////////////////////////////////////////////
size_t next_prime(size_t i);
}

#endif //__TPIE_PRIME_H__
