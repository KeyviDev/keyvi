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
 * zstd_compression_strategy.h
 *
 *  Created on: September 10, 2016
 *      Author: Hendrik Muhs<hendrik.muhs@gmail.com>
 */

#ifndef KEYVI_COMPRESSION_ZSTD_COMPRESSION_STRATEGY_H_
#define KEYVI_COMPRESSION_ZSTD_COMPRESSION_STRATEGY_H_

#include <zstd.h>

#include <string>

#include "keyvi/dictionary/fsa/internal/constants.h"

#ifndef ZSTD_DEFAULT_CLEVEL

/*-=====  Pre-defined compression levels  =====-*/
#define ZSTD_DEFAULT_CLEVEL 3
#define ZSTD_MAX_CLEVEL 22
#endif

#include "keyvi/compression/compression_strategy.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace compression {

/** A compression strategy that wraps zlib. */
struct ZstdCompressionStrategy final : public CompressionStrategy {
  ZstdCompressionStrategy(int compression_level = ZSTD_DEFAULT_CLEVEL) : compression_level_(compression_level) {}

  inline void Compress(buffer_t* buffer, const char* raw, size_t raw_size) { DoCompress(buffer, raw, raw_size); }

  inline void DoCompress(buffer_t* buffer, const char* raw, size_t raw_size) {
    size_t output_length = ZSTD_compressBound(raw_size);
    buffer->resize(output_length + 1);
    buffer->data()[0] = static_cast<char>(ZSTD_COMPRESSION);

    output_length = ZSTD_compress(buffer->data() + 1, output_length, raw, raw_size, compression_level_);
    buffer->resize(output_length + 1);
  }

  inline std::string Decompress(const std::string& compressed) { return DoDecompress(compressed); }

  static std::string DoDecompress(const std::string& compressed) {
    std::string uncompressed;

    size_t dest_size = ZSTD_getFrameContentSize(&compressed.data()[1], compressed.size() - 1);
    uncompressed.resize(dest_size);
    ZSTD_decompress(&uncompressed[0], dest_size, &compressed.data()[1], compressed.size() - 1);

    return uncompressed;
  }

  std::string name() const { return "zstd"; }

  uint64_t GetFileVersionMin() const { return 3; }

 private:
  int compression_level_;
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // KEYVI_COMPRESSION_ZSTD_COMPRESSION_STRATEGY_H_
