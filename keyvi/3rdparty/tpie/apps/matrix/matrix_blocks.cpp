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

//
// File: ami_matrix_blocks.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//

#include <tpie/tpie_assert.h>
#include <tpie/config.h>

#include <sys/types.h>

#include <tpie/err.h>
#include <tpie/gen_perm_object.h>
#include "matrix_blocks.h"

using namespace tpie::apps;

perm_matrix_into_blocks::perm_matrix_into_blocks(TPIE_OS_OFFSET rows,
                                                 TPIE_OS_OFFSET cols,
                                                 TPIE_OS_OFFSET block_extent) :
    r(rows),
    c(cols),
    be(block_extent) {
}

perm_matrix_into_blocks::~perm_matrix_into_blocks() {
}

tpie::ami::err perm_matrix_into_blocks::initialize(TPIE_OS_OFFSET len) {
    return static_cast<TPIE_OS_OUTPUT_SIZE_T>( (r * c) == len) ? tpie::ami::NO_ERROR : tpie::ami::MATRIX_BOUNDS;
}

TPIE_OS_OFFSET perm_matrix_into_blocks::destination(TPIE_OS_OFFSET source) {

    tp_assert(r % be == 0, "Rows not a multiple of block extent.");
    tp_assert(c % be == 0, "Cols not a multiple of block extent.");

    // What row and column are the source in?

    TPIE_OS_OFFSET src_row = source / c;
    TPIE_OS_OFFSET src_col = source % c;

    // How many rows of blocks come before the one the source is in?

    TPIE_OS_OFFSET src_brow = src_row / be;

    // How many blocks in the row of blocks that the source is in come
    // before the block the source is in?

    TPIE_OS_OFFSET src_bcol = src_col / be;

    // Number of objects in block rows above.

    TPIE_OS_OFFSET obj_b_above = src_brow * be * c;

    // Number of objects in blocks in the same block row before it.

    TPIE_OS_OFFSET obj_b_left = src_bcol * be * be;
    
    // Position in block

    TPIE_OS_OFFSET bpos = (src_row - be * src_brow) * be +
        (src_col - be * src_bcol);
    
    return obj_b_above + obj_b_left + bpos;
}

perm_matrix_outof_blocks::perm_matrix_outof_blocks(TPIE_OS_OFFSET rows,
                                                   TPIE_OS_OFFSET cols,
                                                   TPIE_OS_OFFSET block_extent) :
    r(rows),
    c(cols),
    be(block_extent) {
}

perm_matrix_outof_blocks::~perm_matrix_outof_blocks() {
}

tpie::ami::err perm_matrix_outof_blocks::initialize(TPIE_OS_OFFSET len) {
    return static_cast<TPIE_OS_OUTPUT_SIZE_T>( (r * c) == len) ? tpie::ami::NO_ERROR : tpie::ami::MATRIX_BOUNDS;
}

TPIE_OS_OFFSET perm_matrix_outof_blocks::destination(TPIE_OS_OFFSET source) {

    tp_assert(r % be == 0, "Rows not a multiple of block extent.");
    tp_assert(c % be == 0, "Cols not a multiple of block extent.");

    // How many full blocks come before source?

    TPIE_OS_OFFSET src_blocks_before = source / (be * be);

    // How many rows of blocks are above the block source is in?

    TPIE_OS_OFFSET src_brow = src_blocks_before / (c / be);

    // How many blocks in the current row are before the block src is in?

    TPIE_OS_OFFSET src_bleft = src_blocks_before % (c / be); 
    
    // What is the position of source in its block?

    TPIE_OS_OFFSET src_pos_in_block = source % (be * be);

    // What is the row of the source in its block?

    TPIE_OS_OFFSET src_row_in_block = src_pos_in_block / be;

    // What is the col of the source in its block?

    TPIE_OS_OFFSET src_col_in_block = src_pos_in_block % be;

    // Number of items in block rows above src.

    TPIE_OS_OFFSET items_brow_above = src_brow * c * be;

    // Number of items in the current block row above src.

    TPIE_OS_OFFSET items_curr_brow_above = src_row_in_block * c;

    // Number of items in item row to left of source.

    TPIE_OS_OFFSET items_left_in_row = (src_bleft * be) + src_col_in_block;

    // Add up everything before it.
    
    return items_brow_above + items_curr_brow_above + items_left_in_row;
}

