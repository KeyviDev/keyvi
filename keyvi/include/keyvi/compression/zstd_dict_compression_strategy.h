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
#include <memory>
#include <stdexcept>
#include <string>

#include "keyvi/compression/compression_algorithm.h"
#include "keyvi/compression/compression_strategy.h"

namespace keyvi::compression {

#ifndef ZSTD_DEFAULT_CLEVEL
constexpr int kZstdDefaultCompressionLevel = 3;
#else
constexpr int kZstdDefaultCompressionLevel = ZSTD_DEFAULT_CLEVEL;
#endif

struct ZstdDictCompressionStrategy final : public CompressionStrategy {
  ZstdDictCompressionStrategy(const char* dict_data, size_t dict_size,
                              int compression_level = kZstdDefaultCompressionLevel)
      : cctx_(ZSTD_createCCtx()),
        cdict_(ZSTD_createCDict(dict_data, dict_size, compression_level)),
        ddict_(ZSTD_createDDict(dict_data, dict_size)) {
    if (cctx_ == nullptr || cdict_ == nullptr || ddict_ == nullptr) {                // GCOVR_EXCL_BR_LINE
      Cleanup();                                                                     // GCOVR_EXCL_LINE
      throw std::runtime_error("failed to initialize zstd dictionary compression");  // GCOVR_EXCL_LINE
    }
  }

  ~ZstdDictCompressionStrategy() override { Cleanup(); }

  ZstdDictCompressionStrategy(const ZstdDictCompressionStrategy&) = delete;
  ZstdDictCompressionStrategy& operator=(const ZstdDictCompressionStrategy&) = delete;
  ZstdDictCompressionStrategy(ZstdDictCompressionStrategy&&) = delete;
  ZstdDictCompressionStrategy& operator=(ZstdDictCompressionStrategy&&) = delete;

  void DoCompress(buffer_t* buffer, size_t offset, const char* raw, size_t raw_size) override {
    size_t output_length = ZSTD_compressBound(raw_size);
    buffer->resize(output_length + offset);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    output_length = ZSTD_compress_usingCDict(cctx_, buffer->data() + offset, output_length, raw, raw_size, cdict_);
    if (ZSTD_isError(output_length) != 0U) {  // GCOVR_EXCL_BR_LINE
      // GCOVR_EXCL_START
      throw std::runtime_error(std::string("zstd dict compression failed: ") + ZSTD_getErrorName(output_length));
      // GCOVR_EXCL_STOP
    }
    buffer->resize(output_length + offset);
  }

  std::string Decompress(const char* data, const size_t size) const override {
    // unsigned long long: the sentinel values do not fit into size_t on 32 bit platforms
    const unsigned long long content_size = ZSTD_getFrameContentSize(data, size);  // NOLINT(runtime/int)
    if (content_size == ZSTD_CONTENTSIZE_UNKNOWN || content_size == ZSTD_CONTENTSIZE_ERROR) {
      throw std::runtime_error("zstd dict decompression failed: unable to determine content size");
    }

    std::string uncompressed;
    uncompressed.resize(content_size);
    const size_t result = ZSTD_decompress_usingDDict(GetDecompressionContext(), uncompressed.data(),
                                                     uncompressed.size(), data, size, ddict_);
    if (ZSTD_isError(result) != 0U) {  // GCOVR_EXCL_BR_LINE
      // GCOVR_EXCL_START
      throw std::runtime_error(std::string("zstd dict decompression failed: ") + ZSTD_getErrorName(result));
      // GCOVR_EXCL_STOP
    }
    if (result != uncompressed.size()) {                                          // GCOVR_EXCL_BR_LINE
      throw std::runtime_error("zstd dict decompression failed: size mismatch");  // GCOVR_EXCL_LINE
    }

    return uncompressed;
  }

  [[nodiscard]] CompressionAlgorithm GetCompressionAlgorithm() const override { return ZSTD_DICT_COMPRESSION; }

  [[nodiscard]] std::string name() const override { return "zstd_dict"; }

  [[nodiscard]] uint64_t GetFileVersionMin() const override { return 4; }

 private:
  // ZSTD_free* functions accept nullptr
  void Cleanup() {
    ZSTD_freeCCtx(cctx_);
    ZSTD_freeCDict(cdict_);
    ZSTD_freeDDict(ddict_);
  }

  /** decompression context per thread, readers are shared between threads, the DDict is read-only */
  static ZSTD_DCtx* GetDecompressionContext() {
    thread_local const std::unique_ptr<ZSTD_DCtx, decltype(&ZSTD_freeDCtx)> dctx(ZSTD_createDCtx(), &ZSTD_freeDCtx);
    if (!dctx) {                                                            // GCOVR_EXCL_BR_LINE
      throw std::runtime_error("failed to initialize zstd decompression");  // GCOVR_EXCL_LINE
    }
    return dctx.get();
  }

  ZSTD_CCtx* cctx_;
  ZSTD_CDict* cdict_;
  ZSTD_DDict* ddict_;
};

}  // namespace keyvi::compression

#endif  // KEYVI_COMPRESSION_ZSTD_DICT_COMPRESSION_STRATEGY_H_
