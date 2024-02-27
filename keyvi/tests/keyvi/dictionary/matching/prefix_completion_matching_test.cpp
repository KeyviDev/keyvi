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

#include "keyvi/dictionary/matching/prefix_completion_matching.h"

#include <algorithm>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace matching {

BOOST_AUTO_TEST_SUITE(PrefixCompletionMatchingTests)

void test_prefix_completion_matching(std::vector<std::pair<std::string, uint32_t>>* test_data, const std::string& query,
                                     const std::vector<std::string> expected) {
  testing::TempDictionary dictionary(test_data);

  // test using weights
  auto matcher_weights = std::make_shared<matching::PrefixCompletionMatching<>>(
      matching::PrefixCompletionMatching<>::FromSingleFsa(dictionary.GetFsa(), query));

  MatchIterator::MatchIteratorPair it = MatchIterator::MakeIteratorPair(
      [matcher_weights]() { return matcher_weights->NextMatch(); }, matcher_weights->FirstMatch());

  auto expected_it = expected.begin();
  for (auto m : it) {
    BOOST_CHECK(expected_it != expected.end());
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }

  // test without weights
  std::vector<std::string> expected_sorted = expected;
  std::sort(expected_sorted.begin(), expected_sorted.end());

  auto matcher_no_weights = std::make_shared<matching::PrefixCompletionMatching<fsa::StateTraverser<>>>(
      matching::PrefixCompletionMatching<fsa::StateTraverser<>>::FromSingleFsa(dictionary.GetFsa(), query));
  MatchIterator::MatchIteratorPair matcher_no_weights_it = MatchIterator::MakeIteratorPair(
      [matcher_no_weights]() { return matcher_no_weights->NextMatch(); }, matcher_no_weights->FirstMatch());

  expected_it = expected_sorted.begin();
  for (auto m : matcher_no_weights_it) {
    BOOST_CHECK(expected_it != expected_sorted.end());
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected_sorted.end());

  // test with multiple dictionaries
  // split test data into 3 groups with some duplication
  std::vector<std::pair<std::string, uint32_t>> test_data_1;
  std::vector<std::pair<std::string, uint32_t>> test_data_2;
  std::vector<std::pair<std::string, uint32_t>> test_data_3;

  for (size_t i = 0; i < test_data->size(); ++i) {
    if (i % 1 == 0 || i % 5 == 0) {
      test_data_1.push_back((*test_data)[i]);
    }
    if (i % 2 == 0 || i == 3) {
      test_data_2.push_back((*test_data)[i]);
    }
    if (i % 3 == 0) {
      test_data_3.push_back((*test_data)[i]);
    }
  }
  testing::TempDictionary d1(&test_data_1);
  testing::TempDictionary d2(&test_data_2);
  testing::TempDictionary d3(&test_data_3);
  std::vector<fsa::automata_t> fsas = {d1.GetFsa(), d2.GetFsa(), d3.GetFsa()};

  auto matcher_zipped =
      std::make_shared<matching::PrefixCompletionMatching<fsa::ZipStateTraverser<fsa::WeightedStateTraverser>>>(
          matching::PrefixCompletionMatching<fsa::ZipStateTraverser<fsa::WeightedStateTraverser>>::FromMulipleFsas(
              fsas, query));
  MatchIterator::MatchIteratorPair matcher_zipped_it = MatchIterator::MakeIteratorPair(
      [matcher_zipped]() { return matcher_zipped->NextMatch(); }, matcher_zipped->FirstMatch());
  expected_it = expected.begin();
  for (auto m : matcher_zipped_it) {
    BOOST_CHECK(expected_it != expected.end());
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected.end());

  auto matcher_zipped_no_weights =
      std::make_shared<matching::PrefixCompletionMatching<fsa::ZipStateTraverser<fsa::StateTraverser<>>>>(
          matching::PrefixCompletionMatching<fsa::ZipStateTraverser<fsa::StateTraverser<>>>::FromMulipleFsas(fsas,
                                                                                                             query));

  MatchIterator::MatchIteratorPair matcher_zipped_no_weights_it =
      MatchIterator::MakeIteratorPair([matcher_zipped_no_weights]() { return matcher_zipped_no_weights->NextMatch(); },
                                      matcher_zipped_no_weights->FirstMatch());
  expected_it = expected_sorted.begin();
  for (auto m : matcher_zipped_no_weights_it) {
    BOOST_CHECK(expected_it != expected_sorted.end());
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected_sorted.end());
}

BOOST_AUTO_TEST_CASE(prefix_0) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"aaaa", 1000}, {"aabb", 1001}, {"aabc", 1002}, {"aacd", 1030}, {"bbcd", 1040}};

  test_prefix_completion_matching(&test_data, "aa", std::vector<std::string>{"aacd", "aabc", "aabb", "aaaa"});
}

BOOST_AUTO_TEST_CASE(prefix_1) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {{"aa", 100},    {"aaaa", 1000}, {"aabb", 1001},
                                                             {"aabc", 1002}, {"aacd", 1030}, {"bbcd", 1040}};

  test_prefix_completion_matching(&test_data, "aa", std::vector<std::string>{"aa", "aacd", "aabc", "aabb", "aaaa"});
}

BOOST_AUTO_TEST_CASE(prefix_completion_empty_input) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"aaaa", 1000}, {"aabb", 1001}, {"aabc", 1002}, {"aacd", 1030}, {"bbcd", 1040}};

  test_prefix_completion_matching(&test_data, "", std::vector<std::string>{"bbcd", "aacd", "aabc", "aabb", "aaaa"});
}

BOOST_AUTO_TEST_CASE(prefix_completion_cjk) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"あsだ", 331},       {"あsだs", 23698},    {"あsaだsっdさ", 18838},
      {"あkだsdさ", 11387}, {"あsだsっd", 10189}, {"あxださ", 10188},
  };
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  test_prefix_completion_matching(&test_data, "あs",
                                  std::vector<std::string>{"あsだ", "あsだs", "あsだsっd", "あsaだsっdさ"});
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
