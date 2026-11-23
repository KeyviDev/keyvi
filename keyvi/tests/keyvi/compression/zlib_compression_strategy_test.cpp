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

#include <stdexcept>
#include <string>

#include <boost/test/unit_test.hpp>

#include "keyvi/compression/zlib_compression_strategy.h"

namespace keyvi {
namespace compression {

BOOST_AUTO_TEST_SUITE(ZlibCompressionStrategyTests)

BOOST_AUTO_TEST_CASE(CompressAndDecompress) {
  ZlibCompressionStrategy zlib;
  CompressionStrategy& strategy = zlib;
  const std::string input(1000, 'x');
  const std::string compressed = strategy.CompressWithCode(input);
  BOOST_CHECK_EQUAL(input, strategy.DecompressWithCode(compressed.data(), compressed.size()));
}

BOOST_AUTO_TEST_CASE(DecompressTruncated) {
  ZlibCompressionStrategy zlib;
  CompressionStrategy& strategy = zlib;
  const std::string compressed = strategy.CompressWithCode(std::string(1000, 'x'));

  // zlib reports truncated input without a message (msg is NULL)
  BOOST_CHECK_EXCEPTION(
      strategy.DecompressWithCode(compressed.data(), compressed.size() - 3), std::runtime_error,
      [](const std::runtime_error& e) { return std::string(e.what()).find("unknown error") != std::string::npos; });
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace compression
}  // namespace keyvi
