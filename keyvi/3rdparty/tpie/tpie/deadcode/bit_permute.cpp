// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#include <iostream>

#include <bit_permute.h>

using tpie::ami;

bit_perm_object::bit_perm_object(const bit_matrix &A,
				 const bit_matrix &c) :
    mA(A), mc(c) {
    //  No code in this constructor.
}

bit_perm_object::~bit_perm_object(void) {

    //  No code in this destructor.

}

bit_matrix bit_perm_object::A(void) {
    return mA;
}

bit_matrix bit_perm_object::c(void) {
    return mc;
}


