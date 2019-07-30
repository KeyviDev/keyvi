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
 * state_traverser_test.cpp
 *
 *  Created on: Jun 3, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/state_traverser.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE(StateTraverserTests)

BOOST_AUTO_TEST_CASE(someTraversalNoPrune) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  StateTraverser<> s(f);

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
}

BOOST_AUTO_TEST_CASE(someTraversalWithPrune) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  StateTraverser<> s(f);

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

BOOST_AUTO_TEST_CASE(longkeys) {
  std::string a(1000, 'a');
  std::string b(1000, 'b');

  std::vector<std::string> test_data = {a, b};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  StateTraverser<> s(f);

  for (int i = 1; i <= 1000; ++i) {
    BOOST_CHECK_EQUAL('a', s.GetStateLabel());
    BOOST_CHECK_EQUAL(i, s.GetDepth());
    s++;
    BOOST_CHECK(!s.AtEnd());
  }

  for (int i = 1; i <= 1000; ++i) {
    BOOST_CHECK_EQUAL('b', s.GetStateLabel());
    BOOST_CHECK_EQUAL(i, s.GetDepth());
    s++;
    if (i != 1000) {
      BOOST_CHECK(!s.AtEnd());
    } else {
      BOOST_CHECK(s.AtEnd());
    }
  }

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
  std::vector<std::string> test_data = {std::string("\0aaaa", 5), std::string("aa\0bb", 5), "aabc", "aacd",
                                        std::string("bbcd\0", 5)};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  StateTraverser<> s(f);

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

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
