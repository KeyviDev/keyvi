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

#ifndef KEYVI_COMPRESSION_SNAPPY_COMPRESSION_STRATEGY_H_
#define KEYVI_COMPRESSION_SNAPPY_COMPRESSION_STRATEGY_H_

#include <snappy.h>
#include <sstream>
#include <string>

#include "keyvi/compression/compression_strategy.h"
#include "keyvi/dictionary/fsa/internal/constants.h"

namespace keyvi {
namespace compression {

/** A compression strategy that wraps snappy. */
struct SnappyCompressionStrategy final : public CompressionStrategy {
  inline void Compress(buffer_t* buffer, const char* raw, size_t raw_size) { DoCompress(buffer, raw, raw_size); }

  static inline void DoCompress(buffer_t* buffer, const char* raw, size_t raw_size) {
    size_t output_length = snappy::MaxCompressedLength(raw_size);
    buffer->resize(output_length + 1);
    buffer->data()[0] = static_cast<char>(SNAPPY_COMPRESSION);
    snappy::RawCompress(raw, raw_size, buffer->data() + 1, &output_length);
    buffer->resize(output_length + 1);
  }

  inline std::string Decompress(const std::string& compressed) { return DoDecompress(compressed); }

  static std::string DoDecompress(const std::string& compressed) {
    std::string uncompressed;
    snappy::Uncompress(&compressed.data()[1], compressed.size() - 1, &uncompressed);
    return uncompressed;
  }

  std::string name() const { return "snappy"; }

  uint64_t GetFileVersionMin() const { return KEYVI_FILE_VERSION_MIN; }
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // KEYVI_COMPRESSION_SNAPPY_COMPRESSION_STRATEGY_H_
