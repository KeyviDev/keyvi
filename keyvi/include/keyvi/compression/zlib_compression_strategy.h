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
 * zlib_compression_strategy.h
 *
 *  Created on: October 1, 2015
 *      Author: David Mark Nemeskey<nemeskey.david@gmail.com>
 */

#ifndef KEYVI_COMPRESSION_ZLIB_COMPRESSION_STRATEGY_H_
#define KEYVI_COMPRESSION_ZLIB_COMPRESSION_STRATEGY_H_

#define ZLIB_CONST

#include <zconf.h>
#include <zlib.h>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

#include "keyvi/compression/compression_algorithm.h"
#include "keyvi/compression/compression_strategy.h"
#include "keyvi/dictionary/fsa/internal/constants.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace compression {

/** zlib format: deflate with a 2 byte header and an adler32 checksum (4 bytes) */
struct ZlibFormat {
  static constexpr int kWindowBits = MAX_WBITS;
  static constexpr CompressionAlgorithm kAlgorithm = ZLIB_COMPRESSION;
  static constexpr const char* kName = "zlib";
  static constexpr uint64_t kFileVersionMin = KEYVI_FILE_VERSION_MIN;
};

/**
 * raw deflate format: no header and no checksum, saves 6 bytes compared to zlib.
 *
 * Not selectable via the "compression" parameter (compression_strategy(name)).
 */
struct ZlibDeflateFormat {
  static constexpr int kWindowBits = -MAX_WBITS;
  static constexpr CompressionAlgorithm kAlgorithm = ZLIB_DEFLATE;
  static constexpr const char* kName = "zlib_deflate";
  static constexpr uint64_t kFileVersionMin = 4;
};

/** A compression strategy that wraps zlib, Format selects the stream format (zlib or raw deflate). */
template <typename Format>
struct BasicZlibCompressionStrategy final : public CompressionStrategy {
  explicit BasicZlibCompressionStrategy(int compression_level = Z_BEST_COMPRESSION) {
    // init zlib structure for compression

    zstream_compress_.zalloc = Z_NULL;
    zstream_compress_.zfree = Z_NULL;
    zstream_compress_.opaque = Z_NULL;

    // see zlib.h level 9 means more speed for the price of memory
    const int mem_level = 9;

    if (deflateInit2(&zstream_compress_, compression_level, Z_DEFLATED, Format::kWindowBits, mem_level,
                     Z_DEFAULT_STRATEGY) != Z_OK) {
      throw std::bad_alloc();
    }
  }

  ~BasicZlibCompressionStrategy() override { deflateEnd(&zstream_compress_); }

  // the zlib stream must not be shared, deflateEnd would be called twice
  BasicZlibCompressionStrategy(const BasicZlibCompressionStrategy&) = delete;
  BasicZlibCompressionStrategy& operator=(const BasicZlibCompressionStrategy&) = delete;
  BasicZlibCompressionStrategy(BasicZlibCompressionStrategy&&) = delete;
  BasicZlibCompressionStrategy& operator=(BasicZlibCompressionStrategy&&) = delete;

  void DoCompress(buffer_t* buffer, size_t offset, const char* raw, size_t raw_size) override {
    TRACE("Zlib compress length %d", raw_size);

    zstream_compress_.next_in = reinterpret_cast<z_const Bytef*>(raw);
    zstream_compress_.avail_in = raw_size;  // set the z_stream's input
    size_t output_length = deflateBound(&zstream_compress_, raw_size);
    buffer->resize(output_length + offset);

    TRACE("Zlib compress output length %d", output_length);

    int ret;

    // compress bytes
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
    zstream_compress_.next_out = reinterpret_cast<Bytef*>(buffer->data() + offset);
    zstream_compress_.avail_out = buffer->size() - offset;

    ret = deflate(&zstream_compress_, Z_FINISH);

    output_length = zstream_compress_.total_out;

    // GCOVR_EXCL_START
    if (ret != Z_STREAM_END) {  // an error occurred that was not EOF
      std::ostringstream oss;
      // msg might be NULL
      oss << "Exception during zlib compression: (" << ret << ") "
          << (zstream_compress_.msg != nullptr ? zstream_compress_.msg : "unknown error");
      // reset, so that the instance can be reused
      deflateReset(&zstream_compress_);
      throw(std::runtime_error(oss.str()));
    }
    // GCOVR_EXCL_STOP
    deflateReset(&zstream_compress_);
    buffer->resize(output_length + offset);
  }

  std::string Decompress(const char* data, const size_t size) const override { return DoDecompress(data, size); }

  static std::string DoDecompress(const char* data, const size_t size) {
    z_stream zs;  // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, Format::kWindowBits) != Z_OK) {
      throw(std::runtime_error("inflateInit failed while decompressing."));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    zs.next_in = reinterpret_cast<z_const Bytef*>(data);
    zs.avail_in = size;

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
      zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
      zs.avail_out = sizeof(outbuffer);

      ret = inflate(&zs, 0);

      if (outstring.size() < zs.total_out) {
        outstring.append(outbuffer, zs.total_out - outstring.size());
      }
    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {  // an error occurred that was not EOF
      std::ostringstream oss;
      // msg might be NULL
      oss << "Exception during zlib decompression: (" << ret << ") "
          << (zs.msg != nullptr ? zs.msg : "unknown error");  // GCOVR_EXCL_BR_LINE
      throw(std::runtime_error(oss.str()));
    }

    return outstring;
  }

  [[nodiscard]] CompressionAlgorithm GetCompressionAlgorithm() const override { return Format::kAlgorithm; }

  [[nodiscard]] std::string name() const override { return Format::kName; }

  [[nodiscard]] uint64_t GetFileVersionMin() const override { return Format::kFileVersionMin; }

 private:
  z_stream zstream_compress_{};
};

using ZlibCompressionStrategy = BasicZlibCompressionStrategy<ZlibFormat>;
using ZlibDeflateCompressionStrategy = BasicZlibCompressionStrategy<ZlibDeflateFormat>;

} /* namespace compression */
} /* namespace keyvi */

#endif  // KEYVI_COMPRESSION_ZLIB_COMPRESSION_STRATEGY_H_
