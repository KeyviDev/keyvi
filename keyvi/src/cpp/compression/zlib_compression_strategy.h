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

#ifndef ZLIB_COMPRESSION_STRATEGY_H_
#define ZLIB_COMPRESSION_STRATEGY_H_

#include <string>
#include <zlib.h>

#include "compression/compression_strategy.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace compression {

/** A compression strategy that wraps zlib. */
struct ZlibCompressionStrategy final : public CompressionStrategy {
  ZlibCompressionStrategy(int compression_level = Z_BEST_COMPRESSION)
    : compression_level_(compression_level) {

    // init zlib structure for compression

    zstream_compress_.zalloc = Z_NULL;
    zstream_compress_.zfree = Z_NULL;
    zstream_compress_.opaque = Z_NULL;

    // see zlib.h level 9 means more speed for the price of memory
    const int mem_level = 9;

    if (deflateInit2(&zstream_compress_, compression_level, Z_DEFLATED, MAX_WBITS, mem_level, Z_DEFAULT_STRATEGY) != Z_OK) {
      throw std::bad_alloc();
    }
  }

  ~ZlibCompressionStrategy(){
    deflateEnd(&zstream_compress_);
  }

  inline void Compress(buffer_t& buffer, const char* raw, size_t raw_size) {
      DoCompress(buffer, raw, raw_size);
    }


  inline void DoCompress(buffer_t& buffer, const char* raw, size_t raw_size) {

    TRACE("Zlib compress length %d", raw_size);

    zstream_compress_.next_in = (Bytef*)raw;
    zstream_compress_.avail_in = raw_size;           // set the z_stream's input
    size_t output_length = deflateBound(&zstream_compress_, raw_size);
    buffer.resize(output_length + 1);


    TRACE("Zlib compress output length %d", output_length);

    buffer[0] = static_cast<char>(ZLIB_COMPRESSION);

    int ret;

    // compress bytes
    zstream_compress_.next_out = reinterpret_cast<Bytef*>(buffer.data() + 1);
    zstream_compress_.avail_out = buffer.size() - 1;

    ret = deflate(&zstream_compress_, Z_FINISH);

    output_length = zstream_compress_.total_out;

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
      std::ostringstream oss;
      oss << "Exception during zlib compression: (" << ret << ") " << zstream_compress_.msg;
      throw(std::runtime_error(oss.str()));
    }
    deflateReset(&zstream_compress_);
    buffer.resize(output_length + 1);
  }

  inline std::string Decompress(const std::string& compressed) {
    return DoDecompress(compressed);
  }

  static std::string DoDecompress(const std::string& compressed) {
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
      throw(std::runtime_error("inflateInit failed while decompressing."));

    zs.next_in = (Bytef*)compressed.data() + 1;
    zs.avail_in = compressed.size() - 1;

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

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
      std::ostringstream oss;
      oss << "Exception during zlib decompression: (" << ret << ") "
        << zs.msg;
      throw(std::runtime_error(oss.str()));
    }

    return outstring;
  }

  std::string name() const { return "zlib"; }

 private:
  int compression_level_;
  z_stream zstream_compress_;
  //char outbuffer_[32768];
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // ZLIB_COMPRESSION_STRATEGY_H_
