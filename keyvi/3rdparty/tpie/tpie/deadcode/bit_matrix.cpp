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

#include <bit_matrix.h>

using namespace tpie;

bit_matrix::bit_matrix(TPIE_OS_SIZE_T arows, TPIE_OS_SIZE_T acols) :
        matrix<bit>(arows, acols) {

    //  No code in this constructor.
}

bit_matrix::bit_matrix(matrix<bit> &mb) :
        matrix<bit>(mb) {
    //  No code in this constructor.
}
    
bit_matrix::~bit_matrix(void) {
    //  No code in this destructor.
}

bit_matrix bit_matrix::operator=(const bit_matrix &rhs) {
    return this->matrix<bit>::operator=((matrix<bit> &)rhs);
}

bit_matrix & bit_matrix::operator=(const TPIE_OS_OFFSET &rhs) {
    TPIE_OS_SIZE_T rows = this->rows();
    TPIE_OS_SIZE_T ii;

    if (this->cols() != 1) {
#if HANDLE_EXCEPTIONS
        throw matrix_base<bit>::range();
#else
        tp_assert(0, "Range error.");
#endif
    }
    
    for (ii = 0; ii < rows; ii++) {
        (*this)[ii][0] = (long int)(rhs & (1 << ii)) >> ii;
    }
    
    return *this;
}    

bit_matrix::operator TPIE_OS_OFFSET(void) {
    TPIE_OS_OFFSET res;

    TPIE_OS_SIZE_T rows = this->rows();
    TPIE_OS_SIZE_T ii;

    if (this->cols() != 1) {
#if HANDLE_EXCEPTIONS
        throw matrix_base<bit>::range();
#else
        tp_assert(0, "Range error.");
#endif
    }

    for (res = 0, ii = 0; ii < rows; ii++) {
        res |= (long int)((*this)[ii][0]) << ii;
    }
    
    return res;
}


bit_matrix operator+(const bit_matrix &op1, const bit_matrix &op2) {
    matrix<bit> sum = ((matrix<bit> &)op1) + ((matrix<bit> &)op2);

    return sum;
}

bit_matrix operator*(const bit_matrix &op1, const bit_matrix &op2) {
    matrix<bit> prod = ((matrix<bit> &)op1) * ((matrix<bit> &)op2);

    return prod;
}

std::ostream &operator<<(std::ostream &s, bit_matrix &bm) {
    return s << (matrix<bit> &)bm;
}
