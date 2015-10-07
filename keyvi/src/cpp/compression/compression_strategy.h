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

#include "dictionary/util/vint.h"

namespace keyvi {
namespace compression {

/**
 * The base class of every compression strategy.
 *
 * All strategies (aside from RawCompressionStrategy) should insert a " "
 * (space) character before the string to mark it compressed, as well as the
 * encoded length of the resulting string (including the space). The space,
 * but not the encoded size, will also have to be taken into account in the
 * decompress method.
 */
struct CompressionStrategy {
  virtual ~CompressionStrategy() = default;

  inline std::string Compress(const std::string& raw) {
    return Compress(raw.data(), raw.size());
  }

  virtual std::string Compress(const char* raw, size_t raw_size) = 0;

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
  std::string Compress(const char* raw, size_t raw_size) {
    std::string compressed;
    dictionary::util::encodeVarint(raw_size, compressed);
    compressed.append(std::string(raw, raw_size));
    return compressed;
  }

  std::string Decompress(const std::string& compressed) {
    return compressed;
  }

  std::string name() const { return "raw"; }
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // COMPRESSION_STRATEGY_H_
