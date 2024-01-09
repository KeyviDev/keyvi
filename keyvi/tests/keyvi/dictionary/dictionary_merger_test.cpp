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

#include <unordered_set>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/dictionary_merger.h"
#include "keyvi/dictionary/dictionary_types.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/testing/temp_dictionary.h"
#include "keyvi/util/configuration.h"

namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE(DictionaryMergerTests)

BOOST_AUTO_TEST_CASE(MergeKeyOnlyDicts) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary(&test_data);

  std::vector<std::string> test_data2 = {"aaaaz", "aabbe", "cdddefgh"};
  testing::TempDictionary dictionary2(&test_data2);

  keyvi::util::parameters_t merge_configurations[] = {{{"memory_limit_mb", "10"}},
                                                      {{"memory_limit_mb", "10"}, {"merge_mode", "append"}}};

  for (const auto& params : merge_configurations) {
    DictionaryMerger<> merger(params);
    std::string filename("merged-dict-key-only.kv");
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

    BOOST_CHECK_EQUAL(0, merger.GetStats().deleted_keys_);
    BOOST_CHECK_EQUAL(0, merger.GetStats().updated_keys_);
    BOOST_CHECK_EQUAL(10, merger.GetStats().number_of_keys_);

    std::remove(filename.c_str());
  }
}

