// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino=(0 :
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

#include <iostream>
#include <tpie/file_stream.h>
#include "unit/common.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <tpie/stream_header.h>
#include <errno.h>
#include <vector>
#include <fcntl.h>

static void usage(int exitcode = -1) {
	std::cerr
		<< "Parameters: -t <type> [-v] [-b blocksize] <-o stream | stream1 [stream2 ...]>\n"
		<< '\n'
		<< "  -t        : Element type (-t list to get a list of accepted types)\n"
		<< "  -v        : Verbose\n"
		<< "  -b bs     : Set block size (in bytes)\n"
		<< "  -o stream : Read from standard input, output to specified stream\n"
		<< "  streamn   : Read from specified stream, output to standard output\n"
		<< std::flush;
	if (exitcode >= 0) exit(exitcode);
}

template <typename T>
static inline void output_item(const T & item) {
	std::cout << item << '\n';
}

template <>
inline void output_item<char>(const char & item) {
	std::cout << item;
}

static void throw_errno() {
	throw tpie::io_exception(strerror(errno));
}

static tpie::stream_header_t get_stream_header(const std::string & path) {
	tpie::stream_header_t res;
	int fd = ::open(path.c_str(), O_RDONLY);
	if (fd == -1) throw_errno();
	if (::read(fd, &res, sizeof(res)) != sizeof(res)) throw_errno();
	::close(fd);
	return res;
}

template <typename T>
struct parameter_parser;

template <typename T>
static int read_files(const std::vector<std::string> & files, const parameter_parser<T> & params) {
	if (!files.size()) {
		std::cerr << "No input files specified" << std::endl;
		usage(1);
		return 1;
	}
	int result = 0;
	for (std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
		std::string path = *i;
		tpie::stream_header_t header = get_stream_header(path);
		if (params.m_verbose) {
			std::clog
				<< "Path:           " << path << '\n'
				<< "Header version: " << header.version << '\n'
				<< "Item size:      " << header.itemSize << '\n'
				<< "User data size: " << header.userDataSize << '\n'
				<< "Size:           " << header.size << '\n'
				<< std::flush;
		}
		tpie::file_stream<T> fs;
		try {
			fs.open(path, tpie::access_read, static_cast<tpie::memory_size_type>(header.userDataSize));
		} catch (const tpie::stream_exception & e) {
			std::cerr << "Couldn't open " << path << ": " << e.what() << std::endl;
			result = 1;
			continue;
		}
		while (fs.can_read()) {
			output_item(fs.read());
		}
		fs.close();
	}
	return result;
}

template <typename T>
static int write_file(std::string filename, double blockFactor) {
	tpie::file_stream<T> fs(blockFactor);
	try {
		fs.open(filename);
	} catch (const tpie::stream_exception & e) {
		std::cerr << "Couldn't open " << filename << ": " << e.what() << std::endl;
		return 1;
	}
	T el;
	while (std::cin >> el) {
		fs.write(el);
	}
	fs.close();
	return 0;
}

template <typename base_t>
struct parameter_parser_base {
	inline parameter_parser_base(int argc, char ** argv)
		: m_blockSize(0), m_shouldWrite(false), m_verbose(false), argc(argc), argv(argv) {
	}

	int parse(int offset = 0) {
		if (argc <= offset) return finish();
		std::string arg(argv[offset]);
		if (arg == "-b") {
			std::stringstream(argv[offset+1]) >> m_blockSize;
			return parse(offset+2);
		}
		if (arg == "-o") {
			m_outputFile = argv[offset+1];
			m_shouldWrite = true;
			return parse(offset+2);
		}
		if (arg == "-t") {
			return handle_type_parameter(offset+1);
		}
		if (arg == "-v") {
			m_verbose = true;
			return parse(offset+1);
		}
		return handle_input_file(offset);
	}

	size_t m_blockSize;
	bool m_shouldWrite;
	std::string m_outputFile;
	std::vector<std::string> m_inputFiles;
	bool m_verbose;

	int argc;
	char ** argv;

private:

	int finish() {
		return static_cast<base_t*>(this)->finish();
	}

	int handle_type_parameter(int offset) {
		return static_cast<base_t*>(this)->handle_type_parameter(offset);
	}

	int handle_input_file(int offset) {
		return static_cast<base_t*>(this)->handle_input_file(offset);
	}
};

template <typename T>
struct parameter_parser : public parameter_parser_base<parameter_parser<T> > {
	int handle_type_parameter(int /*offset*/) {
		usage();
		return 1;
	}

	int handle_input_file(int offset) {
		std::string arg(this->argv[offset]);
		this->m_inputFiles.push_back(arg);
		return this->parse(offset+1);
	}

	int finish() {
		double blockFactor = this->m_blockSize ? (double) this->m_blockSize / (2 << 20) : 1.0;
		if (this->m_shouldWrite) {
			return write_file<T>(this->m_outputFile, blockFactor);
		} else {
			return read_files<T>(this->m_inputFiles, *this);
		}
	}
};

struct parameter_parser_notype : public parameter_parser_base<parameter_parser_notype> {
	inline parameter_parser_notype(int argc, char ** argv)
		: parameter_parser_base<parameter_parser_notype>(argc, argv) {
	}

	int finish() {
		usage();
		return 1;
	}

	int handle_input_file(int offset) {
		std::string arg(argv[offset]);
		this->m_inputFiles.push_back(arg);
		return parse(offset+1);
	}

	int handle_type_parameter(int offset) {
		std::string arg(argv[offset]);
		std::stringstream types;

#define trytype(target) \
			types << "  " << #target << '\n';\
			if (arg == #target)\
				return parameter_parser<target>(reinterpret_cast<parameter_parser<target> &>(*this)).parse(offset+1)

		trytype(size_t);
		trytype(char);
		usage();
		std::cerr << "\nAccepted types:\n" << types.str() << std::flush;
		return 1;
	}
};

template <typename T>
static int use_type(int argc, char ** argv) {
	parameter_parser<T> p(argc, argv);
	return p.parse();
}

int main(int argc, char ** argv) {
	tpie_initer _;
	parameter_parser_notype(argc-1, argv+1).parse();
	return EXIT_FAILURE;
}
