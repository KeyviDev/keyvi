//
// keyvi - A key value store.
//
// Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * index_settings_test.cpp
 *
 *  Created on: Feb 14, 2018
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/index/internal/index_settings.h"

namespace keyvi {
namespace index {
namespace internal {

BOOST_AUTO_TEST_SUITE(IndexSettingsTests)

BOOST_AUTO_TEST_CASE(defaultkeyvimergerbin) {
  IndexSettings settings({});

  BOOST_CHECK_EQUAL(std::string("keyvimerger"), settings.GetKeyviMergerBin());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */
