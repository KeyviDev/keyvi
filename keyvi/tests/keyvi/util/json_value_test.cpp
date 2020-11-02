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
 * json_value_test.cpp
 *
 *  Created on: Oct 27, 2016
 *      Author: hendrik
 */

#include <cmath>
#include <limits>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include "rapidjson/document.h"

#include "keyvi/util/json_value.h"

namespace keyvi {
namespace util {

BOOST_AUTO_TEST_SUITE(JsonValueTests)

BOOST_AUTO_TEST_CASE(EncodeDecodeTest) {
  std::string input =
      "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"j\":-123,\"pi\":3.1415998935699465,\"a\":[1,"
      "2,3,4],\"d\":{\"k\":\"v\"}}";

  std::string encoded = EncodeJsonValue(input);
  std::string output = DecodeJsonValue(encoded);

  BOOST_CHECK_EQUAL(input, output);

  std::string encoded_single_precision_float = EncodeJsonValue(input, true);
  std::string output_single_precision_float = DecodeJsonValue(encoded_single_precision_float);

  BOOST_CHECK_EQUAL(input, output_single_precision_float);
}

BOOST_AUTO_TEST_CASE(EncodeDecodeFloats) {
  std::stringstream string_stream;
  string_stream << std::scientific;

  string_stream << "{\"f_min\":" << std::numeric_limits<float>::min()
                << ",\"f_max\":" << std::numeric_limits<float>::max()
                << ",\"d_min\":" << std::numeric_limits<double>::min()
                << ",\"d_max\":" << std::numeric_limits<double>::max()
                << ",\"nan\":NaN,\"inf\":Inf,\"ninf\":-Infinity}";

  std::string input = string_stream.str();
  std::string encoded = EncodeJsonValue(input);
  std::string output = DecodeJsonValue(encoded);

  rapidjson::Document json_document;

  json_document.Parse<rapidjson::kParseNanAndInfFlag>(output);
  BOOST_CHECK(!json_document.HasParseError());

  BOOST_CHECK(json_document["f_min"].IsNumber());
  BOOST_CHECK_CLOSE(json_document["f_min"].GetDouble(), std::numeric_limits<float>::min(), 0.0001);
  BOOST_CHECK(json_document["f_max"].IsNumber());
  BOOST_CHECK_CLOSE(json_document["f_max"].GetDouble(), std::numeric_limits<float>::max(), 0.0001);
  BOOST_CHECK(json_document["d_min"].IsNumber());
  BOOST_CHECK_CLOSE(json_document["d_min"].GetDouble(), std::numeric_limits<double>::min(), 0.0001);
  BOOST_CHECK(json_document["d_max"].IsNumber());
  BOOST_CHECK_CLOSE(json_document["d_max"].GetDouble(), std::numeric_limits<double>::max(), 0.0001);
  BOOST_CHECK(json_document["nan"].IsNumber());
  BOOST_CHECK(std::isnan(json_document["nan"].GetDouble()));
  BOOST_CHECK(json_document["inf"].IsNumber());
  BOOST_CHECK_EQUAL(json_document["inf"].GetDouble(), std::numeric_limits<double>::infinity());
  BOOST_CHECK(json_document["ninf"].IsNumber());
  BOOST_CHECK_EQUAL(json_document["ninf"].GetDouble(), -std::numeric_limits<double>::infinity());

  std::string encoded_single_precision_float = EncodeJsonValue(input, true);
  std::string output_single_precision_float = DecodeJsonValue(encoded_single_precision_float);

  json_document.Parse<rapidjson::kParseNanAndInfFlag>(output_single_precision_float);
  BOOST_CHECK(!json_document.HasParseError());

  BOOST_CHECK(json_document["f_min"].IsNumber());
  BOOST_CHECK_CLOSE(json_document["f_min"].GetDouble(), std::numeric_limits<float>::min(), 0.0001);
  BOOST_CHECK(json_document["f_max"].IsNumber());
  BOOST_CHECK_CLOSE(json_document["f_max"].GetDouble(), std::numeric_limits<float>::max(), 0.0001);
  BOOST_CHECK(json_document["d_min"].IsNumber());
  BOOST_CHECK_CLOSE(json_document["d_min"].GetDouble(), std::numeric_limits<double>::min(), 0.0001);
  BOOST_CHECK(json_document["d_max"].IsNumber());
  BOOST_CHECK_CLOSE(json_document["d_max"].GetDouble(), std::numeric_limits<double>::max(), 0.0001);
  BOOST_CHECK(json_document["nan"].IsNumber());
  BOOST_CHECK(std::isnan(json_document["nan"].GetDouble()));
  BOOST_CHECK(json_document["inf"].IsNumber());
  BOOST_CHECK_EQUAL(json_document["inf"].GetDouble(), std::numeric_limits<double>::infinity());
  BOOST_CHECK(json_document["ninf"].IsNumber());
  BOOST_CHECK_EQUAL(json_document["ninf"].GetDouble(), -std::numeric_limits<double>::infinity());

  BOOST_CHECK(encoded.size() > encoded_single_precision_float.size());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
