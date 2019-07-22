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

#include <string>

#include <boost/test/unit_test.hpp>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

#include "keyvi/util/serialization_utils.h"

namespace keyvi {
namespace util {

BOOST_AUTO_TEST_SUITE(SerializationUtilsTests)

BOOST_AUTO_TEST_CASE(GetUint64FromValueOrStringTest) {
  rapidjson::StringBuffer string_buffer;

  // create a test document
  {
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

    writer.StartObject();
    writer.Key("size");
    writer.Uint64(42);
    writer.Key("size_as_string");
    writer.String(std::to_string(42));
    writer.Key("uint64");
    writer.Uint64(999);
    writer.Key("uint64_as_string");
    writer.String(std::to_string(999));
    writer.Key("string");
    writer.String("some_non_numeric_string");
    writer.EndObject();
  }

  rapidjson::Document d;
  d.Parse(string_buffer.GetString(), string_buffer.GetLength());
  BOOST_CHECK_EQUAL(999ul, SerializationUtils::GetOptionalUInt64FromValueOrString(d, "uint64", 77ul));
  BOOST_CHECK_EQUAL(999ul, SerializationUtils::GetOptionalUInt64FromValueOrString(d, "uint64_as_string", 77ul));
  BOOST_CHECK_EQUAL(77ul, SerializationUtils::GetOptionalUInt64FromValueOrString(d, "uint64_not_there", 77ul));
  BOOST_CHECK_THROW(SerializationUtils::GetOptionalUInt64FromValueOrString(d, "string", 77ul), boost::bad_lexical_cast);

  BOOST_CHECK_EQUAL(42ul, SerializationUtils::GetOptionalSizeFromValueOrString(d, "size", 21ul));
  BOOST_CHECK_EQUAL(42ul, SerializationUtils::GetOptionalSizeFromValueOrString(d, "size_as_string", 21ul));
  BOOST_CHECK_EQUAL(21ul, SerializationUtils::GetOptionalSizeFromValueOrString(d, "size_not_there", 21ul));
  BOOST_CHECK_THROW(SerializationUtils::GetOptionalSizeFromValueOrString(d, "string", 77ul), boost::bad_lexical_cast);

  BOOST_CHECK_EQUAL(999ul, SerializationUtils::GetUint64FromValueOrString(d, "uint64"));
  BOOST_CHECK_EQUAL(999ul, SerializationUtils::GetUint64FromValueOrString(d, "uint64_as_string"));
  BOOST_CHECK_THROW(SerializationUtils::GetUint64FromValueOrString(d, "uint64_not_there"), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
