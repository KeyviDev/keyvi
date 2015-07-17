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

#ifndef _TPIE_BIT_MATRIX_H
#define _TPIE_BIT_MATRIX_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <bit.h>
#include <matrix.h>

#include <sys/types.h>


namespace tpie {

    class bit_matrix : public matrix<bit> {

    public:
	using matrix<bit>::rows;
	using matrix<bit>::cols;
	
	bit_matrix(matrix<bit> &mb);
	bit_matrix(TPIE_OS_SIZE_T rows, TPIE_OS_SIZE_T cols);
	virtual ~bit_matrix(void);
	
	bit_matrix operator=(const bit_matrix &rhs);
	
	// We can assign from an offset, which is typically a source
	// address for a BMMC permutation.
	bit_matrix &operator=(const TPIE_OS_OFFSET &rhs);
	
	operator TPIE_OS_OFFSET(void);
	
	friend bit_matrix operator+(const bit_matrix &op1, const bit_matrix &op2);
	friend bit_matrix operator*(const bit_matrix &op1, const bit_matrix &op2);
    };
    
    bit_matrix operator+(const bit_matrix &op1, const bit_matrix &op2);
    bit_matrix operator*(const bit_matrix &op1, const bit_matrix &op2);
    
    std::ostream &operator<<(std::ostream &s, bit_matrix &bm);

}  //  tpie namespace

#endif // _TPIE_BIT_MATRIX_H 
