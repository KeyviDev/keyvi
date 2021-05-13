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

#include <memory>
#include <utility>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/comparable_state_traverser.h"
#include "keyvi/dictionary/fsa/state_traverser.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE(ComparableStateTraverserTests)

BOOST_AUTO_TEST_CASE(StateTraverserCompatibility) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  ComparableStateTraverser<StateTraverser<>> s(f);

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

BOOST_AUTO_TEST_CASE(StateTraverserCompatSomeTraversalWithPrune) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  ComparableStateTraverser<StateTraverser<>> s(f);

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
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  BOOST_CHECK_EQUAL(2, s.GetStateLabels().size());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());

  s.Prune();
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  BOOST_CHECK_EQUAL(2, s.GetStateLabels().size());
  s++;
  BOOST_CHECK(!s.AtEnd());

  BOOST_CHECK_EQUAL('c', s.GetStateLabel());
  BOOST_CHECK_EQUAL(3, s.GetDepth());

  s.Prune();
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  BOOST_CHECK_EQUAL(2, s.GetStateLabels().size());
  s++;

  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(1, s.GetDepth());
  s++;
  BOOST_CHECK_EQUAL('b', s.GetStateLabel());
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  s++;

  s.Prune();
  BOOST_CHECK_EQUAL(2, s.GetDepth());
  BOOST_CHECK_EQUAL(2, s.GetStateLabels().size());
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

BOOST_AUTO_TEST_CASE(StateTraverserCompatLongkeys) {
  std::string a(1000, 'a');
  std::string b(1000, 'b');

  std::vector<std::string> test_data = {a, b};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  ComparableStateTraverser<StateTraverser<>> s(f);

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

BOOST_AUTO_TEST_CASE(StateTraverserCompatZeroByte) {
  std::vector<std::string> test_data = {std::string("\0aaaa", 5), std::string("aa\0bb", 5), "aabc", "aacd",
                                        std::string("bbcd\0", 5)};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  ComparableStateTraverser<StateTraverser<>> s(f);

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

BOOST_AUTO_TEST_CASE(same_data) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  ComparableStateTraverser<StateTraverser<>> t1(f, false, 0);
  ComparableStateTraverser<StateTraverser<>> t2(f, false, 0);
  ComparableStateTraverser<StateTraverser<>> t3(f, false, 1);
  ComparableStateTraverser<StateTraverser<>> t4(f, false, 1);

  BOOST_CHECK((t1 < t2) == false);
  BOOST_CHECK((t2 < t1) == false);
  BOOST_CHECK(t2 == t1);
  BOOST_CHECK(t2 <= t1);
  BOOST_CHECK(t1 <= t2);
  BOOST_CHECK(t3 < t1);
  BOOST_CHECK(t4 < t1);
  BOOST_CHECK((t3 < t4) == false);

  t1++;
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(1, t1.GetDepth());
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK((t1 < t2) == false);
  BOOST_CHECK(t1 != t2);
  BOOST_CHECK(t3 < t1);
  BOOST_CHECK(t4 < t1);
  BOOST_CHECK((t1 < t3) == false);
  BOOST_CHECK((t1 < t4) == false);

  t3++;
  BOOST_CHECK(t3 < t1);
  BOOST_CHECK((t3 > t1) == false);
  BOOST_CHECK(t4 < t3);

  t2++;
  BOOST_CHECK_EQUAL('a', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(1, t2.GetDepth());
  BOOST_CHECK((t1 < t2) == false);
  BOOST_CHECK((t2 < t1) == false);
  BOOST_CHECK(t3 < t1);
  BOOST_CHECK(t4 < t1);

  t2++;
  BOOST_CHECK_EQUAL('a', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t2.GetDepth());
  BOOST_CHECK((t2 < t1) == false);
  BOOST_CHECK(t1 < t2);
}

BOOST_AUTO_TEST_CASE(interleave1) {
  std::vector<std::string> test_data1 = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"aabb", "aabd", "abcd", "bbcd"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ComparableStateTraverser<StateTraverser<>> t1(f1, false, 0);
  ComparableStateTraverser<StateTraverser<>> t2(f2, false, 1);

  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL(t1, t2);
  t1++;
  t2++;
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(1, t1.GetDepth());
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL(t1, t2);
  t1++;
  t2++;
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t1.GetDepth());
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL(t1, t2);
  BOOST_CHECK_EQUAL(2, t2.GetDepth());
  t1++;
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t1.GetDepth());
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK(t1 != t2);
  t1++;
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t1.GetDepth());
  t1++;
  BOOST_CHECK_EQUAL('b', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t1.GetDepth());
  t2++;
  BOOST_CHECK_EQUAL('b', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t2.GetDepth());
  BOOST_CHECK(t1 == t2);
}

