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
 * dictionary_test.cpp
 *
 *  Created on: Jun 7, 2014
 *      Author: hendrik
 */

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/testing/temp_dictionary.h"
#include "keyvi/util/configuration.h"

namespace keyvi {
namespace dictionary {
BOOST_AUTO_TEST_SUITE(DictionaryTests)

BOOST_AUTO_TEST_CASE(loadDict) {
  fsa::internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

  fsa::Generator<fsa::internal::SparseArrayPersistence<>> g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
  g.Add("aaaa");
  g.Add("aabb");
  g.Add("aabc");
  g.Add("aacd");
  g.Add("bbcd");

  g.CloseFeeding();

  std::ofstream out_stream("testFile_completion", std::ios::binary);
  g.Write(out_stream);
  out_stream.close();

  fsa::automata_t f(new fsa::Automata("testFile_completion"));
  dictionary_t d(new Dictionary(f));
}

BOOST_AUTO_TEST_CASE(DictGet) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"test", 22},
      {"otherkey", 24},
      {"other", 444},
      {"bar", 200},
  };

  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  bool matched = false;
  for (auto m : d->Get("test")) {
    BOOST_CHECK_EQUAL("test", m.GetMatchedString());
    BOOST_CHECK_EQUAL(std::string("22"), boost::get<std::string>(m.GetAttribute("weight")));
    matched = true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Get("test2")) {
    matched = true;
  }

  BOOST_CHECK(!matched);

  auto m = (*d)["test"];
  BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
}

BOOST_AUTO_TEST_CASE(DictLookup) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"nude", 22},
      {"nude-party", 24},
  };

  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  bool matched = false;
  for (auto m : d->Lookup("nude")) {
    BOOST_CHECK_EQUAL("nude", m.GetMatchedString());
    BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
    matched = true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Lookup("nude ")) {
    BOOST_CHECK_EQUAL("nude", m.GetMatchedString());
    BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
    matched = true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Lookup("nude at work")) {
    BOOST_CHECK_EQUAL("nude", m.GetMatchedString());
    BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
    matched = true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Lookup("nudelsalat")) {
    matched = true;
  }
  BOOST_CHECK(!matched);
}

BOOST_AUTO_TEST_CASE(DictGetNear) {
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"pizzeria:u281z7hfvzq9", "pizzeria in Munich"}, {"pizzeria:u0vu7uqfyqkg", "pizzeria in Mainz"},
      {"pizzeria:u33db8mmzj1t", "pizzeria in Berlin"}, {"pizzeria:u0yjjd65eqy0", "pizzeria in Frankfurt"},
      {"pizzeria:u28db8mmzj1t", "pizzeria in Munich"}, {"pizzeria:u2817uqfyqkg", "pizzeria in Munich"},
      {"pizzeria:u281wu8bmmzq", "pizzeria in Munich"}};

  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  std::vector<std::string> expected_matches = {"pizzeria:u281wu8bmmzq"};

  size_t i = 0;

  // check near match for pizzeria:u28, it should only return 1 match "281wu" is the closest
  for (auto m : d->GetNear("pizzeria:u281wu88kekq", 12)) {
    if (i >= expected_matches.size()) {
      BOOST_FAIL("got more results than expected.");
    }
    BOOST_CHECK_EQUAL(expected_matches[i++], m.GetMatchedString());
  }

  BOOST_CHECK_EQUAL(expected_matches.size(), i);

  expected_matches = {"pizzeria:u2817uqfyqkg", "pizzeria:u281wu8bmmzq", "pizzeria:u281z7hfvzq9"};

  i = 0;

  // check near match for pizzeria:u28, it should only return 2 matches u2817 and u281w are equally good
  for (auto m : d->GetNear("pizzeria:u2815u88kekq", 12)) {
    if (i >= expected_matches.size()) {
      BOOST_FAIL("got more results than expected.");
    }
    BOOST_CHECK_EQUAL(expected_matches[i++], m.GetMatchedString());
  }

  BOOST_CHECK_EQUAL(expected_matches.size(), i);

  // check exact match
  expected_matches = {"pizzeria:u281wu8bmmzq"};

  i = 0;

  // check near match for pizzeria:u28, it should only return 1 match "281wu" is the closest
  for (auto m : d->GetNear("pizzeria:u281wu8bmmzq", 12)) {
    if (i >= expected_matches.size()) {
      BOOST_FAIL("got more results than expected.");
    }
    BOOST_CHECK_EQUAL(expected_matches[i++], m.GetMatchedString());
  }

  BOOST_CHECK_EQUAL(expected_matches.size(), i);

  i = 0;

  // check near match for the full location pizzeria:u281wu8bmmzq
  for (auto m : d->GetNear("pizzeria:u281wu8bmmzq", 21)) {
    if (i >= expected_matches.size()) {
      BOOST_FAIL("got more results than expected.");
    }
    BOOST_CHECK_EQUAL(expected_matches[i++], m.GetMatchedString());
  }

  BOOST_CHECK_EQUAL(expected_matches.size(), i);
}

BOOST_AUTO_TEST_CASE(DictGetZerobyte) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {std::string("\0test", 5), 22},
      {"otherkey", 24},
      {std::string("ot\0her", 6), 444},
      {"bar\0", 200},
  };

  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  bool matched = false;
  for (auto m : d->Get(std::string("\0test", 5))) {
    BOOST_CHECK_EQUAL(std::string("\0test", 5), m.GetMatchedString());
    BOOST_CHECK_EQUAL(std::string("22"), boost::get<std::string>(m.GetAttribute("weight")));
    matched = true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Get("test2")) {
    matched = true;
  }

  BOOST_CHECK(!matched);

  auto m = (*d)[std::string("\0test", 5)];
  BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */
