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

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace matching {

BOOST_AUTO_TEST_SUITE(FuzzyMatchingTests)

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
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  std::vector<std::string> expected_output = {"tüv i"};

  auto expected_it = expected_output.begin();
  for (auto m : d->GetFuzzy("tüv i", 0)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }

  BOOST_CHECK(expected_it == expected_output.end());
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
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  std::vector<std::string> expected_output = {
      "tüv i",
      "tüv ib",
      "tüv in",
  };

  auto expected_it = expected_output.begin();
  for (auto m : d->GetFuzzy("tüv in", 1)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }

  BOOST_CHECK(expected_it == expected_output.end());
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
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  std::vector<std::string> expected_output = {
      "türkisch",
      "tülkisc",
  };

  auto expected_it = expected_output.begin();
  for (auto m : d->GetFuzzy("türkisch", 2)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }

  BOOST_CHECK(expected_it == expected_output.end());
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
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  std::vector<std::string> expected_output = {
      "tü",
      "tüv i",
      "tüvk",
  };

  auto expected_it = expected_output.begin();
  for (auto m : d->GetFuzzy("tü", 3)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }

  BOOST_CHECK(expected_it == expected_output.end());
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
  };
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  std::vector<std::string> expected_output = {
      "tüv i", "tüv ib", "tüv in", "türkei side", "türkisch", "türkisch für", "tülkisc",
  };

  auto expected_it = expected_output.begin();
  for (auto m : d->GetFuzzy("türkisch", 5)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }

  BOOST_CHECK(expected_it == expected_output.end());
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
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  auto iter = d->GetFuzzy("türkisch", 2);
  BOOST_CHECK(iter.begin() == iter.end());
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
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  auto iter = d->GetFuzzy("", 7);
  BOOST_CHECK(iter.begin() == iter.end());
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
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  const auto iter1 = d->GetFuzzy("t", 2);
  BOOST_CHECK(iter1.begin() == iter1.end());

  const auto iter2 = d->GetFuzzy("ü", 2);
  BOOST_CHECK(iter2.begin() == iter2.end());
}

BOOST_AUTO_TEST_CASE(fuzzy_cjk) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"あsだ", 331},       {"あsだs", 23698},    {"あsだsっdさ", 18838},
      {"あsだsdさ", 11387}, {"あsだsっd", 10189}, {"あsださ", 10188},
  };
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  std::vector<std::pair<std::string, uint32_t>> expected_output = {
      {"あsだs", 2},
      {"あsだsっd", 0},
      {"あsだsっdさ", 1},
      {"あsだsdさ", 2},
  };

  auto expected_it = expected_output.begin();
  for (auto m : d->GetFuzzy("あsだsっd", 2)) {
    BOOST_CHECK_EQUAL(expected_it->first, m.GetMatchedString());
    BOOST_CHECK_EQUAL(expected_it->second, m.GetScore());
    expected_it++;
  }

  BOOST_CHECK(expected_it == expected_output.end());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
