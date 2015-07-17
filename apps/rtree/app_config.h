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
// File: app_config.h
// Created: 10/23/97
// Last modified: 10/23/97
//
// Test program for bte_col_rtree.H

#define LOG_BLK_SZ 8*1024  /* HERE WE NEED TO PUT SOMETHING MEANINGFUL */

#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

// Get the configuration set up by the configure script.
#include <tpie/config.h>

#define LOG_BLK_SZ 8*1024  /* HERE WE NEED TO PUT SOMETHING MEANINGFUL */


// <><><><><><><><><><><><><><><><><><><><><><> //
// <><><> Choose default BTE COLLECTION  <><><> //
// <><><><><><><><><><><><><><><><><><><><><><> //

#if (!defined(BTE_COLLECTION_IMP_MMAP) && !defined(BTE_COLLECTION_IMP_UFS) && !defined(BTE_COLLECTION_IMP_USER_DEFINED))
// Define only one (default is BTE_COLLECTION_IMP_MMAP)
#  define BTE_COLLECTION_IMP_MMAP
//#  define BTE_COLLECTION_IMP_UFS
//#  define BTE_COLLECTION_IMP_USER_DEFINED
#endif

// <><><><><><><><><><><><><><><><><><><><><><> //
// <><><><><><> Choose BTE STREAM  <><><><><><> //
// <><><><><><><><><><><><><><><><><><><><><><> //

// Define only one (default is BTE_STREAM_IMP_UFS)
#define BTE_STREAM_IMP_UFS
//#define BTE_STREAM_IMP_MMAP
//#define BTE_STREAM_IMP_STDIO
//#define BTE_STREAM_IMP_USER_DEFINED


// <><><><><><><><><><><><><><><><><><><><><><><><> //
// <> BTE_COLLECTION_MMAP configuration options  <> //
// <><><><><><><><><><><><><><><><><><><><><><><><> //

// Define write behavior.
// Allowed values:
//  0    (synchronous writes)
//  1    (asynchronous writes using MS_ASYNC - see msync(2))
//  2    (asynchronous bulk writes) [default]
#ifndef BTE_COLLECTION_MMAP_LAZY_WRITE
#  define BTE_COLLECTION_MMAP_LAZY_WRITE 2
#endif

// <><><><><><><><><><><><><><><><><><><><><><><><> //
// <><> BTE_STREAM_MMAP configuration options  <><> //
// <><><><><><><><><><><><><><><><><><><><><><><><> //

#ifdef BTE_STREAM_IMP_MMAP
   // Define logical blocksize factor (default is 32)
#  ifndef BTE_STREAM_MMAP_BLOCK_FACTOR
#    define BTE_STREAM_MMAP_BLOCK_FACTOR 32
#  endif
   // Enable/disable TPIE read ahead; default is enabled (set to 1)
#  define BTE_STREAM_MMAP_READ_AHEAD 1
#endif


// <><><><><><><><><><><><><><><><><><><><><><><><> //
// <><> BTE_STREAM_UFS configuration options <><><> //
// <><><><><><><><><><><><><><><><><><><><><><><><> //

#ifdef BTE_STREAM_IMP_UFS
   // Define logical blocksize factor (default is 32)
#  ifndef STREAM_UFS_BLOCK_FACTOR
#    define STREAM_UFS_BLOCK_FACTOR 32
#  endif
   // Enable/disable TPIE read ahead; default is disabled (set to 0)
#  define BTE_STREAM_UFS_READ_AHEAD 0
#endif

#endif
