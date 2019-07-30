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
 * codepoint_state_traverser_test.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/codepoint_state_traverser.h"
#include "keyvi/dictionary/fsa/state_traverser.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE(CodePointStateTraverserTests)

BOOST_AUTO_TEST_CASE(someASCIITraversalNoPrune) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  CodePointStateTraverser<StateTraverser<>> c(f);

  BOOST_CHECK_EQUAL('a', c.GetStateLabel());
  BOOST_CHECK_EQUAL(1, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('a', c.GetStateLabel());
  BOOST_CHECK_EQUAL(2, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('a', c.GetStateLabel());
  BOOST_CHECK_EQUAL(3, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('a', c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('b', c.GetStateLabel());
  BOOST_CHECK_EQUAL(3, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('b', c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('c', c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('c', c.GetStateLabel());
  BOOST_CHECK_EQUAL(3, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('d', c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('b', c.GetStateLabel());
  BOOST_CHECK_EQUAL(1, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('b', c.GetStateLabel());
  BOOST_CHECK_EQUAL(2, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('c', c.GetStateLabel());
  BOOST_CHECK_EQUAL(3, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('d', c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());

  // traverser shall be exhausted
  c++;
  BOOST_CHECK_EQUAL(0, c.GetStateLabel());
  BOOST_CHECK_EQUAL(0, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0, c.GetStateLabel());
  BOOST_CHECK_EQUAL(0, c.GetDepth());
}

BOOST_AUTO_TEST_CASE(someNonASCIITraversalNoPrune) {
  std::vector<std::string> test_data = {"aüöß", "öäöö", "öäüaöc", "条件指定", "🤓🤘"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  CodePointStateTraverser<StateTraverser<>> c(f);

  BOOST_CHECK_EQUAL('a', c.GetStateLabel());
  BOOST_CHECK_EQUAL(1, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xfc /*ü*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(2, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xf6 /*ö*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(3, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xdf /*'ß'*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xf6 /*'ö'*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(1, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xe4 /*'ä'*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(2, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xf6 /*ö*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(3, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xf6 /*ö*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xfc /*ü*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(3, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('a', c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0xf6 /*ö*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(5, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL('c', c.GetStateLabel());
  BOOST_CHECK_EQUAL(6, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0x6761 /*条*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(1, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0x4ef6 /*件*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(2, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0x6307 /*指*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(3, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0x5b9a /*定*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(4, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0x1f913 /*🤓*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(1, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0x1f918 /*🤘*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(2, c.GetDepth());

  // traverser shall be exhausted
  c++;
  BOOST_CHECK_EQUAL(0, c.GetStateLabel());
  BOOST_CHECK_EQUAL(0, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0, c.GetStateLabel());
  BOOST_CHECK_EQUAL(0, c.GetDepth());
}

BOOST_AUTO_TEST_CASE(someNonASCIITraversalPrune) {
  std::vector<std::string> test_data = {"条件指定", "🤓🤘"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  CodePointStateTraverser<StateTraverser<>> c(f);

  BOOST_CHECK_EQUAL(0x6761 /*条*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(1, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0x4ef6 /*件*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(2, c.GetDepth());

  c.Prune();
  c++;
  BOOST_CHECK_EQUAL(0x1f913 /*🤓*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(1, c.GetDepth());
  c++;
  BOOST_CHECK_EQUAL(0x1f918 /*🤘*/, c.GetStateLabel());
  BOOST_CHECK_EQUAL(2, c.GetDepth());

  // traverser shall be exhausted
  c++;
  BOOST_CHECK_EQUAL(0, c.GetStateLabel());
  BOOST_CHECK_EQUAL(0, c.GetDepth());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
