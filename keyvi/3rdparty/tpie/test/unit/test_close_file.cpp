// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013 The TPIE development team
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

#include "common.h"

#ifndef WIN32
#include <sys/file.h>
#endif

#include <boost/filesystem.hpp>
#include <tpie/tempname.h>
#include <tpie/serialization_stream.h>
#include <tpie/config.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/file_manager.h>

#ifdef WIN32
class open_file_monitor {
public:
	bool ensure_closed_and_delete(std::string fileName) {
		if (!boost::filesystem::exists(fileName)) {
			tpie::log_error() << "ensure_closed_and_delete: File doesn't exist" << std::endl;
			return true;
		}
		try {
			boost::filesystem::remove(fileName);
#if BOOST_FILESYSTEM_VERSION == 2
		} catch (const boost::filesystem::basic_filesystem_error<std::string> & e) {
			tpie::log_debug() << "Caught basic_filesystem_error: " << e.what() << std::endl;
			// Already open?
			return false;
#elif BOOST_FILESYSTEM_VERSION == 3
		} catch (const boost::filesystem::filesystem_error & e) {
			tpie::log_debug() << "Caught filesystem_error: " << e.what() << std::endl;
			// Already open?
			return false;
#else
#error What version of Boost filesystem?
#endif
		} catch (const boost::system::system_error & e) {
			tpie::log_debug() << "Caught system_error: " << e.what() << std::endl;
			// Already open?
			return false;
		}
		return true;
	}
};
#else
class open_file_monitor {
private:
	tpie::memory_size_type m_openFiles;

public:
	open_file_monitor() {
		m_openFiles = tpie::get_file_manager().used();
	}

	bool ensure_closed_and_delete(std::string fileName) {
		tpie::memory_size_type openFiles = tpie::get_file_manager().used();
		tpie::log_debug() << "Open file count was " << m_openFiles
						  << "; is now " << openFiles << std::endl;
		if (openFiles != m_openFiles) {
			// We stipulate that the file was not closed yet.
			return false;
		}
		int e = unlink(fileName.c_str());
		if (e == -1) {
			if (errno == ENOENT) {
				tpie::log_error() << "ensure_closed_and_delete: File doesn't exist" << std::endl;
				return true;
			}
			tpie::log_error() << "Failed to unlink file: " << ::strerror(errno) << std::endl;
			return false;
		}
		if (tpie::get_file_manager().used() != m_openFiles) {
			tpie::log_error() << "ensure_closed_and_delete: Even after unlink, "
								 "file count does not match." << std::endl;
		}
		return true;
	}
};
#endif

// Test that ensure_closed_and_delete actually works
bool test_test() {
	std::string fileName = tpie::tempname::tpie_name();
	tpie::log_debug() << "Temporary file is " << fileName << std::endl;
	boost::filesystem::remove(fileName);
	open_file_monitor m;
	tpie::file_accessor::raw_file_accessor fa;
	fa.open_wo(fileName);
	TEST_ENSURE(boost::filesystem::exists(fileName),
				"fopen did not create file");
	TEST_ENSURE(!m.ensure_closed_and_delete(fileName),
				"ensure_closed_and_delete is wrong");
	fa.close_i();
	TEST_ENSURE(m.ensure_closed_and_delete(fileName),
				"ensure_closed_and_delete is wrong");
	return true;
}

bool serialization_writer_close_test() {
	std::string fileName = tpie::tempname::tpie_name();
	open_file_monitor m;
	tpie::serialization_writer wr;
	wr.open(fileName);
	TEST_ENSURE(!m.ensure_closed_and_delete(fileName),
				"ensure_closed_and_delete is wrong");
	wr.close();
	TEST_ENSURE(m.ensure_closed_and_delete(fileName),
				"serialization_writer::close did not close file");
	return true;
}

bool serialization_writer_dtor_test() {
	std::string fileName = tpie::tempname::tpie_name();
	open_file_monitor m;
	{
		tpie::serialization_writer wr;
		wr.open(fileName);
		TEST_ENSURE(!m.ensure_closed_and_delete(fileName),
					"ensure_closed_and_delete is wrong");
	}
	TEST_ENSURE(m.ensure_closed_and_delete(fileName),
				"serialization_writer dtor did not close file");
	return true;
}

bool serialization_reader_dtor_test() {
	std::string fileName = tpie::tempname::tpie_name();
	open_file_monitor m;
	{
		tpie::serialization_writer wr;
		wr.open(fileName);
		wr.close();
	}
	{
		tpie::serialization_reader rd;
		rd.open(fileName);
		TEST_ENSURE(!m.ensure_closed_and_delete(fileName),
					"ensure_closed_and_delete is wrong");
	}
	TEST_ENSURE(m.ensure_closed_and_delete(fileName),
				"serialization_reader dtor did not close file");
	return true;
}

bool file_limit_enforcement_test() {
	int limit = 5;
	int should_error = limit;

	tpie::get_file_manager().set_limit(limit);
	tpie::get_file_manager().set_enforcement(tpie::file_manager::ENFORCE_THROW);

	std::vector<tpie::file_accessor::raw_file_accessor> fas(should_error + 1);
	int i = 0;
	for (auto &fa : fas) {
		std::string fileName = tpie::tempname::tpie_name();
		try {
			fa.open_wo(fileName);
		} catch(const tpie::out_of_resource_error &e) {
			if (i == should_error) {
				continue;
			} else {
				return false;
			}
		}
		if (i == should_error) {
			return false;
		}
		i++;
	}

	for (auto &fa : fas) {
		fa.close_i();
	}

	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(test_test, "internal")
		.test(serialization_writer_close_test, "serialization_writer_close")
		.test(serialization_writer_dtor_test, "serialization_writer_dtor")
		.test(serialization_reader_dtor_test, "serialization_reader_dtor")
		.test(file_limit_enforcement_test, "file_limit_enforcement")
		;
}
