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
#include "dictionary/fsa/traverser_types.h"
#include "dictionary/dictionary.h"
#include "dictionary/testing/temp_dictionary.h"
#include "dictionary/dictionary_types.h"


namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE( DictionaryMergerTests )

BOOST_AUTO_TEST_CASE ( MergeKeyOnlyDicts) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary (test_data);

  std::vector<std::string> test_data2 = {"aaaaz", "aabbe", "cdddefgh"};
  testing::TempDictionary dictionary2 (test_data2);

  DictionaryMerger<fsa::internal::SparseArrayPersistence<>> merger(merger_param_t({{"memory_limit_mb","10"}}));
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
  DictionaryMerger<fsa::internal::SparseArrayPersistence<>, fsa::internal::IntInnerWeightsValueStore> merger(merger_param_t({{"memory_limit_mb","10"}}));
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

BOOST_AUTO_TEST_CASE ( MergeIntegerDictsValueMerge) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
            { "abc", 22 },
            { "abbc", 24 }
        };
  testing::TempDictionary dictionary (test_data, false);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
             { "abc", 25 },
             { "abbc", 42 },
             { "abcd", 21 },
             { "abbc", 30 },
         };
  testing::TempDictionary dictionary2 (test_data2, false);

  std::string filename ("merged-dict-int-v1.kv");
  IntDictionaryMerger merger(merger_param_t({{"memory_limit_mb","10"}}));
  merger.Add(dictionary.GetFileName());
  merger.Add(dictionary2.GetFileName());

  merger.Merge(filename);

  fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
  dictionary_t d(new Dictionary(fsa));

  BOOST_CHECK(d->Contains("abc"));
  BOOST_CHECK(d->Contains("abbc"));
  BOOST_CHECK(d->Contains("abcd"));

  BOOST_CHECK_EQUAL("25", d->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("30", d->operator[]("abbc").GetValueAsString());

  std::remove(filename.c_str());

  filename = "merged-dict-int-v2.kv";
  testing::TempDictionary dictionary3(test_data, false);
  IntDictionaryMerger merger2(merger_param_t({{"memory_limit_mb","10"}}));
  merger2.Add(dictionary.GetFileName());
  merger2.Add(dictionary2.GetFileName());
  merger2.Add(dictionary3.GetFileName());

  merger2.Merge(filename);

  fsa::automata_t fsa2(new fsa::Automata(filename.c_str()));
  dictionary_t d2(new Dictionary(fsa2));

  BOOST_CHECK(d2->Contains("abc"));
  BOOST_CHECK(d2->Contains("abbc"));
  BOOST_CHECK(d2->Contains("abcd"));

  BOOST_CHECK_EQUAL("22", d2->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d2->operator[]("abcd").GetValueAsString());

  // overwritten by 2nd
  BOOST_CHECK_EQUAL("24", d2->operator[]("abbc").GetValueAsString());

  std::remove(filename.c_str());

  filename = "merged-dict-int-v3.kv";
  IntDictionaryMerger merger3(merger_param_t({{"memory_limit_mb","10"}}));

  merger3.Add(dictionary2.GetFileName());
  merger3.Add(dictionary.GetFileName());

  merger3.Merge(filename);

  fsa::automata_t fsa3(new fsa::Automata(filename.c_str()));
  dictionary_t d3(new Dictionary(fsa3));

  BOOST_CHECK(d3->Contains("abc"));
  BOOST_CHECK(d3->Contains("abbc"));
  BOOST_CHECK(d3->Contains("abcd"));

  BOOST_CHECK_EQUAL("22", d3->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d3->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("24", d3->operator[]("abbc").GetValueAsString());

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE ( MergeIntegerDictsAppendMerge) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
          { "abc", 22 },
          { "abbc", 24 }
  };
  testing::TempDictionary dictionary (test_data, false);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
          { "abc", 25 },
          { "abbc", 42 },
          { "abcd", 21 },
          { "abbc", 30 },
  };
  testing::TempDictionary dictionary2 (test_data2, false);

  std::string filename ("merged-dict-int-v1.kv");
  IntDictionaryMerger merger(merger_param_t({{"memory_limit_mb","10"}, {"merge_mode","append"}}));
  merger.Add(dictionary.GetFileName());
  merger.Add(dictionary2.GetFileName());

  merger.Merge(filename);

  fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
  dictionary_t d(new Dictionary(fsa));

  BOOST_CHECK(d->Contains("abc"));
  BOOST_CHECK(d->Contains("abbc"));
  BOOST_CHECK(d->Contains("abcd"));

  BOOST_CHECK_EQUAL("25", d->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("30", d->operator[]("abbc").GetValueAsString());

  std::remove(filename.c_str());

  filename = "merged-dict-int-v2.kv";
  testing::TempDictionary dictionary3(test_data, false);
  IntDictionaryMerger merger2(merger_param_t({{"memory_limit_mb","10"}, {"merge_mode","append"}}));
  merger2.Add(dictionary.GetFileName());
  merger2.Add(dictionary2.GetFileName());
  merger2.Add(dictionary3.GetFileName());

  merger2.Merge(filename);

  fsa::automata_t fsa2(new fsa::Automata(filename.c_str()));
  dictionary_t d2(new Dictionary(fsa2));

  BOOST_CHECK(d2->Contains("abc"));
  BOOST_CHECK(d2->Contains("abbc"));
  BOOST_CHECK(d2->Contains("abcd"));

  BOOST_CHECK_EQUAL("22", d2->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d2->operator[]("abcd").GetValueAsString());

  // overwritten by 2nd
  BOOST_CHECK_EQUAL("24", d2->operator[]("abbc").GetValueAsString());

  std::remove(filename.c_str());

  filename = "merged-dict-int-v3.kv";
  IntDictionaryMerger merger3(merger_param_t({{"memory_limit_mb","10"}, {"merge_mode","append"}}));

  merger3.Add(dictionary2.GetFileName());
  merger3.Add(dictionary.GetFileName());

  merger3.Merge(filename);

  fsa::automata_t fsa3(new fsa::Automata(filename.c_str()));
  dictionary_t d3(new Dictionary(fsa3));

  BOOST_CHECK(d3->Contains("abc"));
  BOOST_CHECK(d3->Contains("abbc"));
  BOOST_CHECK(d3->Contains("abcd"));

  BOOST_CHECK_EQUAL("22", d3->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d3->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("24", d3->operator[]("abbc").GetValueAsString());

  std::remove(filename.c_str());
}


BOOST_AUTO_TEST_CASE ( MergeStringDicts) {
  std::vector<std::pair<std::string, std::string>> test_data = {
            { "abc", "a" },
            { "abbc", "b" },
            { "abbcd", "c" },
            { "abcde", "a" },
            { "abdd", "b" },
            { "bba", "c" },
        };
  testing::TempDictionary dictionary (test_data);

  std::vector<std::pair<std::string, std::string>> test_data2 = {
             { "abbe", "d" },
             { "abbc", "z" },
             { "abcd", "a" },
             { "bbacd", "f" },
         };
  testing::TempDictionary dictionary2 (test_data2);

  std::string filename ("merged-dict-string.kv");
  DictionaryMerger<fsa::internal::SparseArrayPersistence<>, fsa::internal::StringValueStore> merger(merger_param_t({{"memory_limit_mb","10"}}));
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

  BOOST_CHECK_EQUAL("a", d->operator[]("abc").GetValueAsString());

  // overwritten by 2nd
  BOOST_CHECK_EQUAL("z", d->operator[]("abbc").GetValueAsString());
  BOOST_CHECK_EQUAL("c", d->operator[]("abbcd").GetValueAsString());
  BOOST_CHECK_EQUAL("a", d->operator[]("abcde").GetValueAsString());
  BOOST_CHECK_EQUAL("b", d->operator[]("abdd").GetValueAsString());
  BOOST_CHECK_EQUAL("c", d->operator[]("bba").GetValueAsString());

  BOOST_CHECK(d->Contains("abcd"));
  BOOST_CHECK(d->Contains("abbe"));
  BOOST_CHECK(d->Contains("bbacd"));

  BOOST_CHECK_EQUAL("a", d->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("d", d->operator[]("abbe").GetValueAsString());
  BOOST_CHECK_EQUAL("f", d->operator[]("bbacd").GetValueAsString());

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE ( MergeJsonDicts) {
  std::vector<std::pair<std::string, std::string>> test_data = {
            { "abc", "{a:1}" },
            { "abbc", "{b:2}" },
            { "abbcd", "{c:3}" },
            { "abcde", "{a:1}" },
            { "abdd", "{b:2}" },
            { "bba", "{c:3}" },
        };
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(test_data);

  std::vector<std::pair<std::string, std::string>> test_data2 = {
             { "abbe", "{d:4}" },
             { "abbc", "{b:3}" },
             { "abcd", "{a:1}" },
             { "bbacd", "{f:5}" },
         };
  testing::TempDictionary dictionary2 = testing::TempDictionary::makeTempDictionaryFromJson(test_data2);

  std::string filename ("merged-dict-json.kv");
  DictionaryMerger<fsa::internal::SparseArrayPersistence<>, fsa::internal::JsonValueStore> merger(merger_param_t({{"memory_limit_mb","10"}}));
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

  BOOST_CHECK_EQUAL("\"{a:1}\"", d->operator[]("abc").GetValueAsString());

  // overwritten by 2nd
  BOOST_CHECK_EQUAL("\"{b:3}\"", d->operator[]("abbc").GetValueAsString());
  BOOST_CHECK_EQUAL("\"{c:3}\"", d->operator[]("abbcd").GetValueAsString());
  BOOST_CHECK_EQUAL("\"{a:1}\"", d->operator[]("abcde").GetValueAsString());
  BOOST_CHECK_EQUAL("\"{b:2}\"", d->operator[]("abdd").GetValueAsString());
  BOOST_CHECK_EQUAL("\"{c:3}\"", d->operator[]("bba").GetValueAsString());

  BOOST_CHECK(d->Contains("abcd"));
  BOOST_CHECK(d->Contains("abbe"));
  BOOST_CHECK(d->Contains("bbacd"));

  BOOST_CHECK_EQUAL("\"{a:1}\"", d->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("\"{d:4}\"", d->operator[]("abbe").GetValueAsString());
  BOOST_CHECK_EQUAL("\"{f:5}\"", d->operator[]("bbacd").GetValueAsString());

  std::remove(filename.c_str());
}


BOOST_AUTO_TEST_CASE ( MergeIncompatible ) {

  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary (test_data);
  DictionaryMerger<fsa::internal::SparseArrayPersistence<>, fsa::internal::IntInnerWeightsValueStore> merger(merger_param_t({{"memory_limit_mb","10"}}));

  BOOST_CHECK_THROW(merger.Add(dictionary.GetFileName()), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE ( MergeIntegerWeightDictsValueMerge) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
            { "abc", 22 },
            { "abbc", 24 }
        };
  testing::TempDictionary dictionary (test_data);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
             { "abc", 25 },
             { "abbc", 42 },
             { "abcd", 21 },
             { "abbc", 30 },
         };
  testing::TempDictionary dictionary2 (test_data2);

  std::string filename ("merged-dict-int-weight-v1.kv");
  DictionaryMerger<fsa::internal::SparseArrayPersistence<>, fsa::internal::IntInnerWeightsValueStore> merger(merger_param_t({{"memory_limit_mb","10"}}));
  merger.Add(dictionary.GetFileName());
  merger.Add(dictionary2.GetFileName());

  merger.Merge(filename);

  fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
  dictionary_t d(new Dictionary(fsa));

  BOOST_CHECK(d->Contains("abc"));
  BOOST_CHECK(d->Contains("abbc"));
  BOOST_CHECK(d->Contains("abcd"));

  BOOST_CHECK_EQUAL("25", d->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("30", d->operator[]("abbc").GetValueAsString());

  fsa::WeightedStateTraverser s(fsa);
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  BOOST_CHECK_EQUAL(30, s.GetInnerWeight());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  BOOST_CHECK_EQUAL(30, s.GetInnerWeight());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK_EQUAL(30, s.GetInnerWeight());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK_EQUAL(30, s.GetInnerWeight());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK_EQUAL(25, s.GetInnerWeight());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK_EQUAL(21, s.GetInnerWeight());
  s++;

  // at end
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());

  std::remove(filename.c_str());

  filename = "merged-dict-int-weight-v2.kv";
  testing::TempDictionary dictionary3(test_data);
  CompletionDictionaryMerger merger2(merger_param_t({{"memory_limit_mb","10"}}));
  merger2.Add(dictionary.GetFileName());
  merger2.Add(dictionary2.GetFileName());
  merger2.Add(dictionary3.GetFileName());

  merger2.Merge(filename);

  fsa::automata_t fsa2(new fsa::Automata(filename.c_str()));
  dictionary_t d2(new Dictionary(fsa2));

  BOOST_CHECK(d2->Contains("abc"));
  BOOST_CHECK(d2->Contains("abbc"));
  BOOST_CHECK(d2->Contains("abcd"));

  BOOST_CHECK_EQUAL("22", d2->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d2->operator[]("abcd").GetValueAsString());

  // overwritten by 2nd
  BOOST_CHECK_EQUAL("24", d2->operator[]("abbc").GetValueAsString());

  std::remove(filename.c_str());

  filename = "merged-dict-int-weight-v3.kv";
  CompletionDictionaryMerger merger3(merger_param_t({{"memory_limit_mb","10"}}));

  merger3.Add(dictionary2.GetFileName());
  merger3.Add(dictionary.GetFileName());

  merger3.Merge(filename);

  fsa::automata_t fsa3(new fsa::Automata(filename.c_str()));
  dictionary_t d3(new Dictionary(fsa3));

  BOOST_CHECK(d3->Contains("abc"));
  BOOST_CHECK(d3->Contains("abbc"));
  BOOST_CHECK(d3->Contains("abcd"));

  BOOST_CHECK_EQUAL("22", d3->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d3->operator[]("abcd").GetValueAsString());
  BOOST_CHECK_EQUAL("24", d3->operator[]("abbc").GetValueAsString());

  fsa::WeightedStateTraverser s3(fsa3);
  BOOST_CHECK_EQUAL('a', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s3.GetDepth());
  BOOST_CHECK_EQUAL(24, s3.GetInnerWeight());
  s3++;

  BOOST_CHECK_EQUAL('b', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s3.GetDepth());
  BOOST_CHECK_EQUAL(24, s3.GetInnerWeight());
  s3++;

  BOOST_CHECK_EQUAL('b', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s3.GetDepth());
  BOOST_CHECK_EQUAL(24, s3.GetInnerWeight());
  s3++;

  BOOST_CHECK_EQUAL('c', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s3.GetDepth());
  BOOST_CHECK_EQUAL(24, s3.GetInnerWeight());
  s3++;

  BOOST_CHECK_EQUAL('c', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s3.GetDepth());
  BOOST_CHECK_EQUAL(22, s3.GetInnerWeight());
  s3++;

  BOOST_CHECK_EQUAL('d', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s3.GetDepth());
  BOOST_CHECK_EQUAL(21, s3.GetInnerWeight());
  s3++;

  // at end
  BOOST_CHECK_EQUAL(0, s3.GetStateLabel());

  std::remove(filename.c_str());
}


BOOST_AUTO_TEST_CASE ( MergeIntegerWeightDictsAppendMerge) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
          { "abc", 22 },
          { "abbc", 24 }
  };
  testing::TempDictionary dictionary (test_data);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
          { "abc", 25 },
          { "abbc", 42 },
          { "abcd", 21 },
          { "abbc", 30 },
  };
  testing::TempDictionary dictionary2 (test_data2);

  std::string filename = "merged-dict-int-weight-v2.kv";
  testing::TempDictionary dictionary3(test_data);
  CompletionDictionaryMerger merger2(merger_param_t({{"memory_limit_mb","10"}, {"merge_mode", "append"}}));
  merger2.Add(dictionary.GetFileName());
  merger2.Add(dictionary2.GetFileName());
  merger2.Add(dictionary3.GetFileName());

  merger2.Merge(filename);

  fsa::automata_t fsa2(new fsa::Automata(filename.c_str()));
  dictionary_t d2(new Dictionary(fsa2));

  BOOST_CHECK(d2->Contains("abc"));
  BOOST_CHECK(d2->Contains("abbc"));
  BOOST_CHECK(d2->Contains("abcd"));

  BOOST_CHECK_EQUAL("22", d2->operator[]("abc").GetValueAsString());
  BOOST_CHECK_EQUAL("21", d2->operator[]("abcd").GetValueAsString());

  // overwritten by 2nd
  BOOST_CHECK_EQUAL("24", d2->operator[]("abbc").GetValueAsString());

  std::remove(filename.c_str());
}


