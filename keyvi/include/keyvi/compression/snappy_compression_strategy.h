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
 * snappy_compression_strategy.h
 *
 *  Created on: October 2, 2015
 *      Author: David Mark Nemeskey<nemeskey.david@gmail.com>
 */

#ifndef SNAPPY_COMPRESSION_STRATEGY_H_
#define SNAPPY_COMPRESSION_STRATEGY_H_

#include <string>
#include <sstream>
#include <snappy.h>

#include "compression/compression_strategy.h"

namespace keyvi {
namespace compression {

/** A compression strategy that wraps snappy. */
struct SnappyCompressionStrategy final : public CompressionStrategy {
  inline void Compress(buffer_t& buffer, const char* raw, size_t raw_size) {
    DoCompress(buffer, raw, raw_size);
  }

  static inline void DoCompress (buffer_t& buffer, const char* raw, size_t raw_size)
  {
      size_t output_length = snappy::MaxCompressedLength(raw_size);
      buffer.resize(output_length + 1);
      buffer[0] = static_cast<char>(SNAPPY_COMPRESSION);
      snappy::RawCompress(raw, raw_size, buffer.data() + 1, &output_length);
      buffer.resize(output_length + 1);
  }

  static inline std::string DoCompress(const char* raw, size_t raw_size) {
    buffer_t buf;
    DoCompress(buf, raw, raw_size);
    return std::string(buf.data(), buf.size());
  }

  inline std::string Decompress(const std::string& compressed) {
    return DoDecompress(compressed);
  }

  static std::string DoDecompress(const std::string& compressed) {
    std::string uncompressed;
    snappy::Uncompress(&compressed.data()[1], compressed.size() - 1,
                       &uncompressed);
    return uncompressed;
  }

  std::string name() const { return "snappy"; }
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // SNAPPY_COMPRESSION_STRATEGY_H_
