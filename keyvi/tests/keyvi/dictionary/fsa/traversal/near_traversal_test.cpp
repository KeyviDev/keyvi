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
 * near_traversal.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: hendrik
 */

#include <utility>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/state_traverser.h"
#include "keyvi/dictionary/fsa/traversal/near_traversal.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE(NearTraversalTests)

BOOST_AUTO_TEST_CASE(someTraversalNoPrune) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  std::shared_ptr<std::string> near_key = std::make_shared<std::string>("aace");
  auto payload = traversal::TraversalPayload<traversal::NearTransition>(near_key);

  StateTraverser<traversal::NearTransition> s(f, f->GetStartState(), std::move(payload));

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('h', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

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

  s++;
  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('e', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('g', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  BOOST_CHECK(!s.IsFinalState());
  s++;
  BOOST_CHECK_EQUAL('h', s.GetStateLabel());
  BOOST_CHECK_EQUAL(6, s.GetDepth());
  BOOST_CHECK(s.IsFinalState());

  // traverser shall be exhausted
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK_EQUAL(0, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
  BOOST_CHECK_EQUAL(0, s.GetDepth());
}

BOOST_AUTO_TEST_CASE(nonAscii) {
  std::vector<std::string> test_data = {"aaaa", "aa\xa0\x62\x62", "aa\xff\x62\x63"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  std::shared_ptr<std::string> near_key = std::make_shared<std::string>("aa\xa0\x63\x64");
  auto payload = traversal::TraversalPayload<traversal::NearTransition>(near_key);

  StateTraverser<traversal::NearTransition> s(f, f->GetStartState(), std::move(payload));

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
