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
#include <tpie/prime.h>

///////////////////////////////////////////////////////////////////////////
/// \file prime.cpp
/// \brief Computations related to prime numbers.
///////////////////////////////////////////////////////////////////////////

namespace {
using namespace tpie;

struct is_prime_t {
private:
	const size_type m;
	const size_type mr;
	array<size_type> m_pr;
public:
	is_prime_t(): m(4294967295ul), mr(static_cast<size_t>(sqrt(double(m))+1)) {}
	
	inline void init() {
		array<bool> sieve(mr >> 1, true);
		size_t pc=1;
		for (size_t i=3; i < mr; i+= 2) {
			if (!sieve[i>>1]) continue;
			++pc;
			size_t i2=i+i;
			for (size_t j=i2+i; j < mr; j += i2) sieve[j>>1] = false;
		}
		m_pr.resize(pc);
		size_t p=1;
		m_pr[0] = 2;
		for (size_t i=3; i < mr; i += 2) 
			if (sieve[i>>1]) m_pr[p++] = i;
	}

	inline void free() {
		m_pr.resize(0);
	}

	inline bool operator()(size_type i) { 
		for(size_type j =0; m_pr[j] * m_pr[j] <= i; ++j) {
			if (i % m_pr[j] == 0) return false;
		}
		return true;
	}

	inline hash_type prime_hash(const std::string & x) {
		hash_type r=42;
		for(size_t i=0; i < x.size(); ++i)
			r = r*m_pr[i % m_pr.size()] + x[i];
		return r;
	}
};

is_prime_t is_prime_ins;
} //Nameless namespace

namespace tpie {

void init_prime() {is_prime_ins.init();}

void finish_prime() {is_prime_ins.free();}

bool is_prime(size_type i) {return is_prime_ins(i);}

hash_type prime_hash(const std::string & s) {return is_prime_ins.prime_hash(s);}

size_t next_prime(size_t i)  {
	while(!is_prime(i)) ++i;
	return i;
}


}
