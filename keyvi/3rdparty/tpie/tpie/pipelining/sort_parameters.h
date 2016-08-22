// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_SORT_PARAMETERS_H__
#define __TPIE_PIPELINING_SORT_PARAMETERS_H__

#include <tpie/types.h>
#include <iostream>

namespace tpie {

struct sort_parameters {
	/** files available while forming sorted runs. */
	memory_size_type filesPhase1;
	/** memory available while forming sorted runs. */
	memory_size_type memoryPhase1;
	/** files available while merging runs. */
	memory_size_type filesPhase2;
	/** Memory available while merging runs. */
	memory_size_type memoryPhase2;
	/** files available during output phase. */
	memory_size_type filesPhase3;
	/** Memory available during output phase. */
	memory_size_type memoryPhase3;
	/** Run length, subject to memory restrictions during phase 2.
	 * On 32-bit systems, although we could in principle merge runs longer than 2^32,
	 * this is still a memory_size_type since it must be an amount
	 * that we can have in internal memory.
	 */
	memory_size_type runLength;
	/** Maximum item count for internal reporting, subject to memory
	 * restrictions in all phases. Less or equal to runLength. */
	memory_size_type internalReportThreshold;
	/** Fanout of merge tree during phase 2. */
	memory_size_type fanout;
	/** Fanout of merge tree during phase 3. Less or equal to fanout. */
	memory_size_type finalFanout;

	void dump(std::ostream & out) const {
		out << "Merge sort parameters\n"
			<< "Phase 1 files:               " << filesPhase1 << '\n'
			<< "Phase 1 memory:              " << memoryPhase1 << '\n'
			<< "Run length:                  " << runLength << '\n'
			<< "Phase 2 files:               " << filesPhase2 << '\n'
			<< "Phase 2 memory:              " << memoryPhase2 << '\n'
			<< "Fanout:                      " << fanout << '\n'
			<< "Phase 3 files:               " << filesPhase3 << '\n'
			<< "Phase 3 memory:              " << memoryPhase3 << '\n'
			<< "Final merge level fanout:    " << finalFanout << '\n'
			<< "Internal report threshold:   " << internalReportThreshold << '\n';
	}
};

} // namespace tpie

#endif // __TPIE_PIPELINING_SORT_PARAMETERS_H__
