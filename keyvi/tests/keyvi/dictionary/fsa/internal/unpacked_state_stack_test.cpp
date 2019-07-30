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
 * unpacked_state_stack_test.cpp
 *
 *  Created on: May 7, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/dictionary/fsa/internal/unpacked_state_stack.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

BOOST_AUTO_TEST_SUITE(UnpackedStateStackTests)

BOOST_AUTO_TEST_CASE(basic) {
  SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());
  UnpackedStateStack<SparseArrayPersistence<>> s(&p, 20);

  s.Insert(3, 42, 56);
  s.Insert(3, 47, 57);
  s.Insert(2, 40, 33);
  s.Insert(1, 20, 11);

  auto u = s.Get(3);
  BOOST_CHECK_EQUAL(u->size(), 2);
  BOOST_CHECK(u->get_BitVector().Get(42));
  BOOST_CHECK(!u->get_BitVector().Get(66));
  BOOST_CHECK(u->get_BitVector().Get(47));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
