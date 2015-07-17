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

#include <bit.h>

using namespace tpie;

bit::bit(void) : data(0) {
    //  No code in this constructor.
}

bit::bit(bool b) : data(0) {
    data = (b == true);
}

bit::bit(int i) : data(0) {
    data = (i != 0);
}

bit::bit(long int i) : data(0) {
    data = (i != 0);
}

bit::operator bool(void) {
    return (data != 0);
}
        
bit::operator int(void) {
    return data;
}
        
bit::operator long int(void) {
    return data;
}
        
bit::~bit(void) {
    //  No code in this destructor.
}

bit bit::operator+=(bit rhs) {
    return *this = *this + rhs;
}
        
bit bit::operator*=(bit rhs) {
    return *this = *this + rhs;
}

bit operator+(bit op1, bit op2) {
    return bit(op1.data ^ op2.data);
}

bit operator*(bit op1, bit op2) {
    return bit(op1.data & op2.data);
}

std::ostream &operator<<(std::ostream &s, bit b) {
    return s << int(b.data);
}




