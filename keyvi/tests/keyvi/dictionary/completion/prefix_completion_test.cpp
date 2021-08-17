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
 * prefix_completion_test.cpp
 *
 *  Created on: Jun 3, 2014
 *      Author: hendrik
 */

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/completion/prefix_completion.h"
#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/testing/temp_dictionary.h"
#include "keyvi/util/configuration.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace completion {

BOOST_AUTO_TEST_SUITE(PrefixCompletionTests)

BOOST_AUTO_TEST_CASE(simple) {
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

  PrefixCompletion prefix_completion(d);

  std::vector<std::string> expected_output;
  expected_output.push_back("aaaa");
  expected_output.push_back("aabb");
  expected_output.push_back("aabc");
  expected_output.push_back("aacd");

  auto expected_it = expected_output.begin();
  for (auto m : prefix_completion.GetCompletions("aa")) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }

  BOOST_CHECK(expected_it == expected_output.end());

  auto prefix_match_pair = prefix_completion.GetCompletions("aaaa");
  MatchIterator prefix = prefix_match_pair.begin();
  BOOST_CHECK_EQUAL((*prefix).GetMatchedString(), "aaaa");
  prefix++;
  BOOST_CHECK(prefix == prefix_match_pair.end());
}

BOOST_AUTO_TEST_CASE(limit) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"angel", 22},          {"angeli", 24},       {"angelina", 444},
      {"angela merkel", 200}, {"angela merk", 180}, {"angelo merk", 10},
  };
  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  std::vector<std::string> expected_output;
  expected_output.push_back("angel");
  expected_output.push_back("angeli");
  expected_output.push_back("angelina");
  expected_output.push_back("angela merk");
  expected_output.push_back("angela merkel");
  expected_output.push_back("angelo merk");

  PrefixCompletion prefix_completion(d);

  auto expected_it = expected_output.begin();
  for (auto m : prefix_completion.GetCompletions("angel")) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }

  BOOST_CHECK(expected_it == expected_output.end());
}

BOOST_AUTO_TEST_CASE(approx1) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"aabc", 22},  {"aabcdefghijklmnop", 45}, {"aabcül", 55}, {"bbbc", 22}, {"bbbd", 444},
      {"cdabc", 22}, {"efdffd", 444},           {"xfdebc", 23},
  };

  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));
  PrefixCompletion prefix_completion(d);

  std::vector<std::string> expected_output;
  expected_output.push_back("aabc");
  // not matching aabcül because of last character mismatch
  expected_output.push_back("aabcdefghijklmnop");  // this matches because aab_c_d, "c" is an insert

  auto expected_it = expected_output.begin();
  for (auto m : prefix_completion.GetFuzzyCompletions("aabd", 2)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected_output.end());

  expected_output.clear();
  expected_output.push_back("aabc");
  expected_output.push_back("aabcül");
  expected_output.push_back("aabcdefghijklmnop");

  expected_it = expected_output.begin();
  for (auto m : prefix_completion.GetFuzzyCompletions("aabc", 2)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected_output.end());
}

// see gh#234
BOOST_AUTO_TEST_CASE(approxWithExactPrefix) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {{"mß", 22}};

  testing::TempDictionary dictionary(&test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));
  PrefixCompletion prefix_completion(d);

  std::vector<std::string> expected_output;
  expected_output.push_back("mß");

  auto expected_it = expected_output.begin();
  for (auto m : prefix_completion.GetFuzzyCompletions("mß", 1)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected_output.end());

  expected_it = expected_output.begin();
  for (auto m : prefix_completion.GetFuzzyCompletions("mß", 2)) {
    BOOST_CHECK_EQUAL(*expected_it++, m.GetMatchedString());
  }
  BOOST_CHECK(expected_it == expected_output.end());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace completion */
} /* namespace dictionary */
} /* namespace keyvi */
