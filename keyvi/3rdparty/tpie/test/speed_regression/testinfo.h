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

#ifndef __TPIE_TESTINFO__
#define __TPIE_TESTINFO__

#include <iostream>
#include <tpie/sysinfo.h>

namespace tpie {
namespace test {

struct testinfo {
	///////////////////////////////////////////////////////////////////////////
	/// \param testname    Title of the test
	/// \param memory      Memory limit, or zero if we don't care
	/// \param test_mb     Test data size in megabytes, or zero if unspecified
	/// \param test_times  Times to repeat the test, or zero if inapplicable
	///////////////////////////////////////////////////////////////////////////
	inline testinfo(std::string testname, size_t memory = 0, size_t test_mb = 0, size_t test_times = 0) {
		tpie::tpie_init();
		std::cout
			<< std::string(79, '-') << '\n'
			<< testname << "\n\n"
			<< m_sysinfo << '\n';
		if (memory) {
			tpie::get_memory_manager().set_limit(memory*1024*1024);
			m_sysinfo.printinfo("Memory (MB)", memory);
		}
		if (test_mb) {
			m_sysinfo.printinfo("Data (MB)", test_mb);
		}
		if (test_times > 1) {
			m_sysinfo.printinfo("Repeats", test_times);
		}
		std::cout << std::endl;
	}

	inline ~testinfo() {
		m_sysinfo.printinfo("End time", m_sysinfo.localtime());
		m_sysinfo.printinfo("Read (MB)", get_bytes_read()*1.0/(1024*1024));
		m_sysinfo.printinfo("Written (MB)", get_bytes_written()*1.0/(1024*1024));
		const char * labels[] = {
			"Blocked",
			"Held",
			"Waiting",
			"Reading",
			"Writing",
			"Compressing",
			"Uncompressing",
			"Snappy-blocks",
			"None-blocks",
			NULL};
		for (size_t i = 0; labels[i]; ++i) {
			m_sysinfo.printinfo(labels[i], get_user(i));
		}
		tpie::tpie_finish();
	}

private:
	sysinfo m_sysinfo;
};

}
}

#endif // __TPIE_TESTINFO__
