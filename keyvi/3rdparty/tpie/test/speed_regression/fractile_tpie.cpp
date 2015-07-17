// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#include <tpie/file_stream.h>
#include <tpie/sort.h>
#include "fractile.h"

class fractile_file_stream {
	tpie::file_stream<item_type> sorted;

public:
	void open(std::string path) {
		tpie::file_stream<item_type> fs;
		fs.open(path, tpie::access_read);
		std::string sortedPath = path + ".sorted";
		boost::filesystem::remove(sortedPath);
		sorted.open(sortedPath);
		sorted.truncate(0);
		tpie::sort(fs, sorted);
		fs.close();
		sorted.close();
		sorted.open(sortedPath, tpie::access_read);
	}
	void close() {
		sorted.close();
	}
	item_type read() {
		return sorted.read();
	}
	tpie::stream_size_type size() {
		return sorted.size();
	}
};

int main(int argc, const char ** argv) {
	return fractile_main<fractile_file_stream>(argc, argv);
}
