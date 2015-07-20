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
 * sparse_array_builder_test.cpp
 *
 *  Created on: May 7, 2014
 *      Author: hendrik
 */

#define SPARSE_ARRAY_BUILDER_UNIT_TEST

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "dictionary/fsa/internal/sparse_array_builder.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

BOOST_AUTO_TEST_SUITE( SparseArrayBuilderTests )

BOOST_AUTO_TEST_CASE( writeState ) {
  SparseArrayPersistence<uint32_t> p(2048, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint32_t>> b(limit, &p, false);
  UnpackedState<SparseArrayPersistence<uint32_t>> u1(&p);
  u1.Add(65, 100);
  u1.Add(66, 101);
  u1.Add(233, 102);

  // final state
  u1.Add(256, 1000);
  b.WriteState(10,u1);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(75), 65);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(75), 100);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(76), 66);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(76), 101);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(243), 233);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(243), 102);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(266), 1);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(266), 1000);
}

BOOST_AUTO_TEST_CASE( writeFinalStateCompact ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  b.WriteFinalTransition(25,55);
  BOOST_CHECK_EQUAL(55, p.ReadFinalValue(25));

  b.WriteFinalTransition(42,0);
  BOOST_CHECK_EQUAL(0, p.ReadFinalValue(42));

  b.WriteFinalTransition(2048,23);
  BOOST_CHECK_EQUAL(23, p.ReadFinalValue(2048));

}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
