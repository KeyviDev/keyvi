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
 * index_reader_test.cpp
 *
 *  Created on: Jan 13, 2017
 *      Author: hendrik
 */
#include <chrono>  //NOLINT
#include <thread>  //NOLINT

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "index/index_reader.h"
#include "testing/index_mock.h"

namespace keyvi {
namespace index {
BOOST_AUTO_TEST_SUITE(IndexTests)

BOOST_AUTO_TEST_CASE(loadIndex) {
  testing::IndexMock index;

  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"abbcd", "{c:3}"}, {"abcde", "{a:1}"}, {"abdd", "{b:2}"},
  };

  index.AddSegment(&test_data);

  std::vector<std::pair<std::string, std::string>> test_data_2 = {
      {"abbcd", "{c:6}"}, {"babc", "{a:1}"}, {"babbc", "{b:2}"}, {"babcde", "{a:1}"}, {"babdd", "{b:2}"},
  };

  index.AddSegment(&test_data_2);

  IndexReader reader(index.GetIndexFolder());

  BOOST_CHECK(reader.Contains("abc"));
  BOOST_CHECK(reader.Contains("babdd"));
  BOOST_CHECK(!reader.Contains("ab"));
  BOOST_CHECK(!reader.Contains("bbc"));
  BOOST_CHECK(!reader.Contains(""));
  BOOST_CHECK_EQUAL(reader["abc"].GetValueAsString(), "\"{a:1}\"");

  BOOST_CHECK(reader[""].IsEmpty());
  BOOST_CHECK(reader["ab"].IsEmpty());

  // test priority, last one should be returned
  BOOST_CHECK_EQUAL(reader["abbcd"].GetValueAsString(), "\"{c:6}\"");

  std::vector<std::pair<std::string, std::string>> test_data_3 = {
      {"abbcd", "{c:8}"}, {"cabc", "{a:1}"}, {"cabbc", "{b:2}"}, {"cabcde", "{a:1}"}, {"cabdd", "{b:2}"},
  };

  // sleep for 1s to ensure modification is visible
  std::this_thread::sleep_for(std::chrono::seconds(1));

  index.AddSegment(&test_data_3);
  BOOST_CHECK(reader.Contains("abc"));

  BOOST_CHECK_EQUAL(reader["abbcd"].GetValueAsString(), "\"{c:6}\"");

  // force reload
  reader.Reload();
  BOOST_CHECK(reader.Contains("abc"));

  BOOST_CHECK_EQUAL(reader["abbcd"].GetValueAsString(), "\"{c:8}\"");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::vector<std::pair<std::string, std::string>> test_data_4 = {{"abbcd", "{c:10}"}};
  index.AddSegment(&test_data_4);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  BOOST_CHECK_EQUAL(reader["abbcd"].GetValueAsString(), "\"{c:10}\"");

  std::vector<std::pair<std::string, std::string>> test_data_5 = {{"abbcd", "{c:12}"}};
  index.AddSegment(&test_data_5);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  BOOST_CHECK(reader.Contains("abc"));

  BOOST_CHECK_EQUAL(reader["abbcd"].GetValueAsString(), "\"{c:12}\"");
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace index */
} /* namespace keyvi */
