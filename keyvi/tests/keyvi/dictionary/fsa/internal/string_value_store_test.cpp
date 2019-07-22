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
 * string_value_store_test.cpp
 *
 *  Created on: Jul 30, 2014
 *      Author: hendrik
 */

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/string_value_store.h"
#include "keyvi/util/configuration.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(StringValueTest)

BOOST_AUTO_TEST_CASE(minimization) {
  StringValueStore ivsw1(keyvi::util::parameters_t{{"hello", "bello"}});
  StringValueStore ivsw2;
  StringValueStore strings;

  bool no_minimization = false;
  uint64_t v = strings.AddValue("mytestvalue", &no_minimization);
  BOOST_CHECK_EQUAL(v, 0);
  uint64_t w = strings.AddValue("othervalue", &no_minimization);

  BOOST_CHECK(w > 0);
  BOOST_CHECK_EQUAL(v, strings.AddValue("mytestvalue", &no_minimization));
  BOOST_CHECK_EQUAL(w, strings.AddValue("othervalue", &no_minimization));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
