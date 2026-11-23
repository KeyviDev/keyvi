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

#ifndef KEYVI_COMPRESSION_COMPRESSION_STRATEGY_H_
#define KEYVI_COMPRESSION_COMPRESSION_STRATEGY_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "keyvi/compression/compression_algorithm.h"
#include "keyvi/dictionary/fsa/internal/constants.h"

namespace keyvi {
namespace compression {

// buffer type which is realloc-able
using buffer_t = std::vector<char>;

/**
 * The base class of every compression strategy.
 *
 * Compress() and Decompress() work on the plain compressed data. CompressWithCode() and DecompressWithCode()
 * additionally write and check the compression code as first character, e.g. for values that are stored with
 * their compression. Strategies implement DoCompress() and Decompress() only, the compression code is handled here.
 */
struct CompressionStrategy {
  virtual ~CompressionStrategy() = default;

  /**
   * Compress into buffer starting at offset, without the compression code, the buffer is resized to
   * offset + compressed size.
   */
  virtual void DoCompress(buffer_t* buffer, size_t offset, const char* raw, size_t raw_size) = 0;

  /** The compression code, written as first character by CompressWithCode(). */
  [[nodiscard]] virtual CompressionAlgorithm GetCompressionAlgorithm() const = 0;

  /** Compress without the compression code. */
  void Compress(buffer_t* buffer, const char* raw, size_t raw_size) { DoCompress(buffer, 0, raw, raw_size); }

  std::string Compress(const std::string& raw) { return Compress(raw.data(), raw.size()); }

  std::string Compress(const char* raw, size_t raw_size) {
    buffer_t buf;
    Compress(&buf, raw, raw_size);
    return {buf.data(), buf.size()};
  }

  /** Compress and write the compression code as first character. */
  void CompressWithCode(buffer_t* buffer, const char* raw, size_t raw_size) {
    DoCompress(buffer, 1, raw, raw_size);
    (*buffer)[0] = static_cast<char>(GetCompressionAlgorithm());
  }

  std::string CompressWithCode(const std::string& raw) { return CompressWithCode(raw.data(), raw.size()); }

  std::string CompressWithCode(const char* raw, size_t raw_size) {
    buffer_t buf;
    CompressWithCode(&buf, raw, raw_size);
    return {buf.data(), buf.size()};
  }

  /** Decompress data written by Compress(). */
  virtual std::string Decompress(const char* data, const size_t size) const = 0;

  /** Decompress data written by CompressWithCode(), checks and removes the compression code. */
  std::string DecompressWithCode(const char* data, const size_t size) const {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (size == 0 || static_cast<unsigned char>(data[0]) != GetCompressionAlgorithm()) {
      throw std::invalid_argument(name() + " decompression failed: unexpected compression code");
    }
    return Decompress(data + 1, size - 1);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }

  /** The "name" of the compression strategy. */
  [[nodiscard]] virtual std::string name() const = 0;

  /** The minimum version this compressor requires */
  [[nodiscard]] virtual uint64_t GetFileVersionMin() const = 0;

  /**
   * Bytes the compressor leaves out at the start of every compressed value which are needed by a standard decoder,
   * e.g. the magic number of zstd. Empty if the output is a complete stream.
   */
  [[nodiscard]] virtual std::string GetFramePrefix() const { return {}; }
};

using compression_strategy_t = std::unique_ptr<CompressionStrategy>;

/**
 * A compression strategy that does almost nothing; i.e. it only adds
 * the length field.
 */
struct RawCompressionStrategy final : public CompressionStrategy {
  void DoCompress(buffer_t* buffer, size_t offset, const char* raw, size_t raw_size) override {
    buffer->resize(raw_size + offset);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::memcpy(buffer->data() + offset, raw, raw_size);
  }

  std::string Decompress(const char* data, const size_t size) const override { return DoDecompress(data, size); }

  static std::string DoDecompress(const char* data, const size_t size) { return {data, size}; }

  [[nodiscard]] CompressionAlgorithm GetCompressionAlgorithm() const override { return NO_COMPRESSION; }

  [[nodiscard]] std::string name() const override { return "raw"; }

  [[nodiscard]] uint64_t GetFileVersionMin() const override { return KEYVI_FILE_VERSION_MIN; }
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // KEYVI_COMPRESSION_COMPRESSION_STRATEGY_H_
