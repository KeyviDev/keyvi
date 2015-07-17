// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#include <tpie/unittest.h>
#include <tpie/tpie.h>
#include <tpie/tpie_log.h>
#include <tpie/memory.h>
#include <tpie/tpie_log.h>
#include <sstream>
#include <tpie/sysinfo.h>

namespace tpie {

teststream_buf::teststream_buf() {
	setp(m_line, m_line+line_size-1);
	m_new_line=true;
}

int teststream_buf::overflow(int) {return 0;}

void teststream_buf::stateAlign() {
	if (pptr() - m_line < static_cast<ptrdiff_t>(tests::lineLength)-8) sputc(' ');
	while (pptr() - m_line < static_cast<ptrdiff_t>(tests::lineLength)-9) sputc('.');
	if (pptr() - m_line < static_cast<ptrdiff_t>(tests::lineLength)-8) sputc(' ');
}

int teststream_buf::sync() {
	*pptr() = 0;
	size_t end=0;
	while (m_line[end]) {
		if (m_new_line) log_info() << "  ";
		m_new_line=false;
		size_t start=end;
		while (m_line[end] && m_line[end] != '\n') ++end;
		bool done=(m_line[end] != '\n');
		m_line[end] = '\0';
		log_info() << m_line+start;
		if (!done) {
			log_info() << "\n";
			m_line[end] = '\n';
			m_new_line=true;
			++end;
		}
	}
	log_info() << std::flush;
	setp(m_line, m_line+line_size-1);
	return 0;
}

teststream::teststream(bool do_time): std::ostream(&m_buff), failed(0), total(0), do_time(do_time) {
	if (do_time) 
		time = boost::posix_time::microsec_clock::local_time();
};
bool teststream::success() {return failed ==0;}

void result_manip(teststream & s, bool success) {
	s.m_buff.stateAlign();
	if (success) s << "[ ok ]";
	else {
		s << "[fail]";
		s.failed++;
	}
	s.total++;
	if (s.do_time) {
		boost::posix_time::ptime ntime = boost::posix_time::microsec_clock::local_time();
		s << " " << (ntime - s.time).total_milliseconds() << " ms";
		s.time = ntime;
	}
	s << std::endl;
	

}

testmanip<bool> result(bool success) {return testmanip<bool>(result_manip, success);}
testmanip<bool> success() {return testmanip<bool>(result_manip, true);}
testmanip<bool> failure() {return testmanip<bool>(result_manip, false);}

log_level parse_log_level(const std::string & arg) {
	if (arg == "fatal") return LOG_FATAL;
	if (arg == "error") return LOG_ERROR;
	if (arg == "warning") return LOG_WARNING;
	if (arg == "warn") return LOG_WARNING;
	if (arg == "informational") return LOG_INFORMATIONAL;
	if (arg == "info") return LOG_INFORMATIONAL;
	if (arg == "app_debug") return LOG_APP_DEBUG;
	if (arg == "debug") return LOG_DEBUG;
	if (arg == "mem_debug") return LOG_MEM_DEBUG;
	std::cerr << "Unknown log level \"" << arg << "\"";
	return LOG_FATAL;
}

tests::tests(int argc, char ** argv, memory_size_type memory_limit)
	: tests_runned(0)
	, memory_limit(memory_limit)
{
	exe_name = argv[0];
	testAll = exe_name == "all";
	bool has_seen_test=false;
	bool log_threshold_set = false;
	bad=false;
	usage=false;
	version=false;
	do_time=false;
	log_level log_threshold = LOG_INFORMATIONAL; // default for non-'all' tests
	log_level error_log_threshold = LOG_FATAL;
	for (int i=1; i < argc; ++i) {
		if (argv[i][0] != '-') {
			if (has_seen_test) {
				std::cerr << "More than one test was supplied" << std::endl;
				usage=true;
				bad=true;
				break;
			}
			test_name = argv[i];
			testAll = test_name == "all";
			if (testAll && !log_threshold_set) {
				log_threshold = LOG_ERROR; // default for 'all' tests
			}
			has_seen_test=true;
			continue;
		}

		std::string arg = argv[i];

		if (arg == "-v" || arg == "--verbose") {
			log_threshold = LOG_INFORMATIONAL;
			log_threshold_set = true;
			continue;
		}

		if (arg == "-d" || arg == "--debug") {
			log_threshold = LOG_DEBUG;
			log_threshold_set = true;
			continue;
		}

		if (arg == "-l" || arg == "--log-level") {
			std::string nextarg = (i+1 < argc) ? argv[i+1] : "";
			log_threshold = parse_log_level(nextarg);
			log_threshold_set = true;
			++i;
			continue;
		}

		if (arg == "-e" || arg == "--log-level-on-error") {
			std::string nextarg = (i+1 < argc) ? argv[i+1] : "";
			error_log_threshold = parse_log_level(nextarg);
			++i;
			continue;
		}

		if (arg == "-t" || arg == "--time") {
			do_time=true;
		}

		if (arg == "-V" || arg == "--version") {
			version=true;
			break;
		}

		if (arg == "-h" || arg == "--help") {
			usage=true;
			break;
		}

		if (!has_seen_test) {
			usage=true;
			bad=true;
			std::cerr << "Unknown switch " << arg << std::endl;
			break;
		}

		if (i + 1 < argc && argv[i+1][0] != '-') {
			const char * key = argv[i]+2;
			if (arg[1] != '-' && arg.size() == 2) key = argv[i]+1;
			args[key] = argv[i+1];
			++i;
		} else
			args[argv[i]+2] = "";
	}

	if (!usage && !version && !has_seen_test) {
		std::cerr << "No test supplied" << std::endl;
		usage=true;
		bad=true;
	}
	log.set_threshold(log_threshold);
	log.set_buffer_threshold(error_log_threshold);
	tpie::tpie_init(tpie::ALL & ~tpie::DEFAULT_LOGGING);
	get_log().add_target(&log);
	tpie::get_memory_manager().set_limit(get_arg("memory", memory_limit)*1024*1024);
}

tests::~tests() {
	get_log().remove_target(&log);
	tpie::tpie_finish(tpie::ALL & ~tpie::DEFAULT_LOGGING);
}

void tests::start_test(const std::string & name) {
	++tests_runned;
	log.set_test(name);
	for (size_t i=0; i < setups.size(); ++i)
		(*setups[i])();
}

void tests::end_test(bool result, size_t time) {
	if (result)
		log.set_status(" ok ");
	else
		log.set_status("fail");
	if (do_time) log.write_time(time);
	if (!result) {
		std::string bufferedlog = log.buffer.str();
		std::cerr << bufferedlog << std::flush;
	}

	for (size_t i=0; i < finishs.size(); ++i)
		(*finishs[i])();

	if (!result) bad=true;
}

void tests::build_information(std::ostream & o) {
	sysinfo si;
	o << si << std::flush;
}	

void tests::show_usage(std::ostream & o) {
	o << "Usage: " << exe_name << " [TEST_NAME] [OPTION]..." << std::endl;
	o << "Run the given unit test.\n\n";
	o << "Available tests:" << std::endl;
	for (size_t i = 0; i < m_tests.size(); ++i) {
		o << "  " << m_tests[i] << std::endl;
	}
	o << '\n';
	o << "Mandatory arguments to long options are mandatory for short options too.\n";
	o << "  -h, --help                         Show this help message" << std::endl;
	o << "  -l, --log-level LEVEL              Output log to standard output" << std::endl;
	o << "  -e, --log-level-on-error LEVEL     Output log when a test fails" << std::endl;
	o << "  -v, --verbose                      Alias of --log-level info" << std::endl;
	o << "  -d, --debug                        Alias of --log-level debug" << std::endl;
	o << "  -t, --time                        Time tests" << std::endl;
	o << "  -V, --version                      Show version information" << std::endl;
	o << "      --memory MB                    Set memory limit (default: "
	  << memory_limit << " MB)\n\n";

	o << "LEVEL may be one of the following:\n";
	o << "fatal, error, warn, info, app_debug, debug, mem_debug\n\n";

	o << "When TEST_NAME is \"all\", all tests are run, and log level defaults to \"error\".\n";
	o << "When running just a single test, the default log level is \"info\".\n";
}

tests::operator int() {
	if (!usage && !version && tests_runned == 0) {
		std::cerr << "The test " << test_name << " does not exist" << std::endl;
		usage=bad=true;
	}
	if (usage) {
		if (bad) {
			std::cerr << std::endl;
			show_usage(std::cerr);
		} else
			show_usage(std::cout);
	}

	if (version) {
		build_information(std::cout);
	}
	
	if (bad) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

namespace bits {

	test_runner::test_runner(tests * t, const std::string & name)
		: t(t)
		, result(false)
	{
		t->start_test(name);
		if (t->do_time)
			m_time = boost::posix_time::microsec_clock::local_time();
	}

	test_runner::~test_runner() {
		t->end_test(result, 
					t->do_time?(boost::posix_time::microsec_clock::local_time() - m_time).total_milliseconds(): 0);
	}

	void test_runner::set_result(bool result) {
		this->result = result;
	}

	void test_runner::exception() {
		try {
			throw;
		} catch (const tpie::job_manager_exception & e) {
			log_error() << "Unexpected job_manager_exception: [" << e.what() << "]\n";
		} catch (const tpie::end_of_stream_exception & e) {
			log_error() << "Unexpected end_of_stream_exception: [" << e.what() << "]\n";
		} catch (const tpie::invalid_file_exception & e) {
			log_error() << "Unexpected invalid_file_exception: [" << e.what() << "]\n";
		} catch (const tpie::out_of_space_exception & e) {
			log_error() << "Unexpected out_of_space_exception: [" << e.what() << "]\n";
		} catch (const tpie::io_exception & e) {
			log_error() << "Unexpected io_exception: [" << e.what() << "]\n";
		} catch (const tpie::stream_exception & e) {
			log_error() << "Unexpected stream_exception: [" << e.what() << "]\n";
		} catch (const tpie::invalid_argument_exception & e) {
			log_error() << "Unexpected invalid_argument_exception: [" << e.what() << "]\n";
		} catch (const tpie::exception & e) {
			log_error() << "Unexpected TPIE exception: [" << e.what() << "]\n";
		} catch (const std::runtime_error & e) {
			log_error() << "Unexpected std::runtime_error: [" << e.what() << "]\n";
		}
		set_result(false);
	}

} // namespace bits

} //namespace tpie