BOOST_AUTO_TEST_CASE(interleave2) {
  std::vector<std::string> test_data1 = {"aaaa", "aabb", "aabc", "aacd", "bbcd"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"aaab", "aabd", "abcd", "bbcd"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  ComparableStateTraverser<StateTraverser<>> t1(f1, false, 0);
  ComparableStateTraverser<StateTraverser<>> t2(f2, false, 1);

  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL(t1, t2);
  t1++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK(t1 != t2);
  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL(t1, t2);
  t2++;
  BOOST_CHECK(t1 < t2);
  BOOST_CHECK(t1 != t2);
  t1++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL(t1, t2);
  BOOST_CHECK_EQUAL(2, t2.GetDepth());
  t1++;
  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL(t1, t2);
  BOOST_CHECK_EQUAL(3, t2.GetDepth());
}

BOOST_AUTO_TEST_CASE(nearTraversalSpecialization) {
  std::vector<std::string> test_data1 = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "cdefgh"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"abbb", "aabc", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  std::shared_ptr<std::string> near_key = std::make_shared<std::string>("aace");

  auto payload1 = traversal::TraversalPayload<traversal::NearTransition>(near_key);
  auto payload2 = traversal::TraversalPayload<traversal::NearTransition>(near_key);

  ComparableStateTraverser<NearStateTraverser> t1(f1, f1->GetStartState(), std::move(payload1), true, 0);
  ComparableStateTraverser<NearStateTraverser> t2(f2, f2->GetStartState(), std::move(payload2), true, 1);

  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(1, t1.GetDepth());
  t1++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK(t1 != t2);
  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t1.GetDepth());
  BOOST_CHECK_EQUAL('a', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t2.GetDepth());
  BOOST_CHECK(t1 == t2);
  t1++;
  BOOST_CHECK(t2 < t1);
  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('c', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t1.GetDepth());
  BOOST_CHECK(!t1.IsFinalState());
  BOOST_CHECK_EQUAL('c', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t2.GetDepth());
  BOOST_CHECK(!t2.IsFinalState());
  BOOST_CHECK(t1 != t2);

  // inc to aacd (not exact)
  t1++;
  BOOST_CHECK(t2 < t1);
  // inc to aace (exact match)
  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('d', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t1.GetDepth());
  BOOST_CHECK(t1.IsFinalState());
  BOOST_CHECK_EQUAL('e', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t2.GetDepth());
  BOOST_CHECK(!t2.IsFinalState());
  BOOST_CHECK(t1 != t2);

  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('h', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(5, t2.GetDepth());
  BOOST_CHECK(t2.IsFinalState());

  t2++;
  // order change: t2 because exact matching for t2 finished
  BOOST_CHECK(t2 > t1);
  BOOST_CHECK_EQUAL('b', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t2.GetDepth());
  BOOST_CHECK(!t2.IsFinalState());

  t1++;
  BOOST_CHECK(t2 > t1);
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t1.GetDepth());
  BOOST_CHECK(!t1.IsFinalState());

  t1++;
  BOOST_CHECK(t2 > t1);
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t1.GetDepth());
  BOOST_CHECK(t1.IsFinalState());

  t1++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('b', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t1.GetDepth());
  BOOST_CHECK(!t1.IsFinalState());
}

BOOST_AUTO_TEST_CASE(nearTraversalSpecialization2) {
  std::vector<std::string> test_data1 = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "cdefgh"};
  testing::TempDictionary dictionary1(&test_data1);
  automata_t f1 = dictionary1.GetFsa();

  std::vector<std::string> test_data2 = {"abbb", "aabc", "bbcd", "aaceh", "aacexyz", "aacexz", "cdefgh"};
  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  std::shared_ptr<std::string> near_key = std::make_shared<std::string>("aace");

  auto payload1 = traversal::TraversalPayload<traversal::NearTransition>(near_key);
  auto payload2 = traversal::TraversalPayload<traversal::NearTransition>(near_key);

  ComparableStateTraverser<NearStateTraverser> t1(f1, f1->GetStartState(), std::move(payload1), true, 0);
  ComparableStateTraverser<NearStateTraverser> t2(f2, f2->GetStartState(), std::move(payload2), true, 1);

  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(1, t1.GetDepth());
  t1++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK(t1 != t2);
  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t1.GetDepth());
  BOOST_CHECK_EQUAL('a', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(2, t2.GetDepth());
  t1++;
  BOOST_CHECK(t2 < t1);
  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('c', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t1.GetDepth());
  BOOST_CHECK(!t1.IsFinalState());
  BOOST_CHECK_EQUAL('c', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t2.GetDepth());
  BOOST_CHECK(!t2.IsFinalState());

  // inc to aacd (not exact)
  t1++;
  BOOST_CHECK(t2 < t1);
  // inc to aace (exact match)
  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('d', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t1.GetDepth());
  BOOST_CHECK(t1.IsFinalState());
  BOOST_CHECK_EQUAL('e', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t2.GetDepth());
  BOOST_CHECK(!t2.IsFinalState());

  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('h', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(5, t2.GetDepth());
  BOOST_CHECK(t2.IsFinalState());

  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('x', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(5, t2.GetDepth());
  BOOST_CHECK(!t2.IsFinalState());

  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('y', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(6, t2.GetDepth());
  BOOST_CHECK(!t2.IsFinalState());

  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('z', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(7, t2.GetDepth());
  BOOST_CHECK(t2.IsFinalState());

  t2++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('z', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(6, t2.GetDepth());
  BOOST_CHECK(t2.IsFinalState());

  t2++;
  // order change: t2 because exact matching for t2 finished
  BOOST_CHECK(t2 > t1);
  BOOST_CHECK_EQUAL('b', t2.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t2.GetDepth());
  BOOST_CHECK(!t2.IsFinalState());

  t1++;
  BOOST_CHECK(t2 > t1);
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t1.GetDepth());
  BOOST_CHECK(!t1.IsFinalState());

  t1++;
  BOOST_CHECK(t2 > t1);
  BOOST_CHECK_EQUAL('a', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(4, t1.GetDepth());
  BOOST_CHECK(t1.IsFinalState());

  t1++;
  BOOST_CHECK(t2 < t1);
  BOOST_CHECK_EQUAL('b', t1.GetStateLabel());
  BOOST_CHECK_EQUAL(3, t1.GetDepth());
  BOOST_CHECK(!t1.IsFinalState());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
