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
 * jump_consistent_hash_test.cpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/util/jump_consistent_hash.h"

namespace keyvi {
namespace dictionary {
namespace util {

BOOST_AUTO_TEST_SUITE(JCHTests)

BOOST_AUTO_TEST_CASE(JCHHashTest) {
  BOOST_CHECK_EQUAL(3, JumpConsistentHashString("test", 10));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */
