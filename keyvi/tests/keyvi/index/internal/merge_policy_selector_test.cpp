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
 * merge_policy_selector_test.cpp
 *
 *  Created on: Jan 20, 2018
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/index/internal/merge_policy_selector.h"

namespace keyvi {
namespace index {
namespace internal {

BOOST_AUTO_TEST_SUITE(MergePolicySelectorTests)

BOOST_AUTO_TEST_CASE(wrong_select) {
  // check that it throws
  BOOST_CHECK_THROW(merge_policy("invalid"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(select_simple) {
  merge_policy_t simple = merge_policy("simple");
  BOOST_CHECK(simple);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */
