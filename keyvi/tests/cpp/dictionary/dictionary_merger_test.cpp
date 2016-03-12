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
 * dictionary_merger_test.cpp
 *
 *  Created on: Feb 29, 2016
 *      Author: hendrik
 */

#include <stdio.h>
#include <boost/test/unit_test.hpp>

#include "dictionary/dictionary_merger.h"
#include "dictionary/fsa/automata.h"
#include "dictionary/dictionary.h"
#include "dictionary/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE( DictionaryMergerTests )

BOOST_AUTO_TEST_CASE ( MergeKeyOnlyDicts) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary (test_data);

  std::vector<std::string> test_data2 = {"aaaaz", "aabbe", "cdddefgh"};
  testing::TempDictionary dictionary2 (test_data2);

  DictionaryMerger<fsa::internal::SparseArrayPersistence<>> merger;
  std::string filename ("merged-dict-key-only.kv");
  merger.Add(dictionary.GetFileName());
  merger.Add(dictionary2.GetFileName());

  merger.Merge(filename);

  fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
  dictionary_t d(new Dictionary(fsa));

  BOOST_CHECK(d->Contains("aaaa"));
  BOOST_CHECK(d->Contains("aabb"));
  BOOST_CHECK(d->Contains("aabc"));
  BOOST_CHECK(d->Contains("aacd"));
  BOOST_CHECK(d->Contains("bbcd"));
  BOOST_CHECK(d->Contains("aaceh"));
  BOOST_CHECK(d->Contains("cdefgh"));

  BOOST_CHECK(d->Contains("aaaaz"));
  BOOST_CHECK(d->Contains("aabbe"));
  BOOST_CHECK(d->Contains("cdddefgh"));


  BOOST_CHECK(!d->Contains("aaab"));
  BOOST_CHECK(!d->Contains("a"));
  BOOST_CHECK(!d->Contains("cde"));

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE ( MergeIntegerDicts) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
            { "abc", 22 },
            { "abbc", 24 },
            { "abbcd", 444 },
            { "abcde", 200 },
            { "abdd", 180 },
            { "bba", 10 },
        };
  testing::TempDictionary dictionary (test_data);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
             { "abbe", 25 },
             { "abcd", 21 },
             { "bbacd", 30 },
         };
  testing::TempDictionary dictionary2 (test_data2);

  std::string filename ("merged-dict-int.kv");
  DictionaryMerger<fsa::internal::SparseArrayPersistence<>, fsa::internal::IntValueStoreWithInnerWeights> merger;
  merger.Add(dictionary.GetFileName());
  merger.Add(dictionary2.GetFileName());

  merger.Merge(filename);

  fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
  dictionary_t d(new Dictionary(fsa));

  BOOST_CHECK(d->Contains("abc"));
  BOOST_CHECK(d->Contains("abbc"));
  BOOST_CHECK(d->Contains("abbcd"));
  BOOST_CHECK(d->Contains("abcde"));
  BOOST_CHECK(d->Contains("abdd"));
  BOOST_CHECK(d->Contains("bba"));

  BOOST_CHECK_EQUAL("22", d->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("24", d->operator[]("abbc").GetValueAsString());
  BOOST_CHECK_EQUAL("444", d->operator[]("abbcd").GetValueAsString());
  BOOST_CHECK_EQUAL("200", d->operator[]("abcde").GetValueAsString());
  BOOST_CHECK_EQUAL("180", d->operator[]("abdd").GetValueAsString());
  BOOST_CHECK_EQUAL("10", d->operator[]("bba").GetValueAsString());

  BOOST_CHECK(d->Contains("abcd"));
  BOOST_CHECK(d->Contains("abbe"));
  BOOST_CHECK(d->Contains("bbacd"));

  BOOST_CHECK_EQUAL("21", d->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("25", d->operator[]("abbe").GetValueAsString());
  BOOST_CHECK_EQUAL("30", d->operator[]("bbacd").GetValueAsString());

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE ( MergeIncompatible ) {

  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary (test_data);
  DictionaryMerger<fsa::internal::SparseArrayPersistence<>, fsa::internal::IntValueStoreWithInnerWeights> merger;

  BOOST_CHECK_THROW(merger.Add(dictionary.GetFileName()), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */




