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

// For the moment this is done in terms of general permutations.
// This will obviously change in the future.

#ifndef _TPIE_AMI_BIT_PERMUTE_H
#define _TPIE_AMI_BIT_PERMUTE_H

// Get definitions for working with Unix and Windows
#include <portability.h>

// Get bit_matrix.
#include <bit_matrix.h>

// Get AMI_gen_perm_object.
#include <gen_perm_object.h>
// Get the AMI_general_permute().
#include <gen_perm.h>

namespace tpie {

    namespace ami {

	class bit_perm_object {

	private:
	    // The matrices that define the permutation.
	    bit_matrix mA;
	    bit_matrix mc;

	public:
	    bit_perm_object(const bit_matrix &A,
			    const bit_matrix &c);
	    ~bit_perm_object(void);
	    
	    bit_matrix A(void);
	    bit_matrix c(void);
	};

    }  //  ami namespace

}  //  tpie namespace 
    

namespace tpie {

    namespace ami {
	
	template<class T>
	class bmmc_as_gen_po : public gen_perm_object {

	private:
	    // Prohibit these
	    bmmc_as_gen_po(const bmmc_as_gen_po<T>& other);
	    bmmc_as_gen_po<T>& operator=(const bmmc_as_gen_po<T>& other);
	    
	    bit_matrix *src_bits;
	    bit_matrix A;
	    bit_matrix c;
	    
	public:
	    bmmc_as_gen_po(AMI_bit_perm_object &bpo) :
		A(bpo.A()), c(bpo.c()), src_bits(NULL)
		{
		    tp_assert(A.rows() == A.cols(), "A is not square.");
		    tp_assert(c.cols() == 1, "c is not a column vector.");
		    tp_assert(c.rows() == A.cols(), "A and c dimensions do not match.");
		    src_bits = tpie_new<bit_matrix>(c.rows(),1);
		};
	    
	    //  This destructor was not present before 2005116---why?
	    virtual ~bmmc_as_gen_po() {
			tpie_delete(src_bits);
	    }
	    
	    AMI_err initialize(TPIE_OS_OFFSET /*stream_len*/) {
		return ami::NO_ERROR;
	    }
	    
	    TPIE_OS_OFFSET destination(TPIE_OS_OFFSET input_offset) {
		
		*src_bits = input_offset;
		
		bit_matrix r1 = A * *src_bits;
		bit_matrix res = r1 + c;
		
		return TPIE_OS_OFFSET(res);
	    }
	};

    }  //  ami namespace

}  // tpie namespace

#ifndef TPIE_LIBRARY

template<class T>
AMI_err AMI_BMMC_permute(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                         tpie::ami::bit_perm_object *bpo)
{
    TPIE_OS_OFFSET sz_len = instream->stream_len();

    TPIE_OS_OFFSET sz_pow2;
    unsigned int bits;
    
    // Make sure the length of the input stream is a power of two.

    for (sz_pow2 = 1, bits = 0; sz_pow2 < sz_len; sz_pow2 += sz_pow2) {
        bits++;
    }

    if (sz_pow2 != sz_len) {
        return ami::NOT_POWER_OF_2;
    }
    
    // Make sure the number of bits in the permutation matrix matches
    // the log of the number of items in the input stream.

    {
	tpie::ami::bit_matrix A = bpo->A();
	tpie::ami::bit_matrix c = bpo->c();
        
        if (A.rows() != bits) {
            return ami::BIT_MATRIX_BOUNDS;
        }
    
        if (A.cols() != bits) {
            return ami::BIT_MATRIX_BOUNDS;
        }

        if (c.rows() != bits) {
            return ami::BIT_MATRIX_BOUNDS;
        }

        if (c.cols() != 1) {
            return ami::BIT_MATRIX_BOUNDS;
        }
    }
        
    // Create the general permutation object.
    tpie::ami::bmmc_as_gen_po<T> gpo(*bpo);
    
    // Do the permutation.
    return tpie::ami::general_permute(instream, outstream, &gpo);
}

#endif // ndef TPIE_LIBRARY

#endif // _TPIE_AMI_BIT_PERMUTE_H 
