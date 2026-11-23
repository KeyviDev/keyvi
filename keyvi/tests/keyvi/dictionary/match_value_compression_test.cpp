/* keyvi - A key value store.
 *
 * Copyright 2026 Hendrik Muhs<hendrik.muhs@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "keyvi/compression/compression_algorithm.h"
#include "keyvi/compression/compression_selector.h"
#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/testing/temp_dictionary.h"
#include "keyvi/util/json_value.h"

namespace keyvi::dictionary {

namespace {
constexpr std::array<compression::CompressionAlgorithm, 6> kAlgorithms = {
    compression::NO_COMPRESSION,   compression::ZLIB_COMPRESSION, compression::SNAPPY_COMPRESSION,
    compression::ZSTD_COMPRESSION, compression::ZLIB_DEFLATE,     compression::ZSTD_MAGICLESS};

/** the value of the match in every compression decodes to the same msgpack value as the raw value */
void CheckMsgPackedValues(const Match& match) {
  const std::string msgpacked_value = match.GetMsgPackedValueAsString();
  BOOST_REQUIRE(!msgpacked_value.empty());

  const std::string raw_value = match.GetRawValueAsString();
  BOOST_CHECK_EQUAL(msgpacked_value, compression::decompress(raw_value.data(), raw_value.size()));

  for (const auto algorithm : kAlgorithms) {
    const std::string compressed = match.GetMsgPackedValueAsString(algorithm);
    BOOST_CHECK_EQUAL(msgpacked_value,
                      compression::decompressor_by_code(algorithm)(compressed.data(), compressed.size()));
  }

  BOOST_CHECK_THROW(match.GetMsgPackedValueAsString(compression::ZSTD_DICT_COMPRESSION), std::invalid_argument);
}

void CheckMsgPackedValues(const testing::TempDictionary& temp_dictionary, const std::string& key) {
  const Dictionary d(temp_dictionary.GetFsa());
  const match_t m = d[key];
  BOOST_REQUIRE(m);
  CheckMsgPackedValues(*m);
}
}  // namespace

BOOST_AUTO_TEST_SUITE(MatchValueCompressionTests)

BOOST_AUTO_TEST_CASE(MsgPackedValueByDictionaryType) {
  std::vector<std::pair<std::string, uint32_t>> int_data = {{"abc", 42}, {"abd", 1000000}};
  CheckMsgPackedValues(testing::TempDictionary(&int_data, false), "abd");
  CheckMsgPackedValues(testing::TempDictionary(&int_data, true), "abd");

  std::vector<std::pair<std::string, std::string>> string_data = {{"abc", "a string value"}, {"abd", "another one"}};
  CheckMsgPackedValues(testing::TempDictionary(&string_data), "abc");

  // short and long (compressed) json values
  std::vector<std::pair<std::string, std::string>> json_data = {
      {"abc", R"({"a":1})"}, {"abd", R"({"a":")" + std::string(100, 'x') + R"(","b":[1,2,3]})"}};
  const testing::TempDictionary json_dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&json_data);
  CheckMsgPackedValues(json_dictionary, "abc");
  CheckMsgPackedValues(json_dictionary, "abd");

  std::vector<std::pair<std::string, std::vector<float>>> float_data = {{"abc", {0.1F, 0.2F, 0.3F, 0.4F, 0.5F}},
                                                                        {"abd", {1.1F, 1.2F, 1.3F, 1.4F, 1.5F}}};
  CheckMsgPackedValues(testing::TempDictionary::makeTempDictionaryFromFloats(&float_data), "abd");
}

BOOST_AUTO_TEST_CASE(MsgPackedValueWithoutDictionary) {
  // a match that holds the value itself, e.g. after deserialization
  const std::string json = R"({"a":")" + std::string(100, 'x') + R"("})";
  Match m;
  m.SetRawValue(keyvi::util::EncodeJsonValue(json));
  BOOST_REQUIRE_EQUAL(compression::SNAPPY_COMPRESSION, m.GetRawValueAsString()[0]);
  CheckMsgPackedValues(m);

  m.SetRawValue(keyvi::util::EncodeJsonValue(R"({"a":1})"));
  BOOST_REQUIRE_EQUAL(compression::NO_COMPRESSION, m.GetRawValueAsString()[0]);
  CheckMsgPackedValues(m);

  m.SetRawValue("");
  BOOST_CHECK(m.GetMsgPackedValueAsString(compression::ZSTD_COMPRESSION).empty());
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace keyvi::dictionary
