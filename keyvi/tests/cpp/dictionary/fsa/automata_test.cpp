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
 * automata_test.cpp
 *
 *  Created on: Sep 20, 2015
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>
#include "dictionary/fsa/automata.h"
#include "dictionary/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE( AutomataTests )

BOOST_AUTO_TEST_CASE( GetOutGoingTransitions ) {
  std::vector<std::string> test_data =
        { "\01cd", "aaaa", "aabb", "agbc", "ajcd", "azcd" };
    testing::TempDictionary dictionary(test_data);
    automata_t f = dictionary.GetFsa();
  std::vector<uint32_t> outgoings;
  std::vector<unsigned char> symbols;

  f->GetOutGoingTransitions(f->GetStartState(), outgoings, symbols);

  BOOST_CHECK_EQUAL(2, outgoings.size());
  BOOST_CHECK_EQUAL(f->TryWalkTransition(f->GetStartState(), '\01'), outgoings[0]);
  BOOST_CHECK_EQUAL(f->TryWalkTransition(f->GetStartState(), 'a'), outgoings[1]);
  BOOST_CHECK_EQUAL('\01', symbols[0]);
  BOOST_CHECK_EQUAL('a', symbols[1]);

  // check all outgoings for 'a'
  uint32_t state_a = f->TryWalkTransition(f->GetStartState(), 'a');

  f->GetOutGoingTransitions(state_a, outgoings, symbols);

  BOOST_CHECK_EQUAL(4, outgoings.size());
  BOOST_CHECK_EQUAL(f->TryWalkTransition(state_a, 'a'), outgoings[0]);
  BOOST_CHECK_EQUAL(f->TryWalkTransition(state_a, 'g'), outgoings[1]);
  BOOST_CHECK_EQUAL(f->TryWalkTransition(state_a, 'j'), outgoings[2]);
  BOOST_CHECK_EQUAL(f->TryWalkTransition(state_a, 'z'), outgoings[3]);

  BOOST_CHECK_EQUAL('a', symbols[0]);
  BOOST_CHECK_EQUAL('g', symbols[1]);
  BOOST_CHECK_EQUAL('j', symbols[2]);
  BOOST_CHECK_EQUAL('z', symbols[3]);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */



