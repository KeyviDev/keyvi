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
 * compression_strategy.h
 *
 *  Created on: October 1, 2015
 *      Author: David Mark Nemeskey<nemeskey.david@gmail.com>
 */

#ifndef COMPRESSION_STRATEGY_H_
#define COMPRESSION_STRATEGY_H_

#include <string>
#include <cstring>

namespace keyvi {
namespace compression {

enum CompressionCode {
  NO_COMPRESSION = 0,
  ZLIB_COMPRESSION = 1,
  SNAPPY_COMPRESSION = 2,
};

// buffer type which is realloc-able
typedef std::vector<char> buffer_t;

/**
 * The base class of every compression strategy.
 *
 * All strategies (aside from RawCompressionStrategy) should insert the
 * compression code (see above) as the first character of the encoded string.
 * The code will also have to be taken into account in the decompress method.
 */
struct CompressionStrategy {
  virtual ~CompressionStrategy() = default;

  virtual void Compress(buffer_t& buffer, const char* raw, size_t raw_size) = 0;

  inline std::string Compress(const std::string& raw) {
    return Compress(raw.data(), raw.size());
  }

  inline std::string Compress(const char* raw, size_t raw_size) {
    buffer_t buf;
    Compress(buf, raw, raw_size);
    return std::string(buf.data(), buf.size());
  }

  /**
   * By the time this function is called, the length field added in Compress()
   * will have been removed.
   */
  virtual std::string Decompress(const std::string& compressed) = 0;

  /** The "name" of the compression strategy. */
  virtual std::string name() const = 0;
};

/**
 * A compression strategy that does almost nothing; i.e. it only adds
 * the length field.
 */
struct RawCompressionStrategy final : public CompressionStrategy {
  inline void Compress(buffer_t& buffer, const char* raw, size_t raw_size) {
      DoCompress(buffer, raw, raw_size);
    }


  static inline void DoCompress(buffer_t& buffer, const char* raw, size_t raw_size) {
    buffer.resize(raw_size + 1);
    buffer[0] = static_cast<char>(NO_COMPRESSION);
    std::memcpy(buffer.data() + 1, raw, raw_size);
  }

  static inline std::string DoCompress(const char* raw, size_t raw_size) {
    buffer_t buf;
    DoCompress(buf, raw, raw_size);
    return std::string(buf.data(), buf.size());
  }

  inline std::string Decompress(const std::string& compressed) {
    return DoDecompress(compressed);
  }

  static inline std::string DoDecompress(const std::string& compressed) {
    return compressed.substr(1);
  }

  std::string name() const { return "raw"; }
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // COMPRESSION_STRATEGY_H_
