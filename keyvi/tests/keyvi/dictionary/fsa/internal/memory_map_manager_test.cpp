//
// keyvi - A key value store.
//
// Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/*
 * memory_map_manager_test.cpp
 *
 *  Created on: May 8, 2014
 *      Author: hendrik
 */

#include <cstring>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/memory_map_manager.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

BOOST_AUTO_TEST_SUITE(MemoryMapManagerTests)

BOOST_AUTO_TEST_CASE(basic) {
  size_t chunkSize = 1024 * 1024;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path("dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "basic test");

  m.GetAddress(0);
  char* testptr = static_cast<char*>(m.GetAddress(2 * chunkSize));
  std::memset(testptr, 42, chunkSize);

  BOOST_CHECK_EQUAL(testptr[42], 42);
  char* testptr2 = static_cast<char*>(m.GetAddress(chunkSize));
  std::memset(testptr2, 24, chunkSize);

  BOOST_CHECK_EQUAL(testptr[42], 42);
  BOOST_CHECK_EQUAL(testptr2[42], 24);

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE(GetAddressOverflowCheck) {
  size_t chunkSize = 1024 * 1024;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path("dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "basic test");

  m.GetAddress(0);

  BOOST_CHECK(m.GetAddressQuickTestOk(chunkSize - 1, 1));
  BOOST_CHECK(m.GetAddressQuickTestOk(chunkSize - 3, 2));
  BOOST_CHECK(m.GetAddressQuickTestOk(chunkSize - 5, 5));

  BOOST_CHECK(!m.GetAddressQuickTestOk(chunkSize - 1, 2));
  BOOST_CHECK(!m.GetAddressQuickTestOk(chunkSize - 1, 3));
  BOOST_CHECK(!m.GetAddressQuickTestOk(chunkSize - 1, 4));
  BOOST_CHECK(!m.GetAddressQuickTestOk(chunkSize - 1, 5));

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE(GetBuffer) {
  size_t chunkSize = 1024 * 1024;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path("dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "basic test");

  m.GetAddress(0);
  char* testptr = static_cast<char*>(m.GetAddress(chunkSize - 2));
  *testptr = 104;

  testptr = static_cast<char*>(m.GetAddress(chunkSize - 1));
  *testptr = 101;

  testptr = static_cast<char*>(m.GetAddress(chunkSize));
  *testptr = 108;

  testptr = static_cast<char*>(m.GetAddress(chunkSize + 1));
  *testptr = 108;

  testptr = static_cast<char*>(m.GetAddress(chunkSize + 2));
  *testptr = 111;

  char buffer[20];
  buffer[5] = 0;

  m.GetBuffer(chunkSize - 2, buffer, 20);
  BOOST_CHECK_EQUAL("hello", std::string(buffer));

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE(GetBufferAndNumberOfChunks) {
  size_t chunkSize = 1024;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path("dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "basic test");

  char buf[8];
  m.GetBuffer(10, buf, 8);

  BOOST_CHECK_EQUAL(1, m.GetNumberOfChunks());

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE(AppendLargeChunk) {
  size_t chunkSize = 4096;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path("dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "append large chunk test");

  char buffer[16384];
  std::fill(buffer, buffer + 4096, 'y');
  std::fill(buffer + 4096, buffer + 16384, 'x');
  buffer[8887] = 'w';
  buffer[8889] = 'y';

  buffer[9503] = 'a';
  buffer[9504] = 'b';

  buffer[12003] = 'c';
  buffer[12005] = 'd';

  buffer[14000] = 'e';
  buffer[14001] = 'f';

  buffer[16382] = 'g';
  buffer[16383] = 'h';

  m.Append(buffer, 16384);

  BOOST_CHECK_EQUAL('x', (static_cast<char*>(m.GetAddress(8888))[0]));
  BOOST_CHECK_EQUAL('a', (static_cast<char*>(m.GetAddress(9503))[0]));
  BOOST_CHECK_EQUAL('x', (static_cast<char*>(m.GetAddress(12004))[0]));
  BOOST_CHECK_EQUAL('e', (static_cast<char*>(m.GetAddress(14000))[0]));
  BOOST_CHECK_EQUAL('h', (static_cast<char*>(m.GetAddress(16383))[0]));

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE(Appendchunkoverflow) {
  size_t chunkSize = 4096;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path("dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "append large chunk test");

  char buffer[1000];
  std::fill(buffer, buffer + 1000, 'x');
  m.Append(buffer, 1000);
  std::fill(buffer, buffer + 1000, 'y');
  m.Append(buffer, 1000);
  std::fill(buffer, buffer + 1000, 'z');
  m.Append(buffer, 1000);
  std::fill(buffer, buffer + 1000, '1');
  m.Append(buffer, 1000);
  std::fill(buffer, buffer + 1000, '2');
  m.Append(buffer, 1000);
  std::fill(buffer, buffer + 1000, '3');
  m.Append(buffer, 1000);

  BOOST_CHECK_EQUAL('x', (static_cast<char*>(m.GetAddress(999))[0]));
  BOOST_CHECK_EQUAL('y', (static_cast<char*>(m.GetAddress(1567))[0]));
  BOOST_CHECK_EQUAL('z', (static_cast<char*>(m.GetAddress(2356))[0]));
  BOOST_CHECK_EQUAL('1', (static_cast<char*>(m.GetAddress(3333))[0]));
  BOOST_CHECK_EQUAL('2', (static_cast<char*>(m.GetAddress(4444))[0]));
  BOOST_CHECK_EQUAL('3', (static_cast<char*>(m.GetAddress(5555))[0]));
  BOOST_CHECK_EQUAL(6000, m.GetSize());
  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE(chunkbehindtail) {
  size_t chunkSize = 4096;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path("dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "append large chunk test");

  char buffer[1024];
  std::fill(buffer, buffer + 1024, 'x');
  m.Append(buffer, 1024);
  std::fill(buffer, buffer + 1024, 'y');
  m.Append(buffer, 1024);
  std::fill(buffer, buffer + 1024, 'z');
  m.Append(buffer, 1024);
  std::fill(buffer, buffer + 1024, '1');
  m.Append(buffer, 1023);

  // tail should now be at 4096
  BOOST_CHECK_EQUAL(4095, m.GetSize());

  // force a new chunk to be created, although tail does not require it
  m.GetBuffer(4094, &buffer, 10);

  auto filename = path;
  filename /= "out";
  std::ofstream out_stream(filename.native(), std::ios::binary);
  m.Write(out_stream, m.GetSize());
  BOOST_CHECK_EQUAL(4095, out_stream.tellp());
  out_stream.close();

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE(Persist) {
  size_t chunkSize = 4096;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path("dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "append large chunk test");

  char buffer[4096];
  std::fill(buffer, buffer + 4096, 'x');
  m.Append(buffer, 4096);
  m.Append(buffer, 1);

  m.Persist();

  auto filename = path;
  filename /= "out";
  std::ofstream out_stream(filename.native(), std::ios::binary);
  m.Write(out_stream, 0);
  BOOST_CHECK_EQUAL(4097, out_stream.tellp());
  out_stream.close();

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
