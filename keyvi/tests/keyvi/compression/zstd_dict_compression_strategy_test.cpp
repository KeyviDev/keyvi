/* keyvi - A key value store.
 *
 * Copyright 2025 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#include <zdict.h>
#include <zstd.h>

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "keyvi/compression/zstd_dict_compression_strategy.h"

namespace keyvi {
namespace compression {

BOOST_AUTO_TEST_SUITE(ZstdDictCompressionStrategyTests)

namespace {

std::vector<char> TrainDictionary(const std::vector<std::string>& samples, size_t dict_capacity = 4096) {
  std::vector<char> combined;
  std::vector<size_t> sample_sizes;
  for (const auto& s : samples) {
    combined.insert(combined.end(), s.begin(), s.end());
    sample_sizes.push_back(s.size());
  }

  std::vector<char> dict_buffer(dict_capacity);
  const size_t dict_size = ZDICT_trainFromBuffer(dict_buffer.data(), dict_buffer.size(), combined.data(),
                                                 sample_sizes.data(), static_cast<unsigned>(samples.size()));
  if (ZSTD_isError(dict_size) != 0U) {
    dict_buffer.clear();
    return dict_buffer;
  }
  dict_buffer.resize(dict_size);
  return dict_buffer;
}

}  // namespace

BOOST_AUTO_TEST_CASE(CompressAndDecompress) {
  std::vector<std::string> samples;
  samples.reserve(200);
  for (int i = 0; i < 200; ++i) {
    samples.push_back("the quick brown fox jumps over the lazy dog " + std::to_string(i));
  }

  auto dict = TrainDictionary(samples);
  BOOST_REQUIRE(!dict.empty());

  ZstdDictCompressionStrategy strategy(dict.data(), dict.size());

  const std::string input = "the quick brown fox jumps over the lazy dog 42";
  auto compressed = strategy.Compress(input);

  BOOST_CHECK_EQUAL(static_cast<unsigned char>(compressed[0]), ZSTD_DICT_COMPRESSION);

  auto decompressed = strategy.Decompress(compressed.data(), compressed.size());
  BOOST_CHECK_EQUAL(input, decompressed);
}

BOOST_AUTO_TEST_CASE(CompressedSmallerThanPlainZstd) {
  std::vector<std::string> samples;
  samples.reserve(200);
  for (int i = 0; i < 200; ++i) {
    samples.push_back("the quick brown fox jumps over the lazy dog " + std::to_string(i));
  }

  auto dict = TrainDictionary(samples);
  BOOST_REQUIRE(!dict.empty());

  ZstdDictCompressionStrategy dict_strategy(dict.data(), dict.size());

  const std::string input = "the quick brown fox jumps over the lazy dog 99";

  buffer_t dict_buf;
  dict_strategy.Compress(&dict_buf, input.data(), input.size());

  buffer_t plain_buf;
  plain_buf.resize(ZSTD_compressBound(input.size()) + 1);
  const size_t plain_size =
      ZSTD_compress(plain_buf.data(), plain_buf.size(), input.data(), input.size(), kZstdDefaultCompressionLevel);

  BOOST_CHECK(dict_buf.size() <= plain_size + 1);
}

BOOST_AUTO_TEST_CASE(EmptyInput) {
  std::vector<std::string> samples;
  samples.reserve(200);
  for (int i = 0; i < 200; ++i) {
    samples.push_back("sample data " + std::to_string(i));
  }

  auto dict = TrainDictionary(samples);
  BOOST_REQUIRE(!dict.empty());

  ZstdDictCompressionStrategy strategy(dict.data(), dict.size());

  const std::string input;
  auto compressed = strategy.Compress(input);
  auto decompressed = strategy.Decompress(compressed.data(), compressed.size());
  BOOST_CHECK_EQUAL(input, decompressed);
}

BOOST_AUTO_TEST_CASE(Name) {
  std::vector<std::string> samples;
  samples.reserve(200);
  for (int i = 0; i < 200; ++i) {
    samples.push_back("sample " + std::to_string(i));
  }

  auto dict = TrainDictionary(samples);
  BOOST_REQUIRE(!dict.empty());

  const ZstdDictCompressionStrategy strategy(dict.data(), dict.size());
  BOOST_CHECK_EQUAL("zstd_dict", strategy.name());
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace compression
}  // namespace keyvi
