/* * keyvi - A key value store.
 *
 * Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
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

/*
 * compression_selector.h
 *
 *  Created on: October 6, 2015
 *      Author: David Mark Nemeskey<nemeskey.david@gmail.com>
 */

#ifndef KEYVI_COMPRESSION_COMPRESSION_SELECTOR_H_
#define KEYVI_COMPRESSION_COMPRESSION_SELECTOR_H_

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "keyvi/compression/compression_algorithm.h"
#include "keyvi/compression/compression_strategy.h"
#include "keyvi/compression/snappy_compression_strategy.h"
#include "keyvi/compression/zlib_compression_strategy.h"
#include "keyvi/compression/zstd_compression_strategy.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace compression {

/** Returns an instance of a compression strategy by name. */
inline CompressionStrategy* compression_strategy(const std::string& name = "") {
  // inline to avoid "multiple definition of" errors that come from defining
  // the function in a header file
  auto lower_name = name;
  // NOTE: this does not work for non-ASCII strings
  boost::algorithm::to_lower(lower_name);
  if (lower_name == "zip" || lower_name == "zlib" || lower_name == "z") {
    return new ZlibCompressionStrategy();  // compression level?
  } else if (lower_name == "snappy") {
    return new SnappyCompressionStrategy();
  } else if (lower_name == "zstd") {
    return new ZstdCompressionStrategy();
  } else if (lower_name == "" || lower_name == "none" || lower_name == "raw") {
    return new RawCompressionStrategy();
  } else {
    throw std::invalid_argument(name + " is not a valid compression");
  }
}

using decompress_func_t = std::string (*)(const char*, const size_t);

/**
 * Returns a decompression function for data that does _not_ start with the compression code,
 * e.g. for value stores that store the compression once instead of per value.
 */
inline decompress_func_t decompressor_by_code(const CompressionAlgorithm algorithm) {
  switch (algorithm) {
    case NO_COMPRESSION:
      return RawCompressionStrategy::DoDecompress;
    case ZLIB_COMPRESSION:
      return ZlibCompressionStrategy::DoDecompress;
    case ZLIB_DEFLATE:
      return ZlibDeflateCompressionStrategy::DoDecompress;
    case SNAPPY_COMPRESSION:
      return SnappyCompressionStrategy::DoDecompress;
    case ZSTD_COMPRESSION:
      return ZstdCompressionStrategy::DoDecompress;
    case ZSTD_MAGICLESS:
      return ZstdMagiclessCompressionStrategy::DoDecompress;
    case ZSTD_DICT_COMPRESSION:
      throw std::invalid_argument(
          "zstd dictionary compression requires the dictionary, use ZstdDictCompressionStrategy");
    default:
      throw std::invalid_argument("Invalid compression algorithm " +
                                  boost::lexical_cast<std::string>(static_cast<int>(algorithm)));
  }
}

/** Decompresses data that starts with the compression code, e.g. a value written by
 * CompressionStrategy::CompressWithCode(). */
inline std::string decompress(const char* data, const size_t size) {
  if (size == 0) {
    throw std::invalid_argument("can not decompress empty data, compression code missing");
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  return decompressor_by_code(static_cast<CompressionAlgorithm>(data[0]))(data + 1, size - 1);
}

/**
 * Returns the compression code for the canonical name of a compression strategy, the inverse of
 * CompressionStrategy::name().
 *
 * Meant for names written by keyvi itself, not for user input: the lookup is case-sensitive, does not accept
 * aliases and also covers internal formats. To select a compression by user input (the "compression" parameter),
 * use compression_strategy(name).
 */
inline CompressionAlgorithm compression_algorithm_by_name(const std::string& name) {
  if (name.empty() || name == "raw") {
    return NO_COMPRESSION;
  } else if (name == "zlib") {
    return ZLIB_COMPRESSION;
  } else if (name == "zlib_deflate") {
    return ZLIB_DEFLATE;
  } else if (name == "snappy") {
    return SNAPPY_COMPRESSION;
  } else if (name == "zstd") {
    return ZSTD_COMPRESSION;
  } else if (name == "zstd_magicless") {
    return ZSTD_MAGICLESS;
  } else if (name == "zstd_dict") {
    return ZSTD_DICT_COMPRESSION;
  }
  throw std::invalid_argument(name + " is not a valid compression");
}

/**
 * Returns a cached instance of a compression strategy by enum, owned by the calling thread.
 *
 * Creating a strategy can be expensive (e.g. zlib allocates its compression state), strategies are not
 * thread-safe, therefore one instance per thread and algorithm. Do not keep the pointer beyond the current thread,
 * components that need their own compressor (e.g. value store writers) create the strategy directly.
 */
inline CompressionStrategy* cached_compression_strategy_by_code(const CompressionAlgorithm algorithm) {
  thread_local std::array<compression_strategy_t, 8> strategies;
  static_assert(ZSTD_MAGICLESS < 8, "increase the size of the compression strategy cache");

  const auto index = static_cast<size_t>(algorithm);
  if (index >= strategies.size()) {
    throw std::invalid_argument("Invalid compression algorithm " + std::to_string(static_cast<int>(algorithm)));
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index): index is checked above
  compression_strategy_t& strategy = strategies[index];
  if (!strategy) {
    switch (algorithm) {
      case NO_COMPRESSION:
        strategy = std::make_unique<RawCompressionStrategy>();
        break;
      case ZLIB_COMPRESSION:
        strategy = std::make_unique<ZlibCompressionStrategy>();
        break;
      case ZLIB_DEFLATE:
        strategy = std::make_unique<ZlibDeflateCompressionStrategy>();
        break;
      case SNAPPY_COMPRESSION:
        strategy = std::make_unique<SnappyCompressionStrategy>();
        break;
      case ZSTD_COMPRESSION:
        strategy = std::make_unique<ZstdCompressionStrategy>();
        break;
      case ZSTD_MAGICLESS:
        strategy = std::make_unique<ZstdMagiclessCompressionStrategy>();
        break;
      case ZSTD_DICT_COMPRESSION:
        // not cached and not supported for re-compression: requires the dictionary
        throw std::invalid_argument("zstd dictionary compression is not supported for re-compression");
      default:
        throw std::invalid_argument("Invalid compression algorithm " + std::to_string(static_cast<int>(algorithm)));
    }
  }
  return strategy.get();
}

} /* namespace compression */
} /* namespace keyvi */

#endif  // KEYVI_COMPRESSION_COMPRESSION_SELECTOR_H_
