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

#include <tpie/tpie.h>
#include <tpie/fractional_progress.h>
#include <tpie/execution_time_predictor.h>
#include <tpie/tpie_log.h>
#include <tpie/prime.h>
#include <tpie/memory.h>
#include <tpie/job.h>
#include <tpie/compressed/thread.h>
#include <tpie/compressed/buffer.h>
#include <tpie/hash.h>
#include <tpie/tempname.h>

namespace {
static tpie::memory_size_type the_block_size=0;
}

namespace tpie {

void tpie_init(flags<subsystem> subsystems) {
	if (subsystems & FILE_MANAGER)
	 	init_file_manager();

	if (subsystems & MEMORY_MANAGER)	
	 	init_memory_manager();

	if (subsystems & DEFAULT_LOGGING)
		init_default_log();

	if (subsystems & PRIMEDB)
		init_prime();

	if (subsystems & CAPTURE_FRACTIONS) {
		init_fraction_db(true);
		init_execution_time_db();
	} else if (subsystems & PROGRESS) {
		init_fraction_db(false);
		init_execution_time_db();
	}

	if (subsystems & JOB_MANAGER)
		init_job();

	if (subsystems & STREAMS) {
		init_stream_buffer_pool();
		init_compressor();
	}

	if (subsystems & HASH)
		init_hash();
}

void tpie_finish(flags<subsystem> subsystems) {
	if (subsystems & STREAMS) {
		finish_compressor();
		finish_stream_buffer_pool();
	}

	if (subsystems & JOB_MANAGER)
		finish_job();

    if (subsystems & PROGRESS)  {
		finish_execution_time_db();
		finish_fraction_db();
	}

	if (subsystems & PRIMEDB)
		finish_prime();

	if (subsystems & DEFAULT_LOGGING)
		finish_default_log();

	if (subsystems & MEMORY_MANAGER)	
	 	finish_memory_manager();

	if (subsystems & FILE_MANAGER)
	 	finish_file_manager();

	if (subsystems & TEMPFILE)
		finish_tempfile();
}

memory_size_type get_block_size() {
	if (the_block_size == 0) {
		const char * v = getenv("TPIE_BLOCK_SIZE");
		if (v != NULL) the_block_size = atol(v);
		if (the_block_size == 0) the_block_size=1024*1024*2; //Default block size is 2MB
	}
	return the_block_size;
}

void set_block_size(memory_size_type block_size) {
	the_block_size=block_size;
}

}
