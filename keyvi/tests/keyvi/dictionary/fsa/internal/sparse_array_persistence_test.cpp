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
 * sparse_array_persistence_test.cpp
 *
 *  Created on: May 5, 2014
 *      Author: hendrik
 */

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(SparseArrayPersistenceTests)

BOOST_AUTO_TEST_CASE(basic) {
  size_t memoryLimit = 1024 * 1024;

  SparseArrayPersistence<> p(memoryLimit, boost::filesystem::temp_directory_path());
  p.WriteTransition(1, 42, 43);
  p.WriteTransition(200, 44, 45);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1), 42);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(1), 43);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(200), 44);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(200), 45);

  // enforce flush in persistence
  p.BeginNewState(memoryLimit * 20);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1), 42);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(1), 43);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(200), 44);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(200), 45);
}
/*
BOOST_AUTO_TEST_CASE( bruteforcewriteandread )
{
  size_t memoryLimit = 1024 * 10;

  SparseArrayPersistence p(memoryLimit, boost::filesystem::temp_directory_path());
  for (int i = 0; i < 20 * memoryLimit; ++i){
    if (i%200 == 0){
      // simulate new state
      p.BeginNewState(i);
    }

    p.WriteTransition(i, i % 256, i % 10000);
  }

  for (int i = 0; i < 20 * memoryLimit; ++i){
      BOOST_CHECK_EQUAL(p.ReadTransitionLabel(i), i%256);
      BOOST_CHECK_EQUAL(p.ReadTransitionValue(i), i%10000);
    }
}
*/
BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
