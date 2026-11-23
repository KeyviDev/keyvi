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
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

#include "keyvi/compression/compression_algorithm.h"
#include "keyvi/compression/compression_strategy.h"
#include "keyvi/dictionary/fsa/internal/constants.h"

namespace keyvi {
namespace compression {

/** A compression strategy that wraps snappy. */
struct SnappyCompressionStrategy final : public CompressionStrategy {
  using CompressionStrategy::CompressWithoutHeader;

  void Compress(buffer_t* buffer, const char* raw, size_t raw_size) override { DoCompress(buffer, raw, raw_size); }

  void CompressWithoutHeader(buffer_t* buffer, const char* raw, size_t raw_size) override {
    DoCompressAt(buffer, 0, raw, raw_size);
  }

  static void DoCompress(buffer_t* buffer, const char* raw, size_t raw_size) {
    DoCompressAt(buffer, 1, raw, raw_size);
    (*buffer)[0] = static_cast<char>(SNAPPY_COMPRESSION);
  }

  /** Compress into buffer starting at offset, the buffer is resized to offset + compressed size. */
  static void DoCompressAt(buffer_t* buffer, size_t offset, const char* raw, size_t raw_size) {
    size_t output_length = snappy::MaxCompressedLength(raw_size);
    buffer->resize(output_length + offset);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    snappy::RawCompress(raw, raw_size, buffer->data() + offset, &output_length);
    buffer->resize(output_length + offset);
  }

  std::string Decompress(const char* data, const size_t size) override { return DoDecompress(data, size); }

  static std::string DoDecompress(const char* data, const size_t size) {
    return DoDecompressWithoutHeader(data + 1, size - 1);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }

  static std::string DoDecompressWithoutHeader(const char* data, const size_t size) {
    std::string uncompressed;
    snappy::Uncompress(data, size, &uncompressed);
    return uncompressed;
  }

  [[nodiscard]] std::string name() const override { return "snappy"; }

  [[nodiscard]] uint64_t GetFileVersionMin() const override { return KEYVI_FILE_VERSION_MIN; }
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // KEYVI_COMPRESSION_SNAPPY_COMPRESSION_STRATEGY_H_
