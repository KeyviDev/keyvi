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

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
