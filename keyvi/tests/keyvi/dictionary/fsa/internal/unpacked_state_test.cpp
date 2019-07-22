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
 * unpacked_state_test.cpp
 *
 *  Created on: May 7, 2014
 *      Author: hendrik
 */

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/dictionary/fsa/internal/unpacked_state.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

BOOST_AUTO_TEST_SUITE(UnpackedStateTests)

BOOST_AUTO_TEST_CASE(simple) {
  SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());
  UnpackedState<SparseArrayPersistence<>> u1(&p);
  u1.Add(65, 100);
  u1.Add(66, 101);

  BOOST_CHECK_EQUAL(u1.size(), 2);
  u1.Add(80, 102);
  BOOST_CHECK_EQUAL(u1.size(), 3);

  BOOST_CHECK(u1.get_BitVector().Get(65));
  BOOST_CHECK(u1.get_BitVector().Get(66));
  BOOST_CHECK(!u1.get_BitVector().Get(67));
  int hashcode = u1.GetHashcode();

  u1.Clear();
  BOOST_CHECK_EQUAL(u1.size(), 0);

  BOOST_CHECK(!u1.get_BitVector().Get(65));
  BOOST_CHECK(!u1.get_BitVector().Get(66));
  BOOST_CHECK(!u1.get_BitVector().Get(67));
  BOOST_CHECK(hashcode != u1.GetHashcode());
}

BOOST_AUTO_TEST_CASE(clear) {
  SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());
  UnpackedState<SparseArrayPersistence<>> u1(&p);
  u1.Add(65, 100);
  u1.Add(66, 101);

  BOOST_CHECK(u1.get_BitVector().Get(65));
  BOOST_CHECK(u1.get_BitVector().Get(66));
  BOOST_CHECK(!u1.get_BitVector().Get(67));
  BOOST_CHECK(!u1.get_BitVector().Get(68));
  BOOST_CHECK_EQUAL(u1.size(), 2);
  u1.Clear();
  BOOST_CHECK_EQUAL(u1.size(), 0);
  BOOST_CHECK(!u1.get_BitVector().Get(65));
  BOOST_CHECK(!u1.get_BitVector().Get(66));
  BOOST_CHECK(!u1.get_BitVector().Get(67));
  BOOST_CHECK(!u1.get_BitVector().Get(68));
}

BOOST_AUTO_TEST_CASE(hash_with_weights) {
  SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());
  UnpackedState<SparseArrayPersistence<>> u1(&p);
  u1.Add(65, 100);
  u1.Add(66, 101);

  UnpackedState<SparseArrayPersistence<>> u2(&p);
  u2.Add(65, 100);
  u2.Add(66, 101);
  u2.UpdateWeightIfHigher(42);

  UnpackedState<SparseArrayPersistence<>> u3(&p);
  u3.Add(65, 100);
  u3.Add(66, 101);
  u3.UpdateWeightIfHigher(444);

  // u1 and u2 should have different hashcodes as u2 has weight but u1 not
  BOOST_CHECK(u1.GetHashcode() != u2.GetHashcode());

  // u2 and u3 should have equal hashcodes although the weights are different
  BOOST_CHECK_EQUAL(u2.GetHashcode(), u3.GetHashcode());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
