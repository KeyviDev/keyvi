/* * keyvi - A key value store.
 *
 * Copyright 2024 Hendrik Muhs<hendrik.muhs@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "keyvi/dictionary/matching/multiword_completion_matching.h"

#include <algorithm>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/testing/matching_test_utils.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace matching {

BOOST_AUTO_TEST_SUITE(MultiwordCompletionMatchingTests)

BOOST_AUTO_TEST_CASE(multiword_single_token_0) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"aaaa", 1000}, {"aabb", 1001}, {"aabc", 1002}, {"aacd", 1030}, {"bbcd", 1040}};

  testing::test_matching<matching::MultiwordCompletionMatching>(
      &test_data, "aa", std::vector<std::string>{"aacd", "aabc", "aabb", "aaaa"});
}

BOOST_AUTO_TEST_CASE(multiword_single_token_1) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {{"aa", 100},    {"aaaa", 1000}, {"aabb", 1001},
                                                             {"aabc", 1002}, {"aacd", 1030}, {"bbcd", 1040}};

  testing::test_matching<matching::MultiwordCompletionMatching>(
      &test_data, "aa", std::vector<std::string>{"aa", "aacd", "aabc", "aabb", "aaaa"});
}

BOOST_AUTO_TEST_CASE(multiword_completion_single_token_edge_cases) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"aaaa", 1000}, {"aabb", 1001}, {"aabc", 1002}, {"aacd", 1030}, {"bbcd", 1040}};

  testing::test_matching<matching::MultiwordCompletionMatching>(
      &test_data, "", std::vector<std::string>{"bbcd", "aacd", "aabc", "aabb", "aaaa"});
}

BOOST_AUTO_TEST_CASE(multiword_completion_single_token_cjk) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"あsだ", 331},       {"あsだs", 23698},    {"あsaだsっdさ", 18838},
      {"あkだsdさ", 11387}, {"あsだsっd", 10189}, {"あxださ", 10188},
  };
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  testing::test_matching<matching::MultiwordCompletionMatching>(
      &test_data, "あs", std::vector<std::string>{"あsだ", "あsだs", "あsだsっd", "あsaだsっdさ"});
}

BOOST_AUTO_TEST_CASE(multiword_multi_token_0) {
  // rather basic example, better tests in the python suite
  std::vector<std::pair<std::string, uint32_t>> test_data = {{"aaaa bbb"
                                                              "\x1b"
                                                              "aaaa bbb",
                                                              1000},
                                                             {"aaaa cc"
                                                              "\x1b"
                                                              "aaaa cc",
                                                              1001},
                                                             {"aaaa bc"
                                                              "\x1b"
                                                              "aaaa bc xde",
                                                              1002},
                                                             {"aaaa bc xde"
                                                              "\x1b"
                                                              "aaaa bc xde",
                                                              1002},
                                                             {"bc xde"
                                                              "\x1b"
                                                              "aaaa bc xde",
                                                              1002},
                                                             {"xde"
                                                              "\x1b"
                                                              "aaaa bc xde",
                                                              1040}};

  testing::test_matching<matching::MultiwordCompletionMatching>(&test_data, "aaaa b",
                                                                std::vector<std::string>{"aaaa bc xde", "aaaa bbb"});
  testing::test_matching<matching::MultiwordCompletionMatching>(&test_data, "xd",
                                                                std::vector<std::string>{"aaaa bc xde"});
  testing::test_matching<matching::MultiwordCompletionMatching>(&test_data, "aaaa", std::vector<std::string>{});
  testing::test_matching<matching::MultiwordCompletionMatching>(
      &test_data, "aaaa ", std::vector<std::string>{"aaaa bc xde", "aaaa bbb", "aaaa cc"});
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
