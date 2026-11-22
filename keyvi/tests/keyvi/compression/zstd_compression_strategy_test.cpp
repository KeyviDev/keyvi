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

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "keyvi/compression/compression_selector.h"

namespace keyvi {
namespace compression {

BOOST_AUTO_TEST_SUITE(ZstdCompressionStrategyTests)

BOOST_AUTO_TEST_CASE(SimpleCompressAndUncompress) {
  const char* input = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

  std::unique_ptr<CompressionStrategy> cs;
  cs.reset(compression_strategy("zstd"));

  auto compressed = cs->Compress(input);
  BOOST_CHECK(compressed.size() < strlen(input));

  auto uncompressed = cs->Decompress(compressed.data(), compressed.size());
  BOOST_CHECK_EQUAL(input, uncompressed);
}

BOOST_AUTO_TEST_CASE(SameOutputAsZstdCompress) {
  // the strategy reuses its compression context, the output must be identical to the one-shot ZSTD_compress
  ZstdCompressionStrategy strategy;
  std::string large;
  for (int i = 0; i < 20000; ++i) {
    large += std::to_string(i * 7919 % 1000) + ",";
  }

  for (int round = 0; round < 2; ++round) {
    for (const std::string& input : {std::string(), std::string("a"), std::string(100000, 'x'), large,
                                     std::string("the quick brown fox jumps over the lazy dog")}) {
      std::vector<char> expected(ZSTD_compressBound(input.size()));
      expected.resize(ZSTD_compress(expected.data(), expected.size(), input.data(), input.size(), ZSTD_DEFAULT_CLEVEL));

      const std::string compressed = strategy.CompressWithoutHeader(input);
      BOOST_CHECK_EQUAL(std::string(expected.data(), expected.size()), compressed);
      BOOST_CHECK_EQUAL(input,
                        ZstdCompressionStrategy::DoDecompressWithoutHeader(compressed.data(), compressed.size()));
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace compression
}  // namespace keyvi
