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

#ifndef _AMI_STREAM_COMPATIBILITY_H
#define _AMI_STREAM_COMPATIBILITY_H

#include <tpie/portability.h>

#if defined(AMI_IMP_SINGLE)
	TPIE_OS_UNIX_ONLY_WARNING_AMI_IMP_SINGLE
#else
#  define AMI_STREAM_IMP_SINGLE
#endif

#if defined(AMI_STREAM_IMP_USER_DEFINED)
#  warning The AMI_STREAM_IMP_USER_DEFINED flag is obsolete. 
#  warning User-defined AMIs are not supported anymore. 
#  warning Please contact the TPIE Project.
#endif // AMI_STREAM_IMP_USER_DEFINED

#if defined(AMI_STREAM_IMP_MULTI_IMP)
#  warning The AMI_STREAM_IMP_MULTI_IMP flag is obsolete. 
#  warning The usage of multiple AMIs is not supported anymore. 
#  warning Please contact the TPIE Project.
#endif // AMI_STREAM_IMP_MULTI_IMP

#define AMI_STREAM tpie::ami::stream

#define AMI_stream_base tpie::ami::stream
#define AMI_stream_single tpie::ami::stream

#endif // _AMI_STREAM_COMPATIBILITY_H
