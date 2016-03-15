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
 * compression_selector.h
 *
 *  Created on: October 6, 2015
 *      Author: David Mark Nemeskey<nemeskey.david@gmail.com>
 */

#ifndef COMPRESSION_SELECTOR_H_
#define COMPRESSION_SELECTOR_H_

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "compression/compression_strategy.h"
#include "compression/zlib_compression_strategy.h"
#include "compression/snappy_compression_strategy.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace compression {

/** Returns an instance of a compression strategy by name. */
inline CompressionStrategy* compression_strategy(const std::string& name = "")
    throw (std::invalid_argument) {
  // inline to avoid "multiple definition of" errors that come from defining
  // the function in a header file
  auto lower_name = name;
  // NOTE: this does not work for non-ASCII strings
  boost::algorithm::to_lower(lower_name);
  if (lower_name == "zip" || lower_name == "zlib" || lower_name == "z") {
    return new ZlibCompressionStrategy();  // TODO: compression level?
  } else if (lower_name == "snappy") {
    return new SnappyCompressionStrategy();
  } else if (lower_name == "" || lower_name == "none" || lower_name == "raw") {
    return new RawCompressionStrategy();
  } else {
    throw std::invalid_argument(name + " is not a valid compression");
  }
}

typedef std::string (*decompress_func_t) (const std::string&);
typedef void (CompressionStrategy::*compress_mem_fn_t) (buffer_t&, const char*, size_t);


inline decompress_func_t decompressor_by_code(const std::string& s) {
  switch (s[0]) {
    case NO_COMPRESSION:
      TRACE("unpack uncompressed string");
      return RawCompressionStrategy::DoDecompress;
    case ZLIB_COMPRESSION:
      TRACE("unpack zlib compressed string");
      return ZlibCompressionStrategy::DoDecompress;
    case SNAPPY_COMPRESSION:
      TRACE("unpack snappy compressed string");
      return SnappyCompressionStrategy::DoDecompress;
    default:
      throw std::invalid_argument("Invalid compression code " +
          boost::lexical_cast<std::string>(static_cast<int>(s[0])));
  }
}

} /* namespace compression */
} /* namespace keyvi */

#endif  // COMPRESSION_SELECTOR_H_
