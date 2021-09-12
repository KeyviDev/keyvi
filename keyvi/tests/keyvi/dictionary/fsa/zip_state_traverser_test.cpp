//
// keyvi - A key value store.
//
// Copyright 2020 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * zip_state_traverser_test.cpp
 *
 *  Created on: Dev 18, 2020
 *      Author: hendrik
 */

#include <memory>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/state_traverser.h"
#include "keyvi/dictionary/fsa/zip_state_traverser.h"
#include "keyvi/testing/temp_dictionary.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE(ZipStateTraverserTests)

BOOST_AUTO_TEST_CASE(StateTraverserCompatibility) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  ZipStateTraverser<StateTraverser<>> s({f});

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  BOOST_CHECK(!s.AtEnd());
  BOOST_CHECK(s);

  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  // traverser shall be exhausted
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK(s.AtEnd());
  BOOST_CHECK(!s);
  BOOST_CHECK_EQUAL(0, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK(s.AtEnd());
  BOOST_CHECK_EQUAL(0, s.GetDepth());

  // with advance == false
  ZipStateTraverser<StateTraverser<>> s2({f}, false);
  s2++;

  BOOST_CHECK_EQUAL('a', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s2.GetDepth());
  BOOST_CHECK(!s2.AtEnd());
  BOOST_CHECK(s2);

  s2++;
  BOOST_CHECK_EQUAL('a', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s2.GetDepth());
  s2++;
  BOOST_CHECK_EQUAL('a', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s2.GetDepth());
  BOOST_CHECK(!s2.IsFinalState());

  s2++;
  BOOST_CHECK_EQUAL('a', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s2.GetDepth());
  BOOST_CHECK(s2.IsFinalState());

  s2++;
  BOOST_CHECK_EQUAL('b', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s2.GetDepth());
  BOOST_CHECK(!s2.IsFinalState());

  s2++;
  BOOST_CHECK_EQUAL('b', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s2.GetDepth());
  BOOST_CHECK(s2.IsFinalState());

  s2++;
  BOOST_CHECK_EQUAL('c', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s2.GetDepth());
  BOOST_CHECK(s2.IsFinalState());

  s2++;
  BOOST_CHECK_EQUAL('c', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s2.GetDepth());
  BOOST_CHECK(!s2.IsFinalState());

  s2++;
  BOOST_CHECK_EQUAL('d', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s2.GetDepth());
  BOOST_CHECK(s2.IsFinalState());

  s2++;
  BOOST_CHECK_EQUAL('b', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s2.GetDepth());
  BOOST_CHECK(!s2.IsFinalState());

  s2++;
  BOOST_CHECK_EQUAL('b', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s2.GetDepth());
  s2++;
  BOOST_CHECK_EQUAL('c', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s2.GetDepth());
  s2++;
  BOOST_CHECK_EQUAL('d', s2.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s2.GetDepth());
  BOOST_CHECK(s2.IsFinalState());

  // traverser shall be exhausted
  s2++;
  BOOST_CHECK_EQUAL(0, s2.GetStateLabel());
  BOOST_CHECK(s2.AtEnd());
  BOOST_CHECK(!s2);
  BOOST_CHECK_EQUAL(0, s2.GetDepth());
  s2++;
  BOOST_CHECK_EQUAL(0, s2.GetStateLabel());
  BOOST_CHECK(s2.AtEnd());
  BOOST_CHECK_EQUAL(0, s2.GetDepth());

  std::vector<std::string> test_data2 = {"aaaa", "aabb", "bbcd"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();
  std::vector<std::string> test_data3 = {"aabc", "aacd"};
  testing::TempDictionary dictionary3(&test_data3);
  automata_t f3 = dictionary3.GetFsa();

  // split in 2 dicts and with advance==false
  ZipStateTraverser<StateTraverser<>> s3({f2, f3}, false);
  s3++;

  BOOST_CHECK_EQUAL('a', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s3.GetDepth());
  BOOST_CHECK(!s3.AtEnd());
  BOOST_CHECK(s3);

  s3++;
  BOOST_CHECK_EQUAL('a', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s3.GetDepth());
  s3++;
  BOOST_CHECK_EQUAL('a', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s3.GetDepth());
  BOOST_CHECK(!s3.IsFinalState());

  s3++;
  BOOST_CHECK_EQUAL('a', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s3.GetDepth());
  BOOST_CHECK(s3.IsFinalState());

  s3++;
  BOOST_CHECK_EQUAL('b', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s3.GetDepth());
  BOOST_CHECK(!s3.IsFinalState());

  s3++;
  BOOST_CHECK_EQUAL('b', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s3.GetDepth());
  BOOST_CHECK(s3.IsFinalState());

  s3++;
  BOOST_CHECK_EQUAL('c', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s3.GetDepth());
  BOOST_CHECK(s3.IsFinalState());

  s3++;
  BOOST_CHECK_EQUAL('c', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s3.GetDepth());
  BOOST_CHECK(!s3.IsFinalState());

  s3++;
  BOOST_CHECK_EQUAL('d', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s3.GetDepth());
  BOOST_CHECK(s3.IsFinalState());

  s3++;
  BOOST_CHECK_EQUAL('b', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s3.GetDepth());
  BOOST_CHECK(!s3.IsFinalState());

  s3++;
  BOOST_CHECK_EQUAL('b', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s3.GetDepth());
  s3++;
  BOOST_CHECK_EQUAL('c', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s3.GetDepth());
  s3++;
  BOOST_CHECK_EQUAL('d', s3.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s3.GetDepth());
  BOOST_CHECK(s3.IsFinalState());

  // traverser shall be exhausted
  s3++;
  BOOST_CHECK_EQUAL(0, s3.GetStateLabel());
  BOOST_CHECK(s3.AtEnd());
  BOOST_CHECK(!s3);
  BOOST_CHECK_EQUAL(0, s3.GetDepth());
  s3++;
  BOOST_CHECK_EQUAL(0, s3.GetStateLabel());
  BOOST_CHECK(s3.AtEnd());
  BOOST_CHECK_EQUAL(0, s3.GetDepth());
}

BOOST_AUTO_TEST_CASE(someTraversalWithPrune) {
  std::vector<std::string> test_data1 = {"aaaa", "aabb", "bbcd"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();
  std::vector<std::string> test_data2 = {"aabc", "aacd"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ZipStateTraverser<StateTraverser<>> s({f1, f2});

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  BOOST_CHECK(!s.AtEnd());

  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());

  s.Prune();
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());

  s.Prune();
  s++;
  BOOST_CHECK(!s.AtEnd());

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());

  s.Prune();
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  s.Prune();
  s++;

  // traverser shall be exhausted
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK(s.AtEnd());
  BOOST_CHECK_EQUAL(0, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK(s.AtEnd());
  BOOST_CHECK_EQUAL(0, s.GetDepth());
}

BOOST_AUTO_TEST_CASE(zeroByte) {
  std::vector<std::string> test_data1 = {
      std::string("\0aaaa", 5),
      "aabc",
  };
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();
  std::vector<std::string> test_data2 = {std::string("aa\0bb", 5), "aacd", std::string("bbcd\0", 5)};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ZipStateTraverser<StateTraverser<>> s({f1, f2});

  BOOST_CHECK_EQUAL('\0', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  BOOST_CHECK(!s.AtEnd());

  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('\0', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());

  s++;
  BOOST_CHECK_EQUAL('\0', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  // traverser shall be exhausted
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK_EQUAL(0, s.GetDepth());
  BOOST_CHECK(s.AtEnd());

  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK_EQUAL(0, s.GetDepth());
  BOOST_CHECK(s.AtEnd());
}

BOOST_AUTO_TEST_CASE(basic) {
  std::vector<std::string> test_data1 = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"aaab", "aabb", "aabd", "abcd", "bbcd"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ZipStateTraverser<StateTraverser<>> t({f1, f2});

  BOOST_CHECK_EQUAL('a', t.GetStateLabel());
  BOOST_CHECK_EQUAL(1, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('a', t.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('a', t.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('a', t.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('b', t.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('b', t.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('b', t.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('c', t.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('d', t.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('c', t.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('d', t.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('b', t.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('c', t.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('d', t.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('b', t.GetStateLabel());
  BOOST_CHECK_EQUAL(1, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('b', t.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('c', t.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t.GetDepth());
  t++;
  BOOST_CHECK_EQUAL('d', t.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t.GetDepth());
  t++;
  BOOST_CHECK(!t);
}

std::vector<std::string> GetAllKeys(ZipStateTraverser<StateTraverser<>> *zip_traverser) {
  std::vector<unsigned char> label_stack;
  std::vector<std::string> keys;

  while (*zip_traverser) {
    label_stack.resize(zip_traverser->GetDepth() - 1);
    label_stack.push_back(zip_traverser->GetStateLabel());
    if (zip_traverser->IsFinalState()) {
      keys.emplace_back(label_stack.begin(), label_stack.end());
    }

    zip_traverser->operator++(0);
  }

  return keys;
}

BOOST_AUTO_TEST_CASE(append) {
  std::vector<std::string> test_data1 = {"aa", "bb", "cc", "dd", "ee"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"ff", "gg", "hh", "zz"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ZipStateTraverser<StateTraverser<>> t({f1, f2});

  auto actual = GetAllKeys(&t);
  std::vector<std::string> expected{"aa", "bb", "cc", "dd", "ee", "ff", "gg", "hh", "zz"};

  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());

  ZipStateTraverser<StateTraverser<>> t2({f2, f1});

  actual = GetAllKeys(&t2);
  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(mixed) {
  std::vector<std::string> test_data1 = {"aa", "hh", "ii", "kk", "zz"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"bb", "ff", "hh", "jj"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ZipStateTraverser<StateTraverser<>> t({f1, f2});

  auto actual = GetAllKeys(&t);
  std::vector<std::string> expected{"aa", "bb", "ff", "hh", "ii", "jj", "kk", "zz"};

  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());

  ZipStateTraverser<StateTraverser<>> t2({f2, f1});

  actual = GetAllKeys(&t2);
  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(mixed2) {
  std::vector<std::string> test_data1 = {"aa", "hh", "ii", "kk", "zz"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();
  std::vector<std::string> test_data2 = {"bb", "ff", "hh", "jj"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();
  std::vector<std::string> test_data3 = {"dd", "ee", "jj", "pp", "zz"};
  testing::TempDictionary dictionary3(&test_data3);
  automata_t f3 = dictionary3.GetFsa();
  std::vector<std::string> test_data4 = {"cc", "qq", "rr", "tt"};
  testing::TempDictionary dictionary4(&test_data4);
  automata_t f4 = dictionary4.GetFsa();
  std::vector<std::string> test_data5 = {"aa", "ee", "pp", "zz"};
  testing::TempDictionary dictionary5(&test_data5);
  automata_t f5 = dictionary5.GetFsa();
  std::vector<std::string> test_data6 = {"jj"};
  testing::TempDictionary dictionary6(&test_data6);
  automata_t f6 = dictionary6.GetFsa();

  ZipStateTraverser<StateTraverser<>> t({f1, f2, f3, f4, f5, f6});

  auto actual = GetAllKeys(&t);
  std::vector<std::string> expected{"aa", "bb", "cc", "dd", "ee", "ff", "hh", "ii",
                                    "jj", "kk", "pp", "qq", "rr", "tt", "zz"};

  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());

  ZipStateTraverser<StateTraverser<>> t2({f2, f1, f5, f6, f4, f3});

  actual = GetAllKeys(&t2);
  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(infixes) {
  std::vector<std::string> test_data1 = {"aa", "bbb", "c", "zz"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();
  std::vector<std::string> test_data2 = {"aaa", "bb", "hh", "jj"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();
  std::vector<std::string> test_data3 = {"aaaa", "b", "ccc", "z"};
  testing::TempDictionary dictionary3(&test_data3);
  automata_t f3 = dictionary3.GetFsa();

  ZipStateTraverser<StateTraverser<>> t({f1, f2, f3});

  auto actual = GetAllKeys(&t);
  std::vector<std::string> expected{"aa", "aaa", "aaaa", "b", "bb", "bbb", "c", "ccc", "hh", "jj", "z", "zz"};

  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());

  ZipStateTraverser<StateTraverser<>> t2({f3, f2, f1});
  actual = GetAllKeys(&t2);
  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());

  ZipStateTraverser<StateTraverser<>> t3({f3, f2, f1});
  actual = GetAllKeys(&t3);
  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());

  ZipStateTraverser<StateTraverser<>> t4({f3, f2, f1});
  actual = GetAllKeys(&t4);
  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(after_prefix) {
  std::vector<std::string> test_data1 = {"aa", "hh", "ii", "kk", "zz"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();
  std::vector<std::string> test_data2 = {"bb", "ff", "hh", "jj"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();
  std::vector<std::string> test_data3 = {"add", "ee", "jj", "pp", "zz"};
  testing::TempDictionary dictionary3(&test_data3);
  automata_t f3 = dictionary3.GetFsa();
  std::vector<std::string> test_data4 = {"abcc", "aqq", "rr", "tt"};
  testing::TempDictionary dictionary4(&test_data4);
  automata_t f4 = dictionary4.GetFsa();
  std::vector<std::string> test_data5 = {"aaaaaa", "aee", "pp", "zz"};
  testing::TempDictionary dictionary5(&test_data5);
  automata_t f5 = dictionary5.GetFsa();
  std::vector<std::string> test_data6 = {"acjj"};
  testing::TempDictionary dictionary6(&test_data6);
  automata_t f6 = dictionary6.GetFsa();
  std::vector<std::string> test_data7 = {"a"};
  testing::TempDictionary dictionary7(&test_data7);
  automata_t f7 = dictionary7.GetFsa();

  ZipStateTraverser<StateTraverser<>> t({{f1, f1->TryWalkTransition(f1->GetStartState(), 'a')},
                                         {f2, f2->TryWalkTransition(f2->GetStartState(), 'a')},
                                         {f3, f3->TryWalkTransition(f3->GetStartState(), 'a')},
                                         {f4, f4->TryWalkTransition(f4->GetStartState(), 'a')},
                                         {f5, f5->TryWalkTransition(f5->GetStartState(), 'a')},
                                         {f6, f6->TryWalkTransition(f6->GetStartState(), 'a')},
                                         {f7, f7->TryWalkTransition(f7->GetStartState(), 'a')}});

  auto actual = GetAllKeys(&t);
  std::vector<std::string> expected{"a", "aaaaa", "bcc", "cjj", "dd", "ee", "qq"};

  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());
  ZipStateTraverser<StateTraverser<>> t2({{f1, f1->TryWalkTransition(f1->GetStartState(), 'h')},
                                          {f2, f2->TryWalkTransition(f2->GetStartState(), 'h')},
                                          {f3, f3->TryWalkTransition(f3->GetStartState(), 'h')},
                                          {f4, f4->TryWalkTransition(f4->GetStartState(), 'h')},
                                          {f5, f5->TryWalkTransition(f5->GetStartState(), 'h')},
                                          {f6, f6->TryWalkTransition(f6->GetStartState(), 'h')}});

  auto actual2 = GetAllKeys(&t2);
  std::vector<std::string> expected2{"h"};
  BOOST_CHECK_EQUAL_COLLECTIONS(actual2.begin(), actual2.end(), expected2.begin(), expected2.end());
}

BOOST_AUTO_TEST_CASE(weightedTraversal) {
  std::vector<std::pair<std::string, uint32_t>> test_data1 = {
      {"aabc", 412},
      {"aabde", 22},
      {"efde", 24},
  };
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"cdbde", 444},
      {"cdef", 34},
      {"cdzzz", 56},
      {"efde", 10},
  };
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ZipStateTraverser<WeightedStateTraverser> s({f1, f2});

  // we should get 'c' first
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  // traverser at the end
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
}

BOOST_AUTO_TEST_CASE(weightedTraversal2) {
  std::vector<std::pair<std::string, uint32_t>> test_data1 = {
      {"aabc", 412},
      {"aabde", 22},
      {"efde", 24},
  };
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"cdbde", 444},
      {"cdef", 34},
      {"cdzzz", 56},
      {"efde", 10},
  };
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data3 = {
      {"xyz", 99},
      {"efde", 18},
  };
  testing::TempDictionary dictionary3(&test_data3);
  automata_t f3 = dictionary3.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data4 = {
      {"pag", 2},
      {"efde", 10},
  };
  testing::TempDictionary dictionary4(&test_data4);
  automata_t f4 = dictionary4.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data5 = {
      {"efac", 21},
  };
  testing::TempDictionary dictionary5(&test_data5);
  automata_t f5 = dictionary5.GetFsa();

  ZipStateTraverser<WeightedStateTraverser> s({f1, f2, f3, f4, f5});

  // we should get 'c' first
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('x', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('y', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('p', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('g', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  // traverser at the end
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
}

BOOST_AUTO_TEST_CASE(weightedTraversal3) {
  std::vector<std::pair<std::string, uint32_t>> test_data1 = {
      {"aabc", 42},
      {"aabde", 42},
      {"efde", 42},
  };
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"cdbde", 42},
      {"cdef", 42},
      {"cdzzz", 42},
      {"efde", 10},
  };
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ZipStateTraverser<WeightedStateTraverser> s({f1, f2});

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  // traverser at the end
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
}

BOOST_AUTO_TEST_CASE(weightedTraversal4) {
  std::vector<std::pair<std::string, uint32_t>> test_data1 = {
      {"aabc", 42},
      {"aabde", 42},
      {"efde", 42},
  };
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"aabc", 99},
      {"aabde", 99},
      {"efde", 99},
  };
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ZipStateTraverser<WeightedStateTraverser> s({f1, f2});

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  // traverser at the end
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
}

BOOST_AUTO_TEST_CASE(weightedTraversal_with_prune) {
  std::vector<std::pair<std::string, uint32_t>> test_data1 = {
      {"aabc", 412},
      {"aabde", 22},
      {"efde", 24},
  };
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data2 = {
      {"cdbde", 444},
      {"cdef", 10},
      {"cdzzz", 56},
      {"efde", 5},
  };
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data3 = {
      {"cdbde", 333},
      {"cdef", 34},
      {"cdzzz", 15},
      {"efde", 10},
  };
  testing::TempDictionary dictionary3(&test_data3);
  automata_t f3 = dictionary3.GetFsa();

  std::vector<std::pair<std::string, uint32_t>> test_data4 = {
      {"aabc", 1}, {"aabde", 2}, {"cdbde", 3}, {"cdef", 4}, {"cdzzz", 5}, {"efde", 6},
  };
  testing::TempDictionary dictionary4(&test_data4);
  automata_t f4 = dictionary4.GetFsa();

  ZipStateTraverser<WeightedStateTraverser> s({f1, f2, f3, f4});

  // we should get 'c' first
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('z', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  s.Prune();
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  s.Prune();
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  s.Prune();
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  s.Prune();
  s++;
}

BOOST_AUTO_TEST_CASE(nearTraversal) {
  std::vector<std::string> test_data1 = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "cdefgh"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"abbb", "aabc", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  std::shared_ptr<std::string> near_key = std::make_shared<std::string>("aace");

  auto payload1 = traversal::TraversalPayload<traversal::NearTransition>(near_key);
  auto payload2 = traversal::TraversalPayload<traversal::NearTransition>(near_key);

  std::vector<std::tuple<automata_t, uint64_t, traversal::TraversalPayload<traversal::NearTransition>>> v;

  v.emplace_back(f1, f1->GetStartState(), std::move(payload1));
  v.emplace_back(f2, f2->GetStartState(), std::move(payload2));

  ZipStateTraverser<NearStateTraverser> s(std::move(v));
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('h', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;
}

BOOST_AUTO_TEST_CASE(nonAscii) {
  std::vector<std::string> test_data1 = {"aaaa", "aa\xa0\x62\x62"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"abbb", "aa\xff\x62\x63"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  std::shared_ptr<std::string> near_key = std::make_shared<std::string>("aa\xa0\x63\x64");

  auto payload1 = traversal::TraversalPayload<traversal::NearTransition>(near_key);
  auto payload2 = traversal::TraversalPayload<traversal::NearTransition>(near_key);

  std::vector<std::tuple<automata_t, uint64_t, traversal::TraversalPayload<traversal::NearTransition>>> v;

  v.emplace_back(f1, f1->GetStartState(), std::move(payload1));
  v.emplace_back(f2, f2->GetStartState(), std::move(payload2));

  ZipStateTraverser<NearStateTraverser> s(std::move(v));

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL(0xa0, s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL(0xff, s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  // traverser shall be exhausted
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK_EQUAL(0, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK_EQUAL(0, s.GetDepth());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
