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

#include "dictionary/fsa/internal/memory_map_manager.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

BOOST_AUTO_TEST_SUITE( MemoryMapManagerTests )

BOOST_AUTO_TEST_CASE( basic ) {
  size_t chunkSize = 1024 * 1024;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path(
      "dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "basic test");

  m.GetAddress(0);
  char* testptr = (char*) m.GetAddress(2 * chunkSize);
  std::memset(testptr, 42, chunkSize);

  BOOST_CHECK_EQUAL(testptr[42], 42);
  char* testptr2 = (char*) m.GetAddress(chunkSize);
  std::memset(testptr2, 24, chunkSize);

  BOOST_CHECK_EQUAL(testptr[42], 42);
  BOOST_CHECK_EQUAL(testptr2[42], 24);

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE( GetAddressOverflowCheck ) {
  size_t chunkSize = 1024 * 1024;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path(
      "dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "basic test");

  m.GetAddress(0);

  BOOST_CHECK(m.GetAddressQuickTestOk(chunkSize -1, 1));
  BOOST_CHECK(m.GetAddressQuickTestOk(chunkSize -3, 2));
  BOOST_CHECK(m.GetAddressQuickTestOk(chunkSize -5, 5));

  BOOST_CHECK(!m.GetAddressQuickTestOk(chunkSize -1, 2));
  BOOST_CHECK(!m.GetAddressQuickTestOk(chunkSize -1, 3));
  BOOST_CHECK(!m.GetAddressQuickTestOk(chunkSize -1, 4));
  BOOST_CHECK(!m.GetAddressQuickTestOk(chunkSize -1, 5));

  char* testptr = (char*) m.GetAddress(chunkSize-1);

  boost::filesystem::remove_all(path);
}

BOOST_AUTO_TEST_CASE( GetBuffer ) {
  size_t chunkSize = 1024 * 1024;

  boost::filesystem::path path = boost::filesystem::temp_directory_path();
  path /= boost::filesystem::unique_path(
      "dictionary-fsa-unittest-%%%%-%%%%-%%%%-%%%%");
  boost::filesystem::create_directory(path);
  MemoryMapManager m(chunkSize, path, "basic test");

  m.GetAddress(0);
  char* testptr = (char*) m.GetAddress(chunkSize-2);
  *testptr = 104;

  testptr = (char*) m.GetAddress(chunkSize-1);
  *testptr = 101;

  testptr = (char*) m.GetAddress(chunkSize);
  *testptr = 108;

  testptr = (char*) m.GetAddress(chunkSize+1);
  *testptr = 108;

  testptr = (char*) m.GetAddress(chunkSize+2);
  *testptr = 111;

  char buffer[20];
  buffer[5] = 0;

  m.GetBuffer(chunkSize -2, buffer, 20);
  BOOST_CHECK_EQUAL("hello", std::string(buffer));

  boost::filesystem::remove_all(path);
}


BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