BOOST_AUTO_TEST_CASE(MergeIntegerDicts) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"abc", 22}, {"abbc", 24}, {"abbcd", 444}, {"abcde", 200}, {"abdd", 180}, {"bba", 10},
  };
  testing::TempDictionary dictionary(&test_data);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"abbe", 25},
      {"abcd", 21},
      {"bbacd", 30},
  };
  testing::TempDictionary dictionary2(&test_data2);

  std::string filename("merged-dict-int.kv");
  DictionaryMerger<dictionary_type_t::INT_WITH_WEIGHTS> merger(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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

BOOST_AUTO_TEST_CASE(MergeIntegerDictsValueMerge) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {{"abc", 22}, {"abbc", 24}};
  testing::TempDictionary dictionary(&test_data, false);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"abc", 25},
      {"abbc", 42},
      {"abcd", 21},
      {"abbc", 30},
  };
  testing::TempDictionary dictionary2(&test_data2, false);

  std::string filename("merged-dict-int-v1.kv");
  IntDictionaryMerger merger(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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
  testing::TempDictionary dictionary3(&test_data, false);
  IntDictionaryMerger merger2(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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
  IntDictionaryMerger merger3(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

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

BOOST_AUTO_TEST_CASE(MergeIntegerDictsAppendMerge) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {{"abc", 22}, {"abbc", 24}};
  testing::TempDictionary dictionary(&test_data, false);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"abc", 25},
      {"abbc", 42},
      {"abcd", 21},
      {"abbc", 30},
  };
  testing::TempDictionary dictionary2(&test_data2, false);

  std::string filename("merged-dict-int-v1.kv");
  IntDictionaryMerger merger(keyvi::util::parameters_t({{"memory_limit_mb", "10"}, {"merge_mode", "append"}}));
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
  testing::TempDictionary dictionary3(&test_data, false);
  IntDictionaryMerger merger2(keyvi::util::parameters_t({{"memory_limit_mb", "10"}, {"merge_mode", "append"}}));
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
  IntDictionaryMerger merger3(keyvi::util::parameters_t({{"memory_limit_mb", "10"}, {"merge_mode", "append"}}));

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

BOOST_AUTO_TEST_CASE(MergeStringDicts) {
  keyvi::util::parameters_t merge_configurations[] = {{{"memory_limit_mb", "10"}},
                                                      {{"memory_limit_mb", "10"}, {"merge_mode", "append"}}};

  for (const auto& params : merge_configurations) {
    std::vector<std::pair<std::string, std::string>> test_data = {
        {"abc", "a"}, {"abbc", "b"}, {"abbcd", "c"}, {"abcde", "a"}, {"abdd", "b"}, {"bba", "c"},
    };
    testing::TempDictionary dictionary(&test_data);

    std::vector<std::pair<std::string, std::string>> test_data2 = {
        {"abbe", "d"},
        {"abbc", "z"},
        {"abcd", "a"},
        {"bbacd", "f"},
    };
    testing::TempDictionary dictionary2(&test_data2);

    std::string filename("merged-dict-string.kv");
    DictionaryMerger<dictionary_type_t::STRING> merger(params);
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
}

BOOST_AUTO_TEST_CASE(MergeJsonDicts) {
  keyvi::util::parameters_t merge_configurations[] = {{{"memory_limit_mb", "10"}},
                                                      {{"memory_limit_mb", "10"}, {"merge_mode", "append"}}};

  for (const auto& params : merge_configurations) {
    std::vector<std::pair<std::string, std::string>> test_data = {
        {"abc", "{a:1}"},   {"abbc", "{b:2}"}, {"abbcd", "{c:3}"},
        {"abcde", "{a:1}"}, {"abdd", "{b:2}"}, {"bba", "{c:3}"},
    };
    testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

    std::vector<std::pair<std::string, std::string>> test_data2 = {
        {"abbe", "{d:4}"},
        {"abbc", "{b:3}"},
        {"abcd", "{a:1}"},
        {"bbacd", "{f:5}"},
    };
    testing::TempDictionary dictionary2 = testing::TempDictionary::makeTempDictionaryFromJson(&test_data2);

    std::string filename("merged-dict-json.kv");
    DictionaryMerger<dictionary_type_t::JSON> merger(params);
    merger.Add(dictionary.GetFileName());
    merger.Add(dictionary2.GetFileName());

    merger.Merge(filename);

    fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
    dictionary_t d(new Dictionary(fsa));

    BOOST_CHECK(d->Contains("abc"));
    BOOST_CHECK(d->Contains("abbc"));
    BOOST_CHECK(d->Contains("abbcd"));
    BOOST_CHECK(d->Contains("abbe"));
    BOOST_CHECK(d->Contains("abcd"));
    BOOST_CHECK(d->Contains("abcde"));
    BOOST_CHECK(d->Contains("abdd"));
    BOOST_CHECK(d->Contains("bba"));
    BOOST_CHECK(d->Contains("bbacd"));

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

    BOOST_CHECK_EQUAL(0, merger.GetStats().deleted_keys_);
    BOOST_CHECK_EQUAL(1, merger.GetStats().updated_keys_);
    BOOST_CHECK_EQUAL(9, merger.GetStats().number_of_keys_);

    std::remove(filename.c_str());
  }
}

BOOST_AUTO_TEST_CASE(MergeFloatVectorDicts, *boost::unit_test::tolerance(0.00001)) {
  keyvi::util::parameters_t merge_configurations[] = {{{"memory_limit_mb", "10"}},
                                                      {{"memory_limit_mb", "10"}, {"merge_mode", "append"}}};

  for (const auto& params : merge_configurations) {
    std::vector<std::pair<std::string, std::vector<float>>> test_data = {
        {"abc", {0.1, 0.2, 0.3, 0.4, 0.5}},   {"abbc", {0.1, 1.2, 0.3, 0.4, 0.5}}, {"abbcd", {0.1, 2.2, 0.3, 0.4, 0.5}},
        {"abcde", {0.1, 3.2, 0.3, 0.4, 0.5}}, {"abdd", {0.1, 4.2, 0.3, 0.4, 0.5}}, {"bba", {0.1, 5.2, 0.3, 0.4, 0.5}},
    };
    testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromFloats(&test_data);

    std::vector<std::pair<std::string, std::vector<float>>> test_data2 = {
        {"abbe", {3.1, 0.2, 1.3, 0.4, 0.5}},
        {"abbc", {3.1, 0.2, 2.3, 0.4, 0.5}},
        {"abcd", {3.1, 0.2, 3.3, 0.4, 0.5}},
        {"bbacd", {3.1, 0.2, 4.3, 0.4, 0.5}},
    };

    testing::TempDictionary dictionary2 = testing::TempDictionary::makeTempDictionaryFromFloats(&test_data2);
    std::string filename("merged-dict-float-vector.kv");
    DictionaryMerger<dictionary_type_t::FLOAT_VECTOR> merger(params);
    merger.Add(dictionary.GetFileName());
    merger.Add(dictionary2.GetFileName());

    merger.Merge(filename);

    fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
    dictionary_t d(new Dictionary(fsa));

    BOOST_CHECK(d->Contains("abc"));
    BOOST_CHECK(d->Contains("abbc"));
    BOOST_CHECK(d->Contains("abbcd"));
    BOOST_CHECK(d->Contains("abbe"));
    BOOST_CHECK(d->Contains("abcd"));
    BOOST_CHECK(d->Contains("abcde"));
    BOOST_CHECK(d->Contains("abdd"));
    BOOST_CHECK(d->Contains("bba"));
    BOOST_CHECK(d->Contains("bbacd"));

    auto v = keyvi::util::DecodeFloatVector(d->operator[]("abc").GetRawValueAsString());
    BOOST_CHECK_EQUAL(5, v.size());
    BOOST_TEST(0.4 == v[3]);
    BOOST_TEST(0.2 == v[1]);

    v = keyvi::util::DecodeFloatVector(d->operator[]("abbc").GetRawValueAsString());
    BOOST_CHECK_EQUAL(5, v.size());
    BOOST_TEST(3.1 == v[0]);
    BOOST_TEST(2.3 == v[2]);

    BOOST_CHECK_EQUAL(0, merger.GetStats().deleted_keys_);
    BOOST_CHECK_EQUAL(1, merger.GetStats().updated_keys_);
    BOOST_CHECK_EQUAL(9, merger.GetStats().number_of_keys_);

    std::remove(filename.c_str());
  }
}

BOOST_AUTO_TEST_CASE(MergeIncompatible) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary(&test_data);

  DictionaryMerger<dictionary_type_t::INT_WITH_WEIGHTS> merger(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

  BOOST_CHECK_THROW(merger.Add(dictionary.GetFileName()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(MergeIncompatibleFloatVectorDicts, *boost::unit_test::tolerance(0.00001)) {
  std::vector<std::pair<std::string, std::vector<float>>> test_data = {
      {"abc", {0.1, 0.2, 0.3, 0.4, 0.5}},
      {"abbc", {0.1, 1.2, 0.3, 0.4, 0.5}},
      {"abbcd", {0.1, 2.2, 0.3, 0.4, 0.5}},
  };
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromFloats(&test_data);

  std::vector<std::pair<std::string, std::vector<float>>> test_data2 = {
      {"abbe", {3.1, 0.2, 1.3, 0.4, 0.5, 3.1, 0.2, 2.3, 0.4, 0.5}},
  };

  testing::TempDictionary dictionary2 = testing::TempDictionary::makeTempDictionaryFromFloats(&test_data2);
  std::string filename("merged-dict-float-vector.kv");
  DictionaryMerger<dictionary_type_t::FLOAT_VECTOR> merger(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
  merger.Add(dictionary.GetFileName());
  BOOST_CHECK_THROW(merger.Add(dictionary2.GetFileName()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(MergeIntegerWeightDictsValueMerge) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {{"abc", 22}, {"abbc", 24}};
  testing::TempDictionary dictionary(&test_data);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"abc", 25},
      {"abbc", 42},
      {"abcd", 21},
      {"abbc", 30},
  };
  testing::TempDictionary dictionary2(&test_data2);

  std::string filename("merged-dict-int-weight-v1.kv");
  DictionaryMerger<dictionary_type_t::INT_WITH_WEIGHTS> merger(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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
  testing::TempDictionary dictionary3(&test_data);
  CompletionDictionaryMerger merger2(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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
  CompletionDictionaryMerger merger3(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

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

BOOST_AUTO_TEST_CASE(MergeIntegerWeightDictsAppendMerge) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {{"abc", 22}, {"abbc", 24}};
  testing::TempDictionary dictionary(&test_data);

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"abc", 25},
      {"abbc", 42},
      {"abcd", 21},
      {"abbc", 30},
  };
  testing::TempDictionary dictionary2(&test_data2);

  std::string filename = "merged-dict-int-weight-v2.kv";
  testing::TempDictionary dictionary3(&test_data);
  CompletionDictionaryMerger merger2(keyvi::util::parameters_t({{"memory_limit_mb", "10"}, {"merge_mode", "append"}}));
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

BOOST_AUTO_TEST_CASE(MergeToEmptyDict) {
  std::vector<std::pair<std::string, std::string>> test_data = {};
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  std::vector<std::pair<std::string, std::string>> test_data2 = {
      {"abbe", "{d:4}"},
      {"abbc", "{b:3}"},
  };
  testing::TempDictionary dictionary2 = testing::TempDictionary::makeTempDictionaryFromJson(&test_data2);

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

  BOOST_CHECK_EQUAL(0, merger.GetStats().deleted_keys_);
  BOOST_CHECK_EQUAL(0, merger.GetStats().updated_keys_);
  BOOST_CHECK_EQUAL(2, merger.GetStats().number_of_keys_);

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE(MergeDuplicateAdd) {
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abbe", "{d:4}"},
      {"abbc", "{b:3}"},
  };
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  JsonDictionaryMerger merger;
  merger.Add(dictionary.GetFileName());

  BOOST_CHECK_THROW(merger.Add(dictionary.GetFileName()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Delete) {
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abcd", "{g:5}"},
      {"xyz", "{t:4}"},
  };
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  boost::filesystem::path deleted_keys_file{dictionary.GetFileName()};
  deleted_keys_file += ".dk";
  JsonDictionaryMerger merger;
  {
    std::unordered_set<std::string> deleted_keys{"xyz"};
    std::ofstream out_stream(deleted_keys_file.string(), std::ios::binary);
    msgpack::pack(out_stream, deleted_keys);
  }

  merger.Add(dictionary.GetFileName());

  std::string filename("merge-delete-key-dict.kv");
  std::ofstream out_stream(filename, std::ios::binary);
  merger.Merge();
  merger.Write(out_stream);
  out_stream.close();

  fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
  dictionary_t d(new Dictionary(fsa));

  BOOST_CHECK(d->Contains("abcd"));
  BOOST_CHECK(!d->Contains("xyz"));

  BOOST_CHECK_EQUAL(1, merger.GetStats().deleted_keys_);
  BOOST_CHECK_EQUAL(0, merger.GetStats().updated_keys_);
  BOOST_CHECK_EQUAL(1, merger.GetStats().number_of_keys_);

  std::remove(filename.c_str());
  std::remove(deleted_keys_file.string().c_str());
}

BOOST_AUTO_TEST_CASE(MultipleDeletes) {
  std::vector<std::pair<std::string, std::string>> test_data1 = {
      {"abcd", "{g:5}"},   {"abbc", "{t:4}"}, {"abbcd", "{u:3}"}, {"abbd", "{v:2}"},
      {"abbdef", "{w:1}"}, {"abbe", "{x:0}"}, {"acdd", "{y:-1}"}, {"afgh", "{z:-2}"},
  };
  testing::TempDictionary dictionary1 = testing::TempDictionary::makeTempDictionaryFromJson(&test_data1);
  boost::filesystem::path deleted_keys_file1{dictionary1.GetFileName()};
  deleted_keys_file1 += ".dk";
  {
    std::unordered_set<std::string> deleted_keys1{"abbc", "afgh"};
    std::ofstream out_stream(deleted_keys_file1.string(), std::ios::binary);
    msgpack::pack(out_stream, deleted_keys1);
  }

  std::vector<std::pair<std::string, std::string>> test_data2 = {
      {"abcd", "{g:15}"},   {"abbc", "{t:14}"}, {"abbcd", "{u:13}"}, {"abbd", "{v:12}"},
      {"abbdef", "{w:11}"}, {"abbe", "{x:10}"}, {"acdd", "{y:9}"},
  };
  testing::TempDictionary dictionary2 = testing::TempDictionary::makeTempDictionaryFromJson(&test_data2);
  boost::filesystem::path deleted_keys_file2{dictionary2.GetFileName()};
  deleted_keys_file2 += ".dk";
  {
    std::unordered_set<std::string> deleted_keys2{"abbc", "abbcd", "abbd"};
    std::ofstream out_stream(deleted_keys_file2.string(), std::ios::binary);
    msgpack::pack(out_stream, deleted_keys2);
  }

  std::vector<std::pair<std::string, std::string>> test_data3 = {
      {"abcd", "{g:25}"},   {"abbc", "{t:24}"}, {"abbcd", "{u:23}"},
      {"abbdef", "{w:21}"}, {"abbe", "{x:20}"}, {"acdd", "{y:19}"},
  };
  testing::TempDictionary dictionary3 = testing::TempDictionary::makeTempDictionaryFromJson(&test_data3);
  boost::filesystem::path deleted_keys_file3{dictionary3.GetFileName()};
  deleted_keys_file3 += ".dk";
  {
    std::unordered_set<std::string> deleted_keys3{"abbc", "abbcd", "abbdef"};
    std::ofstream out_stream(deleted_keys_file3.string(), std::ios::binary);
    msgpack::pack(out_stream, deleted_keys3);
  }

  JsonDictionaryMerger merger;
  merger.Add(dictionary1.GetFileName());
  merger.Add(dictionary2.GetFileName());
  merger.Add(dictionary3.GetFileName());

  std::string filename("merge-multiple-deletes-dict.kv");
  merger.Merge();

  merger.WriteToFile(filename);

  fsa::automata_t fsa(new fsa::Automata(filename.c_str()));
  dictionary_t d(new Dictionary(fsa));

  BOOST_CHECK(d->Contains("abcd"));
  BOOST_CHECK(!d->Contains("abbc"));
  BOOST_CHECK(!d->Contains("abbcd"));
  BOOST_CHECK(!d->Contains("abbd"));
  BOOST_CHECK(!d->Contains("abbdef"));
  BOOST_CHECK(d->Contains("abbe"));
  BOOST_CHECK(d->Contains("acdd"));
  BOOST_CHECK(!d->Contains("afgh"));

  BOOST_CHECK_EQUAL(8, merger.GetStats().deleted_keys_);
  BOOST_CHECK_EQUAL(13, merger.GetStats().updated_keys_);
  BOOST_CHECK_EQUAL(3, merger.GetStats().number_of_keys_);

  std::remove(filename.c_str());
  std::remove(deleted_keys_file1.string().c_str());
  std::remove(deleted_keys_file2.string().c_str());
  std::remove(deleted_keys_file3.string().c_str());
}

BOOST_AUTO_TEST_CASE(WriteWithoutMerge) {
  JsonDictionaryMerger merger;
  const std::string filename("write-without-merger.kv");

  BOOST_CHECK_THROW(merger.WriteToFile(filename), merger_exception);
  {
    std::ofstream out_stream(filename, std::ios::binary);
    BOOST_CHECK_THROW(merger.Write(out_stream), merger_exception);
  }
  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
}  // namespace keyvi
