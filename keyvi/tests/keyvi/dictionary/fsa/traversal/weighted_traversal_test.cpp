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
 *  Created on: Sep 23, 2025
 *      Author: hendrik
 */

#include <utility>

#include <boost/test/unit_test.hpp>
#include "keyvi/dictionary/fsa/traversal/weighted_traversal.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace traversal {

BOOST_AUTO_TEST_SUITE(WeightedTraversalTests)

BOOST_AUTO_TEST_CASE(PostProcessStableSortEqualWeights) {
  TraversalStack<WeightedTransition> traversal_stack;

  uint64_t s = 0;
  unsigned char c = 97;

  // add states with same weights
  for (; c < 123; ++c, ++s) {
    traversal_stack.traversal_states[0].Add(s, 24, c, &traversal_stack.traversal_stack_payload);
  }

  // sort by weight (all the same)
  traversal_stack.traversal_states[0].PostProcess(&traversal_stack.traversal_stack_payload);

  // assert that sort did not change the original order (== stable sort)
  c = 97;
  for (const auto state : traversal_stack.traversal_states[0].traversal_state_payload.transitions) {
    BOOST_CHECK_EQUAL(c++, state.label);
  }

  BOOST_CHECK_EQUAL(c, 123);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace traversal*/
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
