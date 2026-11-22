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

// only the stable zstd API is used, no ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>

#include "keyvi/dictionary/fsa/internal/constants.h"

#ifndef ZSTD_DEFAULT_CLEVEL

/*-=====  Pre-defined compression levels  =====-*/
#define ZSTD_DEFAULT_CLEVEL 3
#define ZSTD_MAX_CLEVEL 22
#endif

#include "keyvi/compression/compression_algorithm.h"
#include "keyvi/compression/compression_strategy.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace compression {

/** A compression strategy that wraps zstd. */
struct ZstdCompressionStrategy final : public CompressionStrategy {
  explicit ZstdCompressionStrategy(int compression_level = ZSTD_DEFAULT_CLEVEL)
      : compression_level_(compression_level), cctx_(ZSTD_createCCtx()) {
    if (cctx_ == nullptr) {    // GCOVR_EXCL_BR_LINE
      throw std::bad_alloc();  // GCOVR_EXCL_LINE
    }
  }

  ~ZstdCompressionStrategy() override { ZSTD_freeCCtx(cctx_); }

  ZstdCompressionStrategy(const ZstdCompressionStrategy&) = delete;
  ZstdCompressionStrategy& operator=(const ZstdCompressionStrategy&) = delete;
  ZstdCompressionStrategy(ZstdCompressionStrategy&&) = delete;
  ZstdCompressionStrategy& operator=(ZstdCompressionStrategy&&) = delete;

  using CompressionStrategy::CompressWithoutHeader;

  inline void Compress(buffer_t* buffer, const char* raw, size_t raw_size) { DoCompress(buffer, raw, raw_size); }

  inline void CompressWithoutHeader(buffer_t* buffer, const char* raw, size_t raw_size) override {
    DoCompressAt(buffer, 0, raw, raw_size);
  }

  inline void DoCompress(buffer_t* buffer, const char* raw, size_t raw_size) {
    DoCompressAt(buffer, 1, raw, raw_size);
    buffer->data()[0] = static_cast<char>(ZSTD_COMPRESSION);
  }

  /** Compress into buffer starting at offset, the buffer is resized to offset + compressed size. */
  inline void DoCompressAt(buffer_t* buffer, size_t offset, const char* raw, size_t raw_size) {
    size_t output_length = ZSTD_compressBound(raw_size);
    buffer->resize(output_length + offset);

    // same output as ZSTD_compress, but reuses the compression context instead of creating one per call
    output_length = ZSTD_compressCCtx(cctx_, buffer->data() + offset, output_length, raw, raw_size, compression_level_);
    if (ZSTD_isError(output_length) != 0U) {  // GCOVR_EXCL_BR_LINE
      // GCOVR_EXCL_START
      throw std::runtime_error(std::string("zstd compression failed: ") + ZSTD_getErrorName(output_length));
      // GCOVR_EXCL_STOP
    }
    buffer->resize(output_length + offset);
  }

  std::string Decompress(const char* data, const size_t size) override { return DoDecompress(data, size); }

  static std::string DoDecompress(const char* data, const size_t size) {
    return DoDecompressWithoutHeader(data + 1, size - 1);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }

  static std::string DoDecompressWithoutHeader(const char* data, const size_t size) {
    const unsigned long long content_size = ZSTD_getFrameContentSize(data, size);  // NOLINT(runtime/int)
    if (content_size == ZSTD_CONTENTSIZE_UNKNOWN || content_size == ZSTD_CONTENTSIZE_ERROR) {
      throw std::runtime_error("zstd decompression failed: unable to determine content size");
    }

    std::string uncompressed;
    uncompressed.resize(content_size);

    // reuses the decompression context, ZSTD_decompress would create one per call
    const size_t result =
        ZSTD_decompressDCtx(GetDecompressionContext(), uncompressed.data(), uncompressed.size(), data, size);
    if (ZSTD_isError(result) != 0U) {
      throw std::runtime_error(std::string("zstd decompression failed: ") + ZSTD_getErrorName(result));
    }
    if (result != uncompressed.size()) {  // GCOVR_EXCL_BR_LINE
      throw std::runtime_error("zstd decompression failed: unexpected content size");  // GCOVR_EXCL_LINE
    }

    return uncompressed;
  }

  /**
   * Decompress a zstd frame that has been split into prefix and data, e.g. the frame start (magic number)
   * which is stored only once for all values.
   *
   * Uses only the stable zstd API and a standard decoder, so it works with every frame format the linked
   * zstd library supports, regardless of the magic number.
   */
  static std::string DoDecompressWithPrefix(const std::string& prefix, const char* data, const size_t size) {
    // reassemble the frame in a buffer per thread: copying the compressed data is cheap compared to the
    // decompression and faster than streaming prefix and data as 2 chunks
    thread_local std::string frame;

    // keep the buffer for typical values, but do not keep large buffers per thread (also on error)
    struct ReleaseLargeBuffer {
      std::string* buffer;
      ~ReleaseLargeBuffer() {
        if (buffer->capacity() > kMaxRetainedFrameBufferSize) {
          std::string().swap(*buffer);
        }
      }
    } release_large_buffer{&frame};

    frame.assign(prefix).append(data, size);
    return DoDecompressWithoutHeader(frame.data(), frame.size());
  }

