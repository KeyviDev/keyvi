 // -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team

// This file is part of TPIE.

// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>
#include "../app_config.h"
#include <tpie/portability.h>
#include <cstring>
#include <tpie/file_stream.h>
#include <tpie/file.h>
#include <tpie/util.h>
#include <tpie/file_accessor/stdio.h>
#ifndef WIN32
#include <tpie/file_accessor/posix.h>
#else //WIN32
#include <tpie/file_accessor/win32.h>
#endif //WIN32
using namespace std;
using namespace tpie;

#define ERR(x) {cerr << x << endl; exit(1);}

template <typename T>
void test_file_accessor() {
 	try {
		remove("tmp");
		{
			int d=42;
			
			T x;
			x.open("tmp", false, true, sizeof(int), sizeof(int));
			if (x.size() != 0) ERR("New stream has wrong size");
			if (x.path() != "tmp") ERR("Wrong path");
			
			x.write(&d, 0, 1);
			x.write(&d, 1, 1);
			
			int ud=314;
			x.write_user_data(&ud);
			try {
				x.read(&d, 0, 1);
				ERR("Read should faild");
			} catch(io_exception &) {
				//Do nothing
			}
			if (x.size() != 2) ERR("Wrong size");
			x.close();
		}

		try {
			T x;
			x.open("tmp", true, false, sizeof(int)+1, sizeof(int));
			ERR("Opened file with wrong item size");
		} catch(invalid_file_exception&) {
			//Do nothing
		}
		try {
			T x;
			x.open("tmp", true, false, sizeof(int), 0);
			ERR("Opened file with wrong user data size");
		} catch(invalid_file_exception&) {
			//Do nothing
		}
		
		{
			int d;
			T x;
			x.open("tmp", true, true, sizeof(int), sizeof(int));
			if (x.read(&d, 1, 1) != 1 || d != 42) ERR("Read failed");
			d=12;
			x.write(&d, 1, 1);
			x.write(&d, 2, 1);
			if (x.read(&d, 0, 1) != 1 || d != 42) ERR("Read failed");
			if (x.read(&d, 1, 1) != 1 || d != 12) ERR("Read failed");
			if (x.read(&d, 2, 1) != 1 || d != 12) ERR("Read failed");
			int ud;
			x.read_user_data(&ud);
			
			if (ud != 314) ERR("Wrong user data");
			if (x.size() != 3) ERR("Wrong size");
			x.close();
		}
		
		{
			T x;
			x.open("tmp", true, false, sizeof(int), sizeof(int) );
			try {
				int d=44;
				x.write(&d, 0, 1);
				ERR("Write should faild");
			} catch(io_exception &) {
				//Do nothing
			}
			int d;
			if (x.read(&d, 0, 1) != 1 || d != 42) ERR("Read failed");
			if (x.read(&d, 1, 1) != 1 || d != 12) ERR("Read failed");
			if (x.read(&d, 2, 1) != 1 || d != 12) ERR("Read failed");
			x.close();
		}
	} catch(io_exception & e) {
		ERR("io_exception " << e.what());
	} catch(invalid_file_exception& e) {
		ERR("invalid_file_exception " << e.what());
	} catch(tpie::exception & e) {
		ERR("Other exception " << e.what());
	}
 	remove("tmp");
}

int main(int argc, char ** argv) {
	//TODO add memory allocation tests
 	if (argc == 2 && !strcmp(argv[1], "file_accessor_stdio")) {
 		test_file_accessor<file_accessor::stdio>();
#ifndef WIN32
 	} else if (argc == 2 && !strcmp(argv[1], "file_accessor_posix")) {
 		test_file_accessor<file_accessor::posix>();
#else //WIN32
 	} else if (argc == 2 && !strcmp(argv[1], "file_accessor_win32")) {
 		test_file_accessor<file_accessor::win32>();
#endif //WIN32
 	} else if (argc == 2 && !strcmp(argv[1], "file_stream")) {
		///First a simple test
		double blockFactor=file_base::calculate_block_factor(128*sizeof(int));
		
		remove("tmp");
		int count=40*1024;
		{
			file_stream<int> stream(blockFactor);
			stream.open("tmp", file_base::write, sizeof(int));

			stream.write_user_data<int>(42);
			if (stream.size() != 0) ERR("size failed(1)");
			for(int i=0; i < count; ++i)
				stream.write((i*8209)%8273);
 			if (stream.size() != (size_t)count) ERR("size failed(2)");
 			stream.close();
		}

		{
			file_stream<int> stream(blockFactor);
			stream.open("tmp", file_base::read, sizeof(int));
			if (stream.size() != (size_t)count) ERR("size failed(3)");
			for(int i=0; i< count; ++i) {
				if (stream.can_read() == false) ERR("can_read failed");
				if (stream.read() != (i*8209)%8273) ERR("read failed");
			}
			if (stream.can_read() == true) ERR("can_read failed (2)");
			try {
				int r =stream.read();
				unused(r);
				ERR("read did not fail as expected");
			} catch(end_of_stream_exception &) {
				//Do nothing
			}		
			stream.seek(-1, file_base::end);
			for(int i=count-1; i >= 0; --i) {
				if (stream.can_read_back() == false) {
					ERR("can_read_back failed");
				}
				if (stream.read_back() != (i*8209)%8273) ERR("read back failed");

			}
			if (stream.can_read_back() == true) ERR("can_read_back failed (2)");
			
			int y;
			stream.read_user_data<int>(y);
			if (y != 42) ERR("read did not fail as expected");

			stream.close();
		}
	} else if (argc == 2 && !strcmp(argv[1], "substreams")) {
		double blockFactor=file_base::calculate_block_factor(128*sizeof(int));
		
		temp_file tmp;
		tpie::remove("tmp");
		file<int> f(blockFactor);
		f.open(tmp.path(), file_base::read_write);
 			
		tpie::seed_random(1234);
		const int cnt=4;
		const int size=128;
		typedef file<int>::stream stream_t;
		stream_t ** streams = new stream_t*[cnt];
		for(int i=0; i < cnt; ++i) 
			streams[i] = new stream_t(f);
		
		int content[size];
		for(int i=0; i < size; ++i) {
			content[i] = tpie::random();
			streams[0]->write(content[i]);
		}
		
		for(int i=0; i < 20000; ++i ) {
			int l=tpie::random()%size;
			int s=tpie::random()%cnt;
			streams[s]->seek(l);
			if (tpie::random() % 2 == 0) {
				if (streams[s]->read() != content[l]) ERR("read failed(2)");
			} else {
				content[l] = tpie::random();
				streams[s]->write(content[l]);
			}
		}
		for(int i=0; i < cnt; ++i)
			delete streams[i];
		delete[] streams;
	} else {
		return 1;
	}
	return 0;
}
