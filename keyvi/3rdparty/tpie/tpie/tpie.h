// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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
#ifndef __TPIE_TPIE_H__
#define __TPIE_TPIE_H__

#include <tpie/config.h>
#include <tpie/types.h>
#include <tpie/flags.h>

///////////////////////////////////////////////////////////////////////////////
/// \file tpie.h tpie_init and tpie_finish.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \namespace tpie TPIE's namespace.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief Subsystems of TPIE.
///////////////////////////////////////////////////////////////////////////////
enum subsystem {
	/** \brief Needed for tpie_new and tpie_delete and implicitly needed by all
	 * TPIE algorithm and data structure implementations. */
    MEMORY_MANAGER=1,
	/** \brief TPIE logging framework. See \ref tpie_log.h. */
    DEFAULT_LOGGING=2,
	/** \brief Progress tracking. Needed for the fraction database. */
    PROGRESS=4,
	/** \brief Prime number database, for \ref prime.h. */
	PRIMEDB=8,
	/** \brief Job manager, for \ref job.h and the parallel quick sort. */
	JOB_MANAGER=16,
	/** \brief Capture fractions. */
	CAPTURE_FRACTIONS=32,
	/** \brief Enable support for streams. */
	STREAMS=64,
	/** \brief Generate random hashcodes for tabulation hashing*/
	HASH=128,
	/** \brief Generate temporary files */
	TEMPFILE=256,
	/** \brief Needed for working with files and implicitly by all
	 * TPIE algorithm and data structure implementations. */
	FILE_MANAGER=512,
	/** \brief Alias for all default subsystems. */
    ALL=MEMORY_MANAGER | DEFAULT_LOGGING | PROGRESS | PRIMEDB | JOB_MANAGER | STREAMS | HASH | TEMPFILE | FILE_MANAGER
};

TPIE_DECLARE_OPERATORS_FOR_FLAGS(subsystem)

///////////////////////////////////////////////////////////////////////////////
/// \brief Initialize the given subsystems of TPIE.
/// \param subsystems Logical OR of \ref subsystem entries.
///////////////////////////////////////////////////////////////////////////////
void tpie_init(flags<subsystem> subsystems=ALL);

///////////////////////////////////////////////////////////////////////////////
/// \brief Deinitialize the given subsystems of TPIE.
/// You MUST pass the same bitmask of subsystems to tpie_finish as you did to
/// tpie_init.
/// \param subsystems Logical OR of \ref subsystem entries.
///////////////////////////////////////////////////////////////////////////////
void tpie_finish(flags<subsystem> subsystems=ALL);

///////////////////////////////////////////////////////////////////////////////
/// \brief Get the TPIE block size.
/// This can be changed by setting the TPIE_BLOCK_SIZE environment variable
/// or by calling the set_block_size method.
///
/// The default is 2 MiB (2**21 bytes).
///////////////////////////////////////////////////////////////////////////////
memory_size_type get_block_size();

///////////////////////////////////////////////////////////////////////////////
/// \brief Set the TPIE block size.
///
/// It is not safe to change the block size when any streams are open.
///////////////////////////////////////////////////////////////////////////////
void set_block_size(memory_size_type block_size);

} //namespace tpie

#endif //__TPIE_TPIE_H__
