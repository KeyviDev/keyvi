// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#ifndef __TPIE_PIPELINING_PARALLEL_WORKER_STATE_H__
#define __TPIE_PIPELINING_PARALLEL_WORKER_STATE_H__

namespace tpie {

namespace pipelining {

namespace parallel_bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief  States of the parallel worker state machine.
///////////////////////////////////////////////////////////////////////////////
enum worker_state {
	/** The input is being written by the producer. */
	INITIALIZING,

	/** The input is being written by the producer. */
	IDLE,

	/** The worker is writing output. */
	PROCESSING,

	/** The worker has filled its output buffer, but has not yet consumed the
	 * input buffer. */
	PARTIAL_OUTPUT,

	/** The output is being read by the consumer. */
	OUTPUTTING,

	/** The worker thread is done. */
	DONE
};

} // namespace parallel_bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PARALLEL_WORKER_STATE_H__
