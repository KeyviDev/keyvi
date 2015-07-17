// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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


/* This is an example TPIE program.
 * It writes integers to a file, sorts them, and reads them back in.
 * It uses TPIE progress indicators to notify the user of progress.
 */


#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include <boost/filesystem.hpp> // boost::filesystem::remove
#include <tpie/prime.h> // next_prime

// Progress indicators
#include <tpie/progress_indicator_arrow.h>
#include <tpie/fractional_progress.h>

#include <string>

using namespace tpie;

const char * filename = "helloworld.tpie";

size_t elements = 16*1024*1024;

void cleanup() {
	boost::filesystem::remove(filename);
}

/* Write a permutation of the integers from 0 to s */
void write_number_stream(fractional_subindicator & progress_writer) {
	file_stream<size_t> writer;
	writer.open(filename);

	size_t s = next_prime(elements);
	size_t y = elements-16;

	// The parameter to init tells the PI how many times we will call step().
	progress_writer.init(s);
	for (size_t i = 0; i < s; ++i) {
		// Write a single item
		writer.write((i * y) % s);
		progress_writer.step();
	}
	progress_writer.done();
}

/* Sorting the stream yields an increasing sequence of integers from 0 to s-1 */
void verify_number_stream(fractional_subindicator & progress_sort, fractional_subindicator & progress_verify) {
	file_stream<size_t> numbers;
	numbers.open(filename);
	sort(numbers, numbers, &progress_sort);

	numbers.seek(0);
	size_t expect = 0;
	progress_verify.init(elements);
	while (numbers.can_read()) {
		// Read a single item
		size_t input = numbers.read();
		if (input != expect) {
			progress_verify.done();
			std::cout << "Got a wrong number!" << std::endl;
			return;
		}
		++expect;
		progress_verify.step();
	}
	progress_verify.done();
}

void go();

int main(int argc, char ** argv) {
	// Store program name for usage message
	std::string prog(argv[0]);

	// Iterate through program arguments
	--argc, ++argv;
	while (argc) {
		std::string arg(argv[0]);
		if (arg == "-h" || arg == "--help") {
			std::cout << "Usage: " << prog << " [n]\n";
			std::cout << "Writes n numbers to the file " << filename << ",\n";
			std::cout << "sorts them, and verifies them." << std::endl;
			return EXIT_SUCCESS;
		}

		// the only parameter we accept is the number of items
		size_t n;
		std::stringstream(arg) >> n;
		if (n) elements = n;

		// advance argument pointer
		--argc, ++argv;
	}
#ifdef NO_FRACTION_STATS
	// initialize tpie subsystems (memory manager, job manager for parallel sorting,
	// prime database, progress database, default logger)
	tpie_init();
#include "fractiondb.gen.inl"

#else
	// initialize tpie subsystems and begin tracking progress
	tpie_init(ALL | CAPTURE_FRACTIONS);
	load_fractions("fractiondb.gen.inl");
#endif

	go();

#ifndef NO_FRACTION_STATS
	save_fractions("fractiondb.gen.inl");
	tpie_finish(ALL | CAPTURE_FRACTIONS);
#else
	tpie_finish();
#endif

	return EXIT_SUCCESS;
}

void go() {
	// progress_indicator_arrow draws the progress arrow in the terminal.
	progress_indicator_arrow pi("Hello world", elements);

	// fractional_progress is a progress indicator abstraction that tracks the
	// progress of multiple subroutines as a single progress bar.
	fractional_progress fp(&pi);

	// we have subindicators for each of the subroutines.
	fractional_subindicator progress_writer(fp, "Writer", TPIE_FSI, elements, "Writer");
	fractional_subindicator progress_sort(fp, "Sort", TPIE_FSI, elements, "Sort");
	fractional_subindicator progress_verify(fp, "Verify", TPIE_FSI, elements, "Verify");

	std::cout << "Writing " << next_prime(elements) << " integers to " << filename << std::endl;

	// initialize overall progress indicator
	fp.init();

	cleanup();
	write_number_stream(progress_writer);
	verify_number_stream(progress_sort, progress_verify);
	cleanup();

	fp.done();
}
