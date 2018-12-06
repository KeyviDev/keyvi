//
// keyvi - A key value store.
//
// Copyright 2015-2018 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * serialization_utils_test.cpp
 *
 *  Created on: Dec 1, 2018
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

#include "util/serialization_utils.h"

namespace keyvi {
namespace util {

BOOST_AUTO_TEST_SUITE(SerializationUtilsTests)

BOOST_AUTO_TEST_CASE(GetUint64FromValueOrStringTest) {
  rapidjson::StringBuffer string_buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

  rapidjson::Document d;
  d.Parse(string_buffer.GetString(), string_buffer.GetLength());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
