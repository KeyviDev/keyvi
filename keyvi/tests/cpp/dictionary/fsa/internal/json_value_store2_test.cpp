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
 * json_value_store_test.cpp
 *
 *  Created on: October 14, 2015
 *      Author: David Mark Nemeskey<nemeskey.david@gmail.com>
 */

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "dictionary/fsa/internal/json_value_store_deprecated.h"
#include "dictionary/fsa/internal/constants.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE( JsonValueDeprecatedTest )

BOOST_AUTO_TEST_CASE( minimization )
{
  JsonValueStoreDeprecated strings(
      IValueStoreWriter::vs_param_t{{TEMPORARY_PATH_KEY, "/tmp"}});
  bool no_minimization = false;
  uint32_t v = strings.GetValue("{\"mytestvalue\":25, \"mytestvalue2\":23}", no_minimization);
  BOOST_CHECK_EQUAL(v,0);
  uint32_t w = strings.GetValue("othervalue", no_minimization);
  uint32_t x = strings.GetValue("{\"mytestvalue3\":55, \"mytestvalue4\":773}", no_minimization);

  BOOST_CHECK(w>0);
  BOOST_CHECK_EQUAL(v,strings.GetValue("{\"mytestvalue\": 25, \"mytestvalue2\": 23}", no_minimization));
  BOOST_CHECK_EQUAL(x,strings.GetValue("{\"mytestvalue3\":55, \"mytestvalue4\":773}", no_minimization));
  BOOST_CHECK_EQUAL(w,strings.GetValue("othervalue", no_minimization));
}

BOOST_AUTO_TEST_CASE( minimization2 )
{
  JsonValueStoreDeprecated strings(
      IValueStoreWriter::vs_param_t{{TEMPORARY_PATH_KEY, "/tmp"}});
  bool no_minimization = false;

  uint64_t v = strings.GetValue("{\"f\": 5571575, \"df\": 1362790, \"uqf\": 2129086, \"tf1df\": 99838, \"tf2df\": 274586, \"tf3df\": 481278, \"tf5df\": 811157}", no_minimization);
  uint64_t w = strings.GetValue("{\"f\": 3, \"df\": 1, \"uqf\": 1, \"tf1df\": 0, \"tf2df\": 0, \"tf3df\": 0, \"tf5df\": 0}", no_minimization);
  BOOST_CHECK(v != w);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
