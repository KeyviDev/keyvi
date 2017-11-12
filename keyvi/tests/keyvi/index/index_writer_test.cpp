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
 * index_writer_test.cpp
 *
 *  Created on: Jan 13, 2017
 *      Author: hendrik
 */

#include <chrono>
#include <thread>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "index/index_writer.h"

namespace keyvi {
namespace index {
BOOST_AUTO_TEST_SUITE(IndexWriterTests)

BOOST_AUTO_TEST_CASE(simple) {

  using boost::filesystem::temp_directory_path;
  using boost::filesystem::unique_path;

  auto tmp_path = temp_directory_path();
  tmp_path /= unique_path();
  IndexWriter writer(tmp_path.string());

  writer.Set("a", "{\"id\":3}");

  writer.Set("b", "{\"id\":4}");
  writer.Flush();
  writer.Set("c", "{\"id\":5}");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::this_thread::sleep_for(std::chrono::seconds(1));
  writer.Set("d", "{\"id\":6}");
  writer.Flush();
  // std::this_thread::sleep_for(std::chrono::seconds(1));
}

BOOST_AUTO_TEST_CASE(bigger_feed) {
  using boost::filesystem::temp_directory_path;
  using boost::filesystem::unique_path;

  auto tmp_path = temp_directory_path();
  tmp_path /= unique_path();
  IndexWriter writer(tmp_path.string());

  for (int i = 0; i < 100000; ++i) {
    writer.Set("a", "{\"id\":" + std::to_string(i) + "}");
  }
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace index */
} /* namespace keyvi */
