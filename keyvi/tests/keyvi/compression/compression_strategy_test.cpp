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

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "keyvi/compression/compression_selector.h"

namespace keyvi::compression {

namespace {
/** a new (not cached) instance of a strategy */
compression_strategy_t NewStrategy(const CompressionAlgorithm algorithm) {
  switch (algorithm) {
    case NO_COMPRESSION:
      return std::make_unique<RawCompressionStrategy>();
    case ZLIB_COMPRESSION:
      return std::make_unique<ZlibCompressionStrategy>();
    case ZLIB_DEFLATE:
      return std::make_unique<ZlibDeflateCompressionStrategy>();
    case SNAPPY_COMPRESSION:
      return std::make_unique<SnappyCompressionStrategy>();
    case ZSTD_COMPRESSION:
      return std::make_unique<ZstdCompressionStrategy>();
    case ZSTD_MAGICLESS:
      return std::make_unique<ZstdMagiclessCompressionStrategy>();
    default:
      throw std::invalid_argument("not supported");
  }
}
}  // namespace

BOOST_AUTO_TEST_SUITE(CompressionStrategyTests)

BOOST_AUTO_TEST_CASE(CompressWithoutHeader) {
  const std::vector<std::string> inputs = {"", "a", "the quick brown fox jumps over the lazy dog",
                                           std::string(100000, 'x')};

  for (const auto& algorithm :
       {NO_COMPRESSION, ZLIB_COMPRESSION, SNAPPY_COMPRESSION, ZSTD_COMPRESSION, ZLIB_DEFLATE, ZSTD_MAGICLESS}) {
    CompressionStrategy* const strategy = cached_compression_strategy_by_code(algorithm);
    BOOST_CHECK_EQUAL(algorithm, compression_algorithm_by_name(strategy->name()));

    for (const auto& input : inputs) {
      const std::string with_header = strategy->Compress(input);
      BOOST_REQUIRE(!with_header.empty());
      BOOST_CHECK_EQUAL(algorithm, with_header[0]);

      // the header-less variant produces exactly the same output without the compression code
      buffer_t buffer = {'g', 'a', 'r', 'b', 'a', 'g', 'e'};
      strategy->CompressWithoutHeader(&buffer, input.data(), input.size());
      const std::string without_header(buffer.data(), buffer.size());
      BOOST_CHECK_EQUAL(with_header.substr(1), without_header);
      BOOST_CHECK_EQUAL(without_header, strategy->CompressWithoutHeader(input));

      // round trips
      BOOST_CHECK_EQUAL(input, strategy->Decompress(with_header.data(), with_header.size()));
      BOOST_CHECK_EQUAL(input, decompressor_by_code(algorithm)(with_header.data(), with_header.size()));
      BOOST_CHECK_EQUAL(input,
                        decompressor_without_header_by_code(algorithm)(without_header.data(), without_header.size()));
    }
  }
}

BOOST_AUTO_TEST_CASE(CompressionAlgorithmByName) {
  BOOST_CHECK_EQUAL(NO_COMPRESSION, compression_algorithm_by_name(""));
  BOOST_CHECK_EQUAL(ZSTD_DICT_COMPRESSION, compression_algorithm_by_name("zstd_dict"));
  BOOST_CHECK_THROW(compression_algorithm_by_name("lz4"), std::invalid_argument);
  BOOST_CHECK_THROW(decompressor_by_code(ZSTD_DICT_COMPRESSION), std::invalid_argument);
  BOOST_CHECK_THROW(decompressor_without_header_by_code(ZSTD_DICT_COMPRESSION), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ZlibDeflate) {
  CompressionStrategy* const zlib = cached_compression_strategy_by_code(ZLIB_COMPRESSION);
  CompressionStrategy* const zlib_deflate = cached_compression_strategy_by_code(ZLIB_DEFLATE);
  BOOST_CHECK_EQUAL("zlib_deflate", zlib_deflate->name());
  BOOST_CHECK_EQUAL(ZLIB_DEFLATE, compression_algorithm_by_name("zlib_deflate"));
  BOOST_CHECK_EQUAL(4, zlib_deflate->GetFileVersionMin());
  BOOST_CHECK_EQUAL(2, zlib->GetFileVersionMin());

  // not selectable by name
  BOOST_CHECK_THROW(compression_strategy_t(compression_strategy("zlib_deflate")), std::invalid_argument);
  BOOST_CHECK_THROW(compression_strategy_t(compression_strategy("deflate")), std::invalid_argument);

  // raw deflate is the zlib stream without the 2 byte header and the 4 byte adler32 checksum
  for (const std::string input : {"", "a", "the quick brown fox jumps over the lazy dog"}) {
    const std::string zlib_stream = zlib->CompressWithoutHeader(input);
    const std::string deflate_stream = zlib_deflate->CompressWithoutHeader(input);
    BOOST_CHECK_EQUAL(zlib_stream.size(), deflate_stream.size() + 6);
    BOOST_CHECK_EQUAL(zlib_stream.substr(2, deflate_stream.size()), deflate_stream);
  }
}

BOOST_AUTO_TEST_CASE(ZstdMagicless) {
  CompressionStrategy* const zstd = cached_compression_strategy_by_code(ZSTD_COMPRESSION);
  CompressionStrategy* const zstd_magicless = cached_compression_strategy_by_code(ZSTD_MAGICLESS);
  BOOST_CHECK_EQUAL("zstd_magicless", zstd_magicless->name());
  BOOST_CHECK_EQUAL(ZSTD_MAGICLESS, compression_algorithm_by_name("zstd_magicless"));
  BOOST_CHECK_EQUAL(4, zstd_magicless->GetFileVersionMin());

  // not selectable by name
  BOOST_CHECK_THROW(compression_strategy_t(compression_strategy("zstd_magicless")), std::invalid_argument);

  const std::string magic = "\x28\xb5\x2f\xfd";
  BOOST_CHECK_EQUAL(magic, zstd_magicless->GetFramePrefix());
  BOOST_CHECK(zstd->GetFramePrefix().empty());
  BOOST_CHECK(cached_compression_strategy_by_code(ZLIB_DEFLATE)->GetFramePrefix().empty());
  for (const std::string& input : {std::string(), std::string("a"), std::string(100000, 'x'),
                                   std::string("the quick brown fox jumps over the lazy dog")}) {
    const std::string zstd_stream = zstd->CompressWithoutHeader(input);
    const std::string magicless_stream = zstd_magicless->CompressWithoutHeader(input);

    // the same frame, just without the magic number
    BOOST_CHECK_EQUAL(magic, zstd_stream.substr(0, 4));
    BOOST_CHECK_EQUAL(zstd_stream.substr(4), magicless_stream);

    // prepending the magic number makes it decodable by a standard zstd decoder
    const std::string with_magic = magic + magicless_stream;
    const decompress_func_t zstd_decompress = decompressor_without_header_by_code(ZSTD_COMPRESSION);
    BOOST_CHECK_EQUAL(input, zstd_decompress(with_magic.data(), with_magic.size()));
  }

  // garbage input
  BOOST_CHECK_THROW(decompressor_without_header_by_code(ZSTD_MAGICLESS)("", 0), std::runtime_error);
  const std::string truncated = zstd_magicless->CompressWithoutHeader(std::string(1000, 'y')).substr(0, 6);
  BOOST_CHECK_THROW(decompressor_without_header_by_code(ZSTD_MAGICLESS)(truncated.data(), truncated.size()),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(ZstdDecompressWithPrefix) {
  CompressionStrategy* const zstd = cached_compression_strategy_by_code(ZSTD_COMPRESSION);
  for (const std::string& input : {std::string(), std::string("a"), std::string(100000, 'x'),
                                   std::string("the quick brown fox jumps over the lazy dog")}) {
    const std::string frame = zstd->CompressWithoutHeader(input);

    // any split works, the prefix can be longer or shorter than the magic number
    for (size_t split = 0; split <= std::min<size_t>(9, frame.size()); ++split) {
      const std::string prefix = frame.substr(0, split);
      const std::string data = frame.substr(split);
      BOOST_CHECK_EQUAL(input, ZstdCompressionStrategy::DoDecompressWithPrefix(prefix, data.data(), data.size()));
    }
  }

  const std::string frame = zstd->CompressWithoutHeader(std::string(1000, 'y') + "z");
  const std::string prefix = frame.substr(0, 4);
  const std::string body = frame.substr(4);
  // truncated
  BOOST_CHECK_THROW(ZstdCompressionStrategy::DoDecompressWithPrefix(prefix, body.data(), body.size() - 1),
                    std::runtime_error);
  // wrong prefix
  BOOST_CHECK_THROW(ZstdCompressionStrategy::DoDecompressWithPrefix("abcd", body.data(), body.size()),
                    std::runtime_error);
  // garbage
  BOOST_CHECK_THROW(ZstdCompressionStrategy::DoDecompressWithPrefix(prefix, "", 0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(ZstdDecompressWithPrefixLargeFrame) {
  // a frame larger than the buffer kept per thread, the buffer is released and allocated again
  std::string large;
  uint32_t state = 42;
  for (size_t i = 0; i < 2 * ZstdCompressionStrategy::kMaxRetainedFrameBufferSize; ++i) {
    state = (state * 1664525U) + 1013904223U;  // incompressible
    large.push_back(static_cast<char>(state >> 24U));
  }
  CompressionStrategy* const zstd_magicless = cached_compression_strategy_by_code(ZSTD_MAGICLESS);
  const std::string compressed_large = zstd_magicless->CompressWithoutHeader(large);
  BOOST_REQUIRE(compressed_large.size() > ZstdCompressionStrategy::kMaxRetainedFrameBufferSize);
  const std::string compressed_small = zstd_magicless->CompressWithoutHeader("small");

  for (int i = 0; i < 2; ++i) {
    BOOST_CHECK(large == ZstdMagiclessCompressionStrategy::DoDecompressWithoutHeader(compressed_large.data(),
                                                                                     compressed_large.size()));
    BOOST_CHECK_EQUAL("small", ZstdMagiclessCompressionStrategy::DoDecompressWithoutHeader(compressed_small.data(),
                                                                                           compressed_small.size()));
  }
}

BOOST_AUTO_TEST_CASE(ZstdMagiclessDecompressFromThreads) {
  CompressionStrategy* const zstd_magicless = cached_compression_strategy_by_code(ZSTD_MAGICLESS);
  std::vector<std::string> inputs;
  std::vector<std::string> compressed;
  for (int i = 0; i < 16; ++i) {
    inputs.push_back(std::string(1000 + (i * 100), static_cast<char>('a' + i)) + std::to_string(i));
    compressed.push_back(zstd_magicless->CompressWithoutHeader(inputs.back()));
  }

  std::vector<int> failures(8, 0);
  std::vector<std::thread> threads;
  threads.reserve(failures.size());
  for (int& thread_failures : failures) {
    threads.emplace_back([&] {
      for (int round = 0; round < 200; ++round) {
        for (size_t i = 0; i < inputs.size(); ++i) {
          if (ZstdMagiclessCompressionStrategy::DoDecompressWithoutHeader(compressed[i].data(), compressed[i].size()) !=
              inputs[i]) {
            ++thread_failures;
          }
        }
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  for (const int f : failures) {
    BOOST_CHECK_EQUAL(0, f);
  }
}

BOOST_AUTO_TEST_CASE(CachedCompressionStrategy) {
  const std::vector<CompressionAlgorithm> algorithms = {NO_COMPRESSION,   ZLIB_COMPRESSION, SNAPPY_COMPRESSION,
                                                        ZSTD_COMPRESSION, ZLIB_DEFLATE,     ZSTD_MAGICLESS};
  const std::string input = "the quick brown fox jumps over the lazy dog";

  for (const auto algorithm : algorithms) {
    CompressionStrategy* strategy = cached_compression_strategy_by_code(algorithm);
    BOOST_REQUIRE(strategy != nullptr);
    // same instance within the thread
    BOOST_CHECK_EQUAL(strategy, cached_compression_strategy_by_code(algorithm));
    BOOST_CHECK_EQUAL(NewStrategy(algorithm)->name(), strategy->name());

    // reusing the instance gives the same result as a fresh instance
    const std::string expected = NewStrategy(algorithm)->CompressWithoutHeader(input);
    for (int i = 0; i < 3; ++i) {
      BOOST_CHECK_EQUAL(expected, strategy->CompressWithoutHeader(input));
      BOOST_CHECK_EQUAL(expected, strategy->CompressWithoutHeader(input.data(), input.size()));
    }

    // a different instance in another thread
    CompressionStrategy* other_thread_strategy = nullptr;
    std::string other_thread_result;
    std::thread([&] {
      other_thread_strategy = cached_compression_strategy_by_code(algorithm);
      other_thread_result = other_thread_strategy->CompressWithoutHeader(input);
    }).join();
    BOOST_CHECK(other_thread_strategy != strategy);
    BOOST_CHECK_EQUAL(expected, other_thread_result);
  }

  // not supported by code or invalid
  BOOST_CHECK_THROW(cached_compression_strategy_by_code(ZSTD_DICT_COMPRESSION), std::invalid_argument);
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): invalid on purpose
  BOOST_CHECK_THROW(cached_compression_strategy_by_code(static_cast<CompressionAlgorithm>(42)), std::invalid_argument);
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): invalid on purpose
  BOOST_CHECK_THROW(cached_compression_strategy_by_code(static_cast<CompressionAlgorithm>(-1)), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace keyvi::compression
