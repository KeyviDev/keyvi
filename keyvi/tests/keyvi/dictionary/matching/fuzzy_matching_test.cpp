/* * keyvi - A key value store.
 *
 * Copyright 2018   Narek Gharibyan<narekgharibyan@gmail.com>
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

/*
 * fuzzy_match_test.cpp
 *
 *  Created on: February 11, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#include <algorithm>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/matching/fuzzy_matching.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace matching {

BOOST_AUTO_TEST_SUITE(FuzzyMatchingTests)

void test_fuzzy_matching(std::vector<std::pair<std::string, uint32_t>>* test_data, const std::string& query,
                         size_t max_edit_distance, const std::vector<std::string> expected) {
  testing::TempDictionary dictionary(test_data);

  // test using weights
  auto matcher_weights = std::make_shared<matching::FuzzyMatching<>>(
      matching::FuzzyMatching<>::FromSingleFsa<>(dictionary.GetFsa(), query, max_edit_distance));

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

  auto matcher_no_weights = std::make_shared<matching::FuzzyMatching<fsa::StateTraverser<>>>(
      matching::FuzzyMatching<fsa::StateTraverser<>>::FromSingleFsa<fsa::StateTraverser<>>(dictionary.GetFsa(), query,
                                                                                           max_edit_distance));
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

  auto matcher_zipped = std::make_shared<matching::FuzzyMatching<fsa::ZipStateTraverser<fsa::WeightedStateTraverser>>>(
      matching::FuzzyMatching<fsa::ZipStateTraverser<fsa::WeightedStateTraverser>>::FromMulipleFsas<
          fsa::WeightedStateTraverser>(fsas, query, max_edit_distance));
  MatchIterator::MatchIteratorPair matcher_zipped_it = MatchIterator::MakeIteratorPair(
      [matcher_zipped]() { return matcher_zipped->NextMatch(); }, matcher_zipped->FirstMatch());
  expected_it = expected.begin();
  for (auto m : matcher_zipped_it) {
    BOOST_CHECK(expected_it != expected.end());
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected.end());

  auto matcher_zipped_no_weights =
      std::make_shared<matching::FuzzyMatching<fsa::ZipStateTraverser<fsa::StateTraverser<>>>>(
          matching::FuzzyMatching<fsa::ZipStateTraverser<fsa::StateTraverser<>>>::FromMulipleFsas<
              fsa::StateTraverser<>>(fsas, query, max_edit_distance));
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

BOOST_AUTO_TEST_CASE(fuzzy_0) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"türkei news", 23698},
      {"türkei side", 18838},
      {"türkei urlaub", 23424},
      {"türkisch anfänger", 20788},
      {"türkisch für", 21655},
      {"türkisch für anfänger", 20735},
      {"türkçe dublaj", 28575},
      {"türkçe dublaj izle", 16391},
      {"türkçe izle", 19946},
      {"tüv akademie", 9557},
      {"tüv hessen", 7744},
      {"tüv i", 331},
      {"tüv in", 10188},
      {"tüv ib", 10189},
      {"tüv kosten", 11387},
      {"tüv nord", 46052},
      {"tüs rhein", 462},
      {"tüs rheinland", 39131},
      {"tüs öffnungszeiten", 15999},
  };

  test_fuzzy_matching(&test_data, "tüv i", 0, std::vector<std::string>{"tüv i"});
}

BOOST_AUTO_TEST_CASE(fuzzy_1) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"türkei news", 23698},
      {"türkei side", 18838},
      {"türkei urlaub", 23424},
      {"türkisch anfänger", 20788},
      {"türkisch für", 21655},
      {"türkisch für anfänger", 20735},
      {"türkçe dublaj", 28575},
      {"türkçe dublaj izle", 16391},
      {"türkçe izle", 19946},
      {"tüv akademie", 9557},
      {"tüv hessen", 7744},
      {"tüv jnohn", 334},
      {"tüv jnack", 331},
      {"tüv i", 331},
      {"tüv in", 10188},
      {"tüv ib", 10189},
      {"tüv kosten", 11387},
      {"tüv nord", 46052},
      {"tüs rhein", 462},
      {"tüs rheinland", 39131},
      {"tüs öffnungszeiten", 15999},
  };

  test_fuzzy_matching(&test_data, "tüv in", 1, std::vector<std::string>{"tüv i", "tüv ib", "tüv in"});
}

BOOST_AUTO_TEST_CASE(fuzzy_2) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"türkei news", 23698},
      {"türkei side", 18838},
      {"türkei urlaub", 23424},
      {"türkisch anfänger", 20788},
      {"türisch anfänger", 20788},
      {"türkisch", 21657},
      {"tülkisc", 21654},
      {"türkisch für", 21655},
      {"türkisch für anfänger", 20735},
      {"türkçe dublaj", 28575},
      {"türkçe dublaj izle", 16391},
      {"türkçe izle", 19946},
      {"tüv akademie", 9557},
      {"tüv hessen", 7744},
      {"tüv jnohn", 334},
      {"tüv jnack", 331},
      {"tüv i", 331},
      {"tüv in", 10188},
      {"tüv ib", 10189},
      {"tüv kosten", 11387},
      {"tüv nord", 46052},
      {"tüs rhein", 462},
      {"tüs rheinland", 39131},
      {"tüs öffnungszeiten", 15999},
  };

  test_fuzzy_matching(&test_data, "türkisch", 2, std::vector<std::string>{"türkisch", "tülkisc"});
}

BOOST_AUTO_TEST_CASE(fuzzy_2_exact_minimum_prefix) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"türkei news", 23698},
      {"türkei side", 18838},
      {"türkei urlaub", 23424},
      {"türkisch anfänger", 20788},
      {"türkisch für", 21655},
      {"türkisch für anfänger", 20735},
      {"türkçe dublaj", 28575},
      {"türkçe dublaj izle", 16391},
      {"türkçe izle", 19946},
      {"tüv akademie", 9557},
      {"tüv hessen", 7744},
      {"tü", 340},
      {"tüv i", 331},
      {"tüvk", 334},
      {"tüv in", 10188},
      {"tüv ib", 10189},
      {"tüv kosten", 11387},
      {"tüv nord", 46052},
      {"tüs rhein", 462},
      {"tüs rheinland", 39131},
      {"tüs öffnungszeiten", 15999},
  };

  test_fuzzy_matching(&test_data, "tü", 3, std::vector<std::string>{"tü", "tüv i", "tüvk"});
}

BOOST_AUTO_TEST_CASE(fuzzy_5) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"türkei news", 23698},
      {"türkei side", 18838},
      {"türkei urlaub", 23424},
      {"türkisch anfänger", 20788},
      {"türisch anfänger", 20788},
      {"türkisch", 21657},
      {"tülkisc", 21654},
      {"türkisch für", 21655},
      {"türkisch für anfänger", 20735},
      {"türkçe dublaj", 28575},
      {"türkçe dublaj izle", 16391},
      {"türkçe izle", 19946},
      {"tüv akademie", 9557},
      {"tüv hessen", 7744},
      {"tüv jnohn", 334},
      {"tüv jnack", 331},
      {"tüv i", 331},
      {"tüv in", 10188},
      {"tüv ib", 10189},
      {"tüv kosten", 11387},
      {"tüv nord", 46052},
      {"tüs rhein", 462},
      {"tüs rheinland", 39131},
      {"tüs öffnungszeiten", 15999},
      {"abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 42}};

  test_fuzzy_matching(&test_data, "türkisch", 5,
                      std::vector<std::string>{
                          "tüv i",
                          "tüv ib",
                          "tüv in",
                          "türkei side",
                          "türkisch",
                          "türkisch für",
                          "tülkisc",
                      });

  test_fuzzy_matching(&test_data, "abcdefghijXXmnopqrstuvqxyzXXXDEFGHIJKLMNOPQRSTUVWXYZ", 5,
                      std::vector<std::string>{"abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"});
}

BOOST_AUTO_TEST_CASE(fuzzy_no_match) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"türkei news", 23698},
      {"türkei side", 18838},
      {"türkei urlaub", 23424},
      {"türkisch anfänger", 20788},
      {"türkisch für", 21655},
      {"türkisch für anfänger", 20735},
      {"türkçe dublaj", 28575},
      {"türkçe dublaj izle", 16391},
      {"türkçe izle", 19946},
      {"tüv akademie", 9557},
      {"tüv hessen", 7744},
      {"tüv i", 331},
      {"tüv in", 10188},
      {"tüv ib", 10189},
      {"tüv kosten", 11387},
      {"tüv nord", 46052},
      {"tüs rhein", 462},
      {"tüs rheinland", 39131},
      {"tüs öffnungszeiten", 15999},
  };
  test_fuzzy_matching(&test_data, "türkisch", 2, std::vector<std::string>{});
}

BOOST_AUTO_TEST_CASE(fuzzy_empty_input) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"türkei news", 23698},
      {"türkei side", 18838},
      {"türkei urlaub", 23424},
      {"türkisch anfänger", 20788},
      {"türkisch für", 21655},
      {"türkisch für anfänger", 20735},
      {"türkçe dublaj", 28575},
      {"türkçe dublaj izle", 16391},
      {"türkçe izle", 19946},
      {"tüv akademie", 9557},
      {"tüv hessen", 7744},
      {"tüv i", 331},
      {"tüv in", 10188},
      {"tüv ib", 10189},
      {"tüv kosten", 11387},
      {"tüv nord", 46052},
      {"tüs rhein", 462},
      {"tüs rheinland", 39131},
      {"tüs öffnungszeiten", 15999},
  };
  test_fuzzy_matching(&test_data, "", 7, std::vector<std::string>{});
}

BOOST_AUTO_TEST_CASE(fuzzy_short_prefix) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"türkei news", 23698},
      {"türkei side", 18838},
      {"türkei urlaub", 23424},
      {"türkisch anfänger", 20788},
      {"türkisch für", 21655},
      {"türkisch für anfänger", 20735},
      {"türkçe dublaj", 28575},
      {"türkçe dublaj izle", 16391},
      {"türkçe izle", 19946},
      {"tüv akademie", 9557},
      {"tüv hessen", 7744},
      {"üüv i", 331},
      {"üüv in", 10188},
      {"üüv ib", 10189},
      {"üüv kosten", 11387},
      {"üüv nord", 46052},
      {"üüs rhein", 462},
      {"üüs rheinland", 39131},
      {"üüs öffnungszeiten", 15999},
  };
  test_fuzzy_matching(&test_data, "t", 2, std::vector<std::string>{});
  test_fuzzy_matching(&test_data, "ü", 2, std::vector<std::string>{});
}

BOOST_AUTO_TEST_CASE(fuzzy_cjk) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"あsだ", 331},       {"あsだs", 23698},    {"あsだsっdさ", 18838},
      {"あsだsdさ", 11387}, {"あsだsっd", 10189}, {"あsださ", 10188},
  };
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  test_fuzzy_matching(&test_data, "あsだsっd", 2,
                      std::vector<std::string>{"あsだs", "あsだsっd", "あsだsっdさ", "あsだsdさ"});
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
