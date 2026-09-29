/* * keyvi - A key value store.
 *
 * Copyright 2025 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#ifndef KEYVI_COMPRESSION_ZSTD_DICT_COMPRESSION_STRATEGY_H_
#define KEYVI_COMPRESSION_ZSTD_DICT_COMPRESSION_STRATEGY_H_

#include <zstd.h>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "keyvi/compression/compression_algorithm.h"
#include "keyvi/compression/compression_strategy.h"

#ifndef ZSTD_DEFAULT_CLEVEL
constexpr int kZstdDefaultCompressionLevel = 3;
#else
constexpr int kZstdDefaultCompressionLevel = ZSTD_DEFAULT_CLEVEL;
#endif

namespace keyvi::compression {

struct ZstdDictCompressionStrategy final : public CompressionStrategy {
  ZstdDictCompressionStrategy(const char* dict_data, size_t dict_size,
                              int compression_level = kZstdDefaultCompressionLevel)
      : cctx_(ZSTD_createCCtx()),
        dctx_(ZSTD_createDCtx()),
        cdict_(ZSTD_createCDict(dict_data, dict_size, compression_level)),
        ddict_(ZSTD_createDDict(dict_data, dict_size)) {
    if (cctx_ == nullptr || dctx_ == nullptr || cdict_ == nullptr || ddict_ == nullptr) {
      Cleanup();
      throw std::runtime_error("failed to initialize zstd dictionary compression");
    }
  }

  ~ZstdDictCompressionStrategy() override { Cleanup(); }

  ZstdDictCompressionStrategy(const ZstdDictCompressionStrategy&) = delete;
  ZstdDictCompressionStrategy& operator=(const ZstdDictCompressionStrategy&) = delete;
  ZstdDictCompressionStrategy(ZstdDictCompressionStrategy&&) = delete;
  ZstdDictCompressionStrategy& operator=(ZstdDictCompressionStrategy&&) = delete;

  using CompressionStrategy::Compress;

  void Compress(buffer_t* buffer, const char* raw, size_t raw_size) override {
    size_t output_length = ZSTD_compressBound(raw_size);
    buffer->resize(output_length + 1);
    (*buffer)[0] = static_cast<char>(ZSTD_DICT_COMPRESSION);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    output_length = ZSTD_compress_usingCDict(cctx_, buffer->data() + 1, output_length, raw, raw_size, cdict_);
    if (ZSTD_isError(output_length) != 0U) {
      throw std::runtime_error(std::string("zstd dict compression failed: ") + ZSTD_getErrorName(output_length));
    }
    buffer->resize(output_length + 1);
  }

  std::string Decompress(const char* data, const size_t size) override {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const size_t dest_size = ZSTD_getFrameContentSize(data + 1, size - 1);
    if (dest_size == ZSTD_CONTENTSIZE_UNKNOWN || dest_size == ZSTD_CONTENTSIZE_ERROR) {
      throw std::runtime_error("zstd dict decompression failed: unable to determine content size");
    }

    std::string uncompressed;
    uncompressed.resize(dest_size);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const size_t result = ZSTD_decompress_usingDDict(dctx_, uncompressed.data(), dest_size, data + 1, size - 1, ddict_);
    if (ZSTD_isError(result) != 0U) {
      throw std::runtime_error(std::string("zstd dict decompression failed: ") + ZSTD_getErrorName(result));
    }

    return uncompressed;
  }

  [[nodiscard]] std::string name() const override { return "zstd_dict"; }

  [[nodiscard]] uint64_t GetFileVersionMin() const override { return 4; }

 private:
  void Cleanup() {
    if (cctx_ != nullptr) {
      ZSTD_freeCCtx(cctx_);
    }
    if (dctx_ != nullptr) {
      ZSTD_freeDCtx(dctx_);
    }
    if (cdict_ != nullptr) {
      ZSTD_freeCDict(cdict_);
    }
    if (ddict_ != nullptr) {
      ZSTD_freeDDict(ddict_);
    }
  }

  ZSTD_CCtx* cctx_;
  ZSTD_DCtx* dctx_;
  ZSTD_CDict* cdict_;
  ZSTD_DDict* ddict_;
};

}  // namespace keyvi::compression

#endif  // KEYVI_COMPRESSION_ZSTD_DICT_COMPRESSION_STRATEGY_H_
