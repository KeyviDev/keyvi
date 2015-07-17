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

#ifndef _TPIE_BTE_STREAM_H
#define _TPIE_BTE_STREAM_H

#include <tpie/config.h>

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#ifdef BTE_VIRTUAL_BASE
#error BTE_VIRTUAL_BASE has been deprecated.
#endif

// Get the base class, enums, etc...
#include <tpie/bte/stream_base.h>

#ifdef BTE_IMP_UFS
#  define BTE_STREAM_IMP_UFS
#endif
    
#ifdef BTE_IMP_MMB
#  define BTE_STREAM_IMP_MMAP
#endif
    
#ifdef BTE_IMP_STDIO
#  define BTE_STREAM_IMP_STDIO
#endif
    
#ifdef BTE_IMP_USER_DEFINED
#  define BTE_STREAM_IMP_USER_DEFINED
#endif
    
// The number of implementations to be defined.
#define _BTE_STREAM_IMP_COUNT (defined(BTE_STREAM_IMP_USER_DEFINED) + \
			       defined(BTE_STREAM_IMP_STDIO) +	      \
			       defined(BTE_STREAM_IMP_MMAP)   +	      \
			       defined(BTE_STREAM_IMP_UFS) )
    
// Multiple implementations are allowed to coexist, with some
// restrictions.
    
// If the including module did not explicitly ask for multiple
// implementations but requested more than one implementation, issue a
// warning.
#ifndef BTE_STREAM_IMP_MULTI_IMP
#  if (_BTE_STREAM_IMP_COUNT > 1)
#    define BTE_STREAM_IMP_MULTI_IMP
#  endif // (_BTE_STREAM_IMP_COUNT > 1)
#endif // BTE_STREAM_IMP_MULTI_IMP
    
// Make sure at least one implementation was chosen.  If none was, then
// choose one by default, but warn the user.
#if (_BTE_STREAM_IMP_COUNT < 1)
#  define BTE_STREAM_IMP_STDIO
#endif // (_BTE_STREAM_IMP_COUNT < 1)
    
// Now include the definitions of each implementation
// that will be used.
    
#ifdef BTE_STREAM_IMP_MULTI_IMP
#  define BTE_STREAM tpie::bte::stream_base
#endif
    
// User defined implementation.
#if defined(BTE_STREAM_IMP_USER_DEFINED)
// Do nothing.  The user will provide a definition of BTE_STREAM.
#endif
    
// stdio implementation.
#if defined(BTE_STREAM_IMP_STDIO)
#  include <tpie/bte/stream_stdio.h>
// If this is the only implementation, then make it easier to get to.
#  ifndef BTE_STREAM_IMP_MULTI_IMP
#    ifdef BTE_STREAM
#      undef BTE_STREAM
#    endif
#    define BTE_STREAM tpie::bte::stream_stdio
#  endif
#endif

// mmap implementation.
#if defined(BTE_STREAM_IMP_MMAP)
#  include <tpie/bte/stream_mmap.h>    
// If this is the only implementation, then make it easier to get to.
#  ifndef BTE_STREAM_IMP_MULTI_IMP
#    ifdef BTE_STREAM
#      undef BTE_STREAM
#    endif
#    define BTE_STREAM tpie::bte::stream_mmap
#  endif
#endif

// ufs implementation.
#if defined(BTE_STREAM_IMP_UFS)
#  include <tpie/bte/stream_ufs.h>
// If this is the only implementation, then make it easier to get to.
#  ifndef BTE_STREAM_IMP_MULTI_IMP
#    ifdef BTE_STREAM
#      undef BTE_STREAM
#    endif
#    define BTE_STREAM tpie::bte::stream_ufs
#  endif
#endif
    
#endif // _TPIE_BTE_STREAM_H 
