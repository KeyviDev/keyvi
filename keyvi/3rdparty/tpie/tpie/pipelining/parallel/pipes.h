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

#ifndef __TPIE_PIPELINING_PARALLEL_PIPES_H__
#define __TPIE_PIPELINING_PARALLEL_PIPES_H__

#include <tpie/job.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/parallel/factory.h>
#include <tpie/pipelining/maintain_order_type.h>

namespace tpie {

namespace pipelining {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Runs a pipeline in multiple threads.
/// \param maintainOrder  Whether to make sure that items are processed and
/// output in the order they are input.
/// \param numJobs  The number of threads to utilize for parallel execution.
/// \param bufSize  The number of items to store in the buffer sent between
/// threads.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
pipe_middle<parallel_bits::factory<fact_t> >
parallel(pipe_middle<fact_t> && fact, maintain_order_type maintainOrder, size_t numJobs, size_t bufSize = 2048) {
	parallel_bits::options opts;
	switch (maintainOrder) {
		case arbitrary_order:
			opts.maintainOrder = false;
			break;
		case maintain_order:
			opts.maintainOrder = true;
			break;
	}
	opts.numJobs = numJobs;
	opts.bufSize = bufSize;
	return pipe_middle<parallel_bits::factory<fact_t> >
		(parallel_bits::factory<fact_t>
		 (std::move(fact.factory), std::move(opts)));
}

///////////////////////////////////////////////////////////////////////////////
/// \brief  Runs a pipeline in multiple threads, using the number of threads
/// reported by tpie::default_worker_count.
/// \param maintainOrder  Whether to make sure that items are processed and
/// output in the order they are input.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
pipe_middle<parallel_bits::factory<fact_t> >
parallel(pipe_middle<fact_t> && fact, maintain_order_type maintainOrder = arbitrary_order) {
	return parallel(std::move(fact), maintainOrder, default_worker_count());
}

template <typename fact_t>
pipe_middle<parallel_bits::factory<fact_t> >
parallel(pipe_middle<fact_t> && fact, bool maintainOrder, size_t numJobs, size_t bufSize = 2048) {
	log_fatal() << "The second argument to tpie::pipelining::parallel has changed.\n"
		<< "Use maintain_order instead of true and arbitrary_order instead of false."
		<< std::endl;
	return parallel(std::move(fact), maintainOrder ? maintain_order : arbitrary_order, numJobs, bufSize);
}

template <typename fact_t>
pipe_middle<parallel_bits::factory<fact_t> >
parallel(pipe_middle<fact_t> && fact, bool maintainOrder) {
	log_fatal() << "The second argument to tpie::pipelining::parallel has changed.\n"
		<< "Use maintain_order instead of true and arbitrary_order instead of false."
		<< std::endl;
	return parallel(std::move(fact), maintainOrder ? maintain_order : arbitrary_order);
}

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PARALLEL_PIPES_H__
