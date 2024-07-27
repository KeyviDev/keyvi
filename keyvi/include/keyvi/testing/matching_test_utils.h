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

#ifndef KEYVI_TESTING_MATCHING_TEST_UTILS_H_
#define KEYVI_TESTING_MATCHING_TEST_UTILS_H_

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/dictionary/fsa/zip_state_traverser.h"
#include "keyvi/dictionary/match_iterator.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace testing {

template <template <class traverserType> class matchingType>
void test_matching(std::vector<std::pair<std::string, uint32_t>>* test_data, const std::string& query,
                   const std::vector<std::string> expected) {
  testing::TempDictionary dictionary(test_data);

  // test using weights
  auto matcher_weights = std::make_shared<matchingType<dictionary::fsa::WeightedStateTraverser>>(
      matchingType<dictionary::fsa::WeightedStateTraverser>::FromSingleFsa(dictionary.GetFsa(), query));

  dictionary::MatchIterator::MatchIteratorPair it = dictionary::MatchIterator::MakeIteratorPair(
      [matcher_weights]() { return matcher_weights->NextMatch(); }, std::move(matcher_weights->FirstMatch()));

  auto expected_it = expected.begin();
  for (auto m : it) {
    BOOST_CHECK(expected_it != expected.end());
    BOOST_CHECK_EQUAL(*expected_it++, m->GetMatchedString());
  }

  // test without weights
  std::vector<std::string> expected_sorted = expected;
  std::sort(expected_sorted.begin(), expected_sorted.end());

  auto matcher_no_weights = std::make_shared<matchingType<dictionary::fsa::StateTraverser<>>>(
      matchingType<dictionary::fsa::StateTraverser<>>::FromSingleFsa(dictionary.GetFsa(), query));
  dictionary::MatchIterator::MatchIteratorPair matcher_no_weights_it = dictionary::MatchIterator::MakeIteratorPair(
      [matcher_no_weights]() { return matcher_no_weights->NextMatch(); }, std::move(matcher_no_weights->FirstMatch()));

  expected_it = expected_sorted.begin();
  for (auto m : matcher_no_weights_it) {
    BOOST_CHECK(expected_it != expected_sorted.end());
    BOOST_CHECK_EQUAL(*expected_it++, m->GetMatchedString());
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
  std::vector<dictionary::fsa::automata_t> fsas = {d1.GetFsa(), d2.GetFsa(), d3.GetFsa()};

  auto matcher_zipped =
      std::make_shared<matchingType<dictionary::fsa::ZipStateTraverser<dictionary::fsa::WeightedStateTraverser>>>(
          matchingType<dictionary::fsa::ZipStateTraverser<dictionary::fsa::WeightedStateTraverser>>::FromMulipleFsas(
              fsas, query));
  dictionary::MatchIterator::MatchIteratorPair matcher_zipped_it = dictionary::MatchIterator::MakeIteratorPair(
      [matcher_zipped]() { return matcher_zipped->NextMatch(); }, std::move(matcher_zipped->FirstMatch()));
  expected_it = expected.begin();
  for (auto m : matcher_zipped_it) {
    BOOST_CHECK(expected_it != expected.end());
    BOOST_CHECK_EQUAL(*expected_it++, m->GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected.end());

  auto matcher_zipped_no_weights =
      std::make_shared<matchingType<dictionary::fsa::ZipStateTraverser<dictionary::fsa::StateTraverser<>>>>(
          matchingType<dictionary::fsa::ZipStateTraverser<dictionary::fsa::StateTraverser<>>>::FromMulipleFsas(fsas,
                                                                                                               query));

  dictionary::MatchIterator::MatchIteratorPair matcher_zipped_no_weights_it =
      dictionary::MatchIterator::MakeIteratorPair(
          [matcher_zipped_no_weights]() { return matcher_zipped_no_weights->NextMatch(); },
          std::move(matcher_zipped_no_weights->FirstMatch()));
  expected_it = expected_sorted.begin();
  for (auto m : matcher_zipped_no_weights_it) {
    BOOST_CHECK(expected_it != expected_sorted.end());
    BOOST_CHECK_EQUAL(*expected_it++, m->GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected_sorted.end());
}

} /* namespace testing */
} /* namespace keyvi */

#endif  // KEYVI_TESTING_MATCHING_TEST_UTILS_H_
