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
#include <snappy.h>

#include "compression/compression_strategy.h"

namespace keyvi {
namespace compression {

/** A compression strategy that does nothing. */
struct SnappyCompressionStrategy final : public CompressionStrategy {
  inline std::string Compress(const char* raw, size_t raw_size) {
    return DoCompress(raw, raw_size);
  }

  static std::string DoCompress(const char* raw, size_t raw_size) {
    std::string compressed;
    snappy::Compress(raw, raw_size, &compressed);

    return std::string(1, static_cast<char>(SNAPPY_COMPRESSION)) + compressed;
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