  /** maximum size of the frame buffer kept per thread for DoDecompressWithPrefix */
  static constexpr size_t kMaxRetainedFrameBufferSize = 1024 * 1024;

  std::string name() const { return "zstd"; }

  uint64_t GetFileVersionMin() const { return 3; }

 private:
  int compression_level_;
  ZSTD_CCtx* cctx_;

  /** decompression context per thread, readers are shared between threads */
  static ZSTD_DCtx* GetDecompressionContext() {
    thread_local const std::unique_ptr<ZSTD_DCtx, decltype(&ZSTD_freeDCtx)> dctx(ZSTD_createDCtx(), &ZSTD_freeDCtx);
    if (!dctx) {  // GCOVR_EXCL_BR_LINE
      throw std::runtime_error("failed to initialize zstd decompression");  // GCOVR_EXCL_LINE
    }
    return dctx.get();
  }
};

/**
 * zstd without the 4 byte magic number at the start of every frame.
 *
 * Not selectable by name, used internally by value stores that store the compression once (JsonValueStore2).
 * To decompress with a standard zstd decoder, prepend the magic number (0x28 0xb5 0x2f 0xfd).
 */
struct ZstdMagiclessCompressionStrategy final : public CompressionStrategy {
  explicit ZstdMagiclessCompressionStrategy(int compression_level = ZSTD_DEFAULT_CLEVEL)
      : compression_level_(compression_level), cctx_(ZSTD_createCCtx()) {
    if (cctx_ == nullptr) {    // GCOVR_EXCL_BR_LINE
      throw std::bad_alloc();  // GCOVR_EXCL_LINE
    }
  }

  ~ZstdMagiclessCompressionStrategy() override { ZSTD_freeCCtx(cctx_); }

  ZstdMagiclessCompressionStrategy(const ZstdMagiclessCompressionStrategy&) = delete;
  ZstdMagiclessCompressionStrategy& operator=(const ZstdMagiclessCompressionStrategy&) = delete;
  ZstdMagiclessCompressionStrategy(ZstdMagiclessCompressionStrategy&&) = delete;
  ZstdMagiclessCompressionStrategy& operator=(ZstdMagiclessCompressionStrategy&&) = delete;

  using CompressionStrategy::Compress;
  using CompressionStrategy::CompressWithoutHeader;

  void Compress(buffer_t* buffer, const char* raw, size_t raw_size) override {
    CompressAt(buffer, 1, raw, raw_size);
    buffer->data()[0] = static_cast<char>(ZSTD_MAGICLESS);
  }

  void CompressWithoutHeader(buffer_t* buffer, const char* raw, size_t raw_size) override {
    CompressAt(buffer, 0, raw, raw_size);
  }

  std::string Decompress(const char* data, const size_t size) override { return DoDecompress(data, size); }

  static std::string DoDecompress(const char* data, const size_t size) {
    return DoDecompressWithoutHeader(data + 1, size - 1);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }

  static std::string DoDecompressWithoutHeader(const char* data, const size_t size) {
    return ZstdCompressionStrategy::DoDecompressWithPrefix(FramePrefix(), data, size);
  }

  std::string name() const override { return "zstd_magicless"; }

  uint64_t GetFileVersionMin() const override { return 4; }

  /**
   * The magic number of the zstd1 frame format (0xFD2FB528, little endian), the bytes left out of every frame.
   *
   * Intentionally a constant and not ZSTD_MAGICNUMBER: compression verifies that the frames written by the linked
   * zstd library start with it and fails otherwise, a different frame format must be introduced explicitly.
   */
  static std::string FramePrefix() { return {"\x28\xb5\x2f\xfd", 4}; }

  std::string GetFramePrefix() const override { return FramePrefix(); }

 private:
  int compression_level_;
  ZSTD_CCtx* cctx_;

  /** Compress into buffer starting at offset, the buffer is resized to offset + compressed size. */
  void CompressAt(buffer_t* buffer, size_t offset, const char* raw, size_t raw_size) {
    static constexpr size_t kPrefixSize = 4;
    size_t output_length = ZSTD_compressBound(raw_size);
    buffer->resize(output_length + offset);

    // a standard frame (ZSTD_compressCCtx is faster than ZSTD_compress2 for small values), the magic number is
    // removed afterwards
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    char* frame = buffer->data() + offset;
    output_length = ZSTD_compressCCtx(cctx_, frame, output_length, raw, raw_size, compression_level_);
    if (ZSTD_isError(output_length) != 0U) {  // GCOVR_EXCL_BR_LINE
      // GCOVR_EXCL_START
      throw std::runtime_error(std::string("zstd magicless compression failed: ") + ZSTD_getErrorName(output_length));
      // GCOVR_EXCL_STOP
    }
    if (output_length < kPrefixSize || std::memcmp(frame, FramePrefix().data(), kPrefixSize) != 0) {
      // GCOVR_EXCL_START
      throw std::runtime_error("zstd magicless compression failed: unexpected frame format");
      // GCOVR_EXCL_STOP
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::memmove(frame, frame + kPrefixSize, output_length - kPrefixSize);
    buffer->resize(offset + output_length - kPrefixSize);
  }
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // KEYVI_COMPRESSION_ZSTD_COMPRESSION_STRATEGY_H_
