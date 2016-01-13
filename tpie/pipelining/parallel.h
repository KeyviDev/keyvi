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

#ifndef __TPIE_PIPELINING_PARALLEL_H__
#define __TPIE_PIPELINING_PARALLEL_H__

///////////////////////////////////////////////////////////////////////////////
/// \file parallel.h  Parallel execution of nodes.
///
/// Given a sequential computation as a partial pipeline, this parallel
/// framework naively parallelizes it by having multiple thread handle some
/// items each.
///
/// Throughout the code, the input type is named T1, and the output type is
/// named T2.
///
/// Each worker has a pipeline instance of a parallel_bits::before pushing items to
/// the user-supplied pipeline which pushes to an instance of parallel_bits::after.
///
/// The producer sits in the main thread and distributes item buffers to
/// parallel_bits::befores running in different threads, and the consumer
/// receives the items pushed to each after instance.
///
/// All nodes have access to a single parallel_bits::state instance
/// which has the mutex and the necessary condition variables.
///    It also has pointers to the parallel_bits::before and
/// parallel_bits::after instances and it holds an array of worker states (of
/// enum type parallel_bits::worker_state).
///    It also has a options struct which contains the user-supplied
/// parameters to the framework (size of item buffer and number of concurrent
/// workers).
///
/// The TPIE job framework is insufficient for this parallelization code,
/// since we get deadlocks if some of the workers are allowed to wait for a
/// ready tpie::job worker. Instead, we use std::threads directly.
///
/// Parallel worker states. The main thread has a condition variable
/// (producerCond) which is signalled every time a worker changes its own
/// state. Each worker thread has a condition variable (workerCond[]) which is
/// signalled when the main thread changes the worker's state.
///
/// Initializing: Before the input/output buffers are initialized.
/// -> Idle (worker thread)
///
/// Idle: Input/output buffers are empty.
/// -> Processing (main thread)
///
/// Processing: Input buffer is full; output buffer is empty.
/// -> Partial output (worker thread; signals main)
/// -> Outputting (worker thread)
///
/// Partial output: Output buffer is full; input buffer is non-empty.
/// -> Processing (main thread)
///
/// Outputting: Output buffer is full; input buffer is empty.
/// -> Idle (main thread)
///
/// TODO at some future point: Optimize code for the case where the buffer size
/// is one.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/pipelining/parallel/options.h>
#include <tpie/pipelining/parallel/worker_state.h>
#include <tpie/pipelining/parallel/aligned_array.h>
#include <tpie/pipelining/parallel/base.h>
#include <tpie/pipelining/parallel/factory.h>
#include <tpie/pipelining/parallel/pipes.h>

#endif // __TPIE_PIPELINING_PARALLEL_H__