BOOST_AUTO_TEST_CASE (MergeToEmptyDict) {
    std::vector<std::pair<std::string, std::string>> test_data = {};
    testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(test_data);

    std::vector<std::pair<std::string, std::string>> test_data2 = {
            { "abbe", "{d:4}" },
            { "abbc", "{b:3}" },
    };
    testing::TempDictionary dictionary2 = testing::TempDictionary::makeTempDictionaryFromJson(test_data2);

    std::string filename("merge-to-empty-dict.kv");
    JsonDictionaryMerger merger;
    merger.Add(dictionary.GetFileName());
    merger.Add(dictionary2.GetFileName());
    merger.Merge(filename);

    dictionary_t d(new Dictionary(filename));

    BOOST_CHECK(d->Contains("abbc"));
    BOOST_CHECK(d->Contains("abbe"));
    BOOST_CHECK_EQUAL("\"{b:3}\"", d->operator[]("abbc").GetValueAsString());
    BOOST_CHECK_EQUAL("\"{d:4}\"", d->operator[]("abbe").GetValueAsString());

    std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE ( MergeDuplicateAdd) {
      std::vector<std::pair<std::string, std::string>> test_data = {
              { "abbe", "{d:4}" },
              { "abbc", "{b:3}" },
      };
      testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(test_data);

      JsonDictionaryMerger merger;
      merger.Add(dictionary.GetFileName());

      BOOST_CHECK_THROW(merger.Add(dictionary.GetFileName()), std::invalid_argument);
}


BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */




