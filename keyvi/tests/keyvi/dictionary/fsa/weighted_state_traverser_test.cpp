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
 * weighted_state_traverser_test.cpp
 *
 *  Created on: Jun 3, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/testing/temp_dictionary.h"

#define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE(WeightedStateTraverserTests)

BOOST_AUTO_TEST_CASE(minimizationoninnerWeights) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"aabc", 22}, {"bbbc", 22}, {"bbbd", 444}, {"cdabc", 22}, {"efdffd", 444}, {"xfdebc", 23},
  };

  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  WeightedStateTraverser s(f);

  // we should get 'b' first
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());

  uint32_t state_id_d_444 = s.GetStateId();
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  uint32_t state_id_c_22 = s.GetStateId();
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

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('f', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(6, s.GetDepth());

  // check minimization
  BOOST_CHECK_EQUAL(state_id_d_444, s.GetStateId());
  s++;

  BOOST_CHECK_EQUAL('x', s.GetStateLabel());
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

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(6, s.GetDepth());
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

  // check minimization
  BOOST_CHECK_EQUAL(state_id_c_22, s.GetStateId());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('d', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('a', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(4, s.GetDepth());
  s++;

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(5, s.GetDepth());

  s++;

  // traverser at the end
  BOOST_CHECK_EQUAL(0, s.GetStateLabel());
}

BOOST_AUTO_TEST_CASE(weightedTraversal) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"aabc", 412}, {"aabde", 22}, {"cdbde", 444}, {"cdef", 34}, {"cdzzz", 56}, {"efde", 23},
  };

  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  WeightedStateTraverser s(f);

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

BOOST_AUTO_TEST_CASE(weightedTraversalWithPrune) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"aabc", 412}, {"aabde", 22}, {"cdbde", 444}, {"cdef", 34}, {"cdzzz", 56}, {"efde", 23},
  };

  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  WeightedStateTraverser s(f);
  // WeightedStateTraverser end;

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

  // BOOST_CHECK(s==end);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
