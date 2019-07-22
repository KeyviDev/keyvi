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
 * entry_iterator_test.cpp
 *
 *  Created on: Oct 27, 2015
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/entry_iterator.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE(EntryIteratorTests)

BOOST_AUTO_TEST_CASE(EmptyDictionary) {
  // empty dictionary
  std::vector<std::string> test_data = {};
  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  EntryIterator it(f);
  EntryIterator end_it = EntryIterator();

  BOOST_CHECK(end_it == it);
}

BOOST_AUTO_TEST_CASE(DictionariesIteratorCompare) {
  std::vector<std::string> test_data = {"aaa", "aaaa", "aabc", "cdef", "ef"};

  std::vector<std::string> test_data2 = {"bbb", "bcd", "cdag", "effffff"};

  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  testing::TempDictionary dictionary2(&test_data2);
  automata_t f2 = dictionary2.GetFsa();

  EntryIterator it1(f);
  EntryIterator it1_2(f);

  EntryIterator it2(f2);

  EntryIterator end_it = EntryIterator();

  BOOST_CHECK(it1 < it2);

  ++it1_2;
  BOOST_CHECK(it1 < it1_2);
  ++it1;
  BOOST_CHECK(it1 == it1_2);
  BOOST_CHECK(it1 < it2);
  ++it1;
  BOOST_CHECK(it1 > it1_2);
  BOOST_CHECK(it1 < it2);
  ++it1;
  BOOST_CHECK(it1 > it2);
  ++it1;
  ++it1;
  BOOST_CHECK(end_it == it1);
}

BOOST_AUTO_TEST_CASE(DictionariesIteratorCompareString) {
  std::vector<std::string> test_data = {"aaa", "aaaa", "aabc", "cdef", "ef"};

  testing::TempDictionary dictionary(&test_data);
  automata_t f = dictionary.GetFsa();

  EntryIterator it(f);

  BOOST_CHECK_EQUAL("aaa", it.GetKey());
  BOOST_CHECK(it == "aaa");
  ++it;
  BOOST_CHECK_EQUAL("aaaa", it.GetKey());
  BOOST_CHECK(it != "aaaaa");
  BOOST_CHECK(it == "aaaa");
  ++it;
  BOOST_CHECK_EQUAL("aabc", it.GetKey());
  BOOST_CHECK(it != "a");
  BOOST_CHECK(it == "aabc");
  ++it;
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
