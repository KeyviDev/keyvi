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

#include "keyvi/index/read_only_index.h"
#include "keyvi/testing/index_mock.h"

namespace keyvi {
namespace index {
BOOST_AUTO_TEST_SUITE(ReadOnlyIndexTests)

BOOST_AUTO_TEST_CASE(basicindex) {
  testing::IndexMock index;

  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"abbcd", "{c:3}"}, {"abcde", "{a:1}"}, {"abdd", "{b:2}"},
  };

  index.AddSegment(&test_data);

  std::vector<std::pair<std::string, std::string>> test_data_2 = {
      {"abbcd", "{c:6}"}, {"babc", "{a:1}"}, {"babbc", "{b:2}"}, {"babcde", "{a:1}"}, {"babdd", "{b:2}"},
  };

  index.AddSegment(&test_data_2);

  ReadOnlyIndex reader(index.GetIndexFolder(), {{"refresh_interval", "400"}});

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

BOOST_AUTO_TEST_CASE(indexwithdeletedkeys) {
  testing::IndexMock index;

  std::vector<std::pair<std::string, std::string>> test_data = {
      {"cdefg", "{t:1}"}, {"键", "{b:2}"}, {"核心价值", "{c:3}"}, {"商店", "{a:1}"}, {"störe", "{b:2}"},
  };

  index.AddSegment(&test_data);

  std::vector<std::pair<std::string, std::string>> test_data_2 = {
      {"متجر", "{c:6}"},   {"مفتاح", "{a:1}"}, {"מַפְתֵחַ", "{b:2}"},
      {"babcde", "{a:1}"}, {"商店", "{b:2}"},  {"störe", "{t:44}"},
  };

  index.AddSegment(&test_data_2);

  ReadOnlyIndex reader(index.GetIndexFolder(), {{"refresh_interval", "600"}});

  BOOST_CHECK(reader.Contains("cdefg"));
  BOOST_CHECK(reader.Contains("مفتاح"));
  BOOST_CHECK(reader.Contains("核心价值"));
  BOOST_CHECK(reader.Contains("商店"));
  BOOST_CHECK(!reader.Contains(""));
  BOOST_CHECK(!reader.Contains("תֵ"));

  BOOST_CHECK_EQUAL(reader["מַפְתֵחַ"].GetValueAsString(), "\"{b:2}\"");

  BOOST_CHECK(reader[""].IsEmpty());
  BOOST_CHECK(reader["ab"].IsEmpty());

  // test priority, last one should be returned
  BOOST_CHECK_EQUAL(reader["商店"].GetValueAsString(), "\"{b:2}\"");
  index.AddDeletedKeys({"מַפְתֵחַ", "商店"}, 1);
  reader.Reload();

  BOOST_CHECK(reader.Contains("cdefg"));
  BOOST_CHECK(reader.Contains("störe"));
  BOOST_CHECK(reader.Contains("مفتاح"));
  BOOST_CHECK(reader.Contains("核心价值"));
  BOOST_CHECK(!reader.Contains("商店"));
  BOOST_CHECK(!reader.Contains("מַפְתֵחַ"));

  index.AddDeletedKeys({"störe", "商店"}, 0);
  reader.Reload();
  BOOST_CHECK(reader.Contains("störe"));
  index.AddDeletedKeys({"מַפְתֵחַ", "商店", "störe", "商店"}, 1);
  reader.Reload();
  BOOST_CHECK(!reader.Contains("störe"));
}

void testFuzzyMatching(ReadOnlyIndex* reader, const std::string& query, const size_t max_edit_distance,
                       const size_t minimum_exact_prefix, const std::vector<std::string>& expected) {
  auto expected_it = expected.begin();
  auto matcher = reader->GetFuzzy(query, max_edit_distance, minimum_exact_prefix);
  for (auto m : matcher) {
    BOOST_REQUIRE(expected_it != expected.end());
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected.end());
}

BOOST_AUTO_TEST_CASE(fuzzyMatching) {
  testing::IndexMock index;

  std::vector<std::pair<std::string, std::string>> test_data = {{"abc", "{a:1}"},   {"abbc", "{b:2}"},
                                                                {"abbcd", "{c:3}"}, {"abcde", "{a:1}"},
                                                                {"abdd", "{b:2}"},  {"bbdd", "{b:2}"}};
  index.AddSegment(&test_data);
  std::vector<std::pair<std::string, std::string>> test_data_2 = {
      {"abbcd", "{c:6}"}, {"abcde", "{x:1}"},  {"babc", "{a:1}"},
      {"babbc", "{b:2}"}, {"babcde", "{a:1}"}, {"babdd", "{b:2}"},
  };

  index.AddSegment(&test_data_2);
  ReadOnlyIndex reader_1(index.GetIndexFolder(), {{"refresh_interval", "400"}});
  testFuzzyMatching(&reader_1, "abbc", 0, 2, {"abbc"});
  testFuzzyMatching(&reader_1, "abbc", 1, 2, {"abbc", "abbcd", "abc"});
  testFuzzyMatching(&reader_1, "cde", 2, 3, {});
  testFuzzyMatching(&reader_1, "babbc", 0, 0, {"babbc"});
  testFuzzyMatching(&reader_1, "babbc", 0, 3, {"babbc"});
  testFuzzyMatching(&reader_1, "babbc", 3, 10, {});
  testFuzzyMatching(&reader_1, "abbc", 4, 1, {"abbc", "abbcd", "abc", "abcde", "abdd"});

  index.AddDeletedKeys({"abbcd", "abcde", "babbc"}, 1);
  index.AddDeletedKeys({"abbcd", "bbdd"}, 0);

  ReadOnlyIndex reader_2(index.GetIndexFolder(), {{"refresh_interval", "400"}});

  testFuzzyMatching(&reader_2, "abbc", 0, 2, {"abbc"});
  testFuzzyMatching(&reader_2, "abbc", 1, 2, {"abbc", "abc"});
  testFuzzyMatching(&reader_2, "abbc", 2, 2, {"abbc", "abc", "abdd"});

  testFuzzyMatching(&reader_2, "bbdd", 1, 2, {});
  testFuzzyMatching(&reader_2, "bbdd", 2, 1, {"babdd"});

  testFuzzyMatching(&reader_2, "babbc", 0, 0, {});
  testFuzzyMatching(&reader_2, "babbc", 0, 3, {});
  testFuzzyMatching(&reader_2, "babbc", 2, 3, {"babc", "babdd"});

  testFuzzyMatching(&reader_2, "cde", 2, 3, {});
  testFuzzyMatching(&reader_2, "abbc", 4, 4, {"abbc"});
  testFuzzyMatching(&reader_2, "abbc", 4, 1, {"abbc", "abc", "abdd"});
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace index */
} /* namespace keyvi */
