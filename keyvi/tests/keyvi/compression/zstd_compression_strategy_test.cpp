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

  auto uncompressed = cs->Decompress(compressed);
  BOOST_CHECK_EQUAL(input, uncompressed);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace compression
}  // namespace keyvi
