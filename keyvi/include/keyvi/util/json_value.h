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
 * json_value.h
 *
 *  Created on: Dec 8, 2015
 *      Author: hendrik
 */

#ifndef KEYVI_UTIL_JSON_VALUE_H_
#define KEYVI_UTIL_JSON_VALUE_H_

#include <string>

#include "keyvi/compression/compression_selector.h"
#include "keyvi/util/msgpack_util.h"
#include "msgpack.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace util {

/** Decompresses (if needed) and decodes a json value stored in a JsonValueStore. */
inline std::string DecodeJsonValue(const std::string& encoded_value) {
  compression::decompress_func_t decompressor = compression::decompressor_by_code(encoded_value);
  std::string packed_string = decompressor(encoded_value);
  TRACE("unpacking %s", packed_string.c_str());

  msgpack::object_handle doc;
  msgpack::unpack(doc, packed_string.data(), packed_string.size());

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator,
                    rapidjson::kWriteNanAndInfFlag>
      writer(buffer);
  MsgPackDump(&writer, doc.get());
  return buffer.GetString();
}

inline void EncodeJsonValue(std::function<void(compression::buffer_t*, const char*, size_t)> long_compress,
                            std::function<void(compression::buffer_t*, const char*, size_t)> short_compress,
                            msgpack::sbuffer* msgpack_buffer, compression::buffer_t* buffer,
                            const std::string& raw_value, bool single_precision_float = false,
                            size_t compression_threshold = 32) {
  msgpack_buffer->clear();

  rapidjson::Document json_document;
  json_document.Parse<rapidjson::kParseNanAndInfFlag>(raw_value.c_str());

  if (!json_document.HasParseError()) {
    TRACE("Got json");
    msgpack::packer<msgpack::sbuffer> packer(msgpack_buffer);
    JsonToMsgPack(json_document, &packer, single_precision_float);
  } else {
    TRACE("Got a normal string");
    msgpack::pack(msgpack_buffer, raw_value);
  }
  // compression
  if (msgpack_buffer->size() > compression_threshold) {
    long_compress(buffer, msgpack_buffer->data(), msgpack_buffer->size());
  } else {
    short_compress(buffer, msgpack_buffer->data(), msgpack_buffer->size());
  }
}

/**
 * Encodes @p raw_value with msgpack and compresses it, if it is longer than
 * the specified threshold.
 *
 * @note This is a default implementation that uses snappy for string longer
 *       than 32 characters.
 */
inline std::string EncodeJsonValue(const std::string& raw_value, bool single_precision_float = false,
                                   size_t compression_threshold = 32) {
  msgpack::sbuffer msgpack_buffer;
  compression::buffer_t buffer;

  EncodeJsonValue(static_cast<void (*)(compression::buffer_t*, const char*, size_t)>(
                      &compression::SnappyCompressionStrategy::DoCompress),
                  static_cast<void (*)(compression::buffer_t*, const char*, size_t)>(
                      &compression::RawCompressionStrategy::DoCompress),
                  &msgpack_buffer, &buffer, raw_value, single_precision_float, compression_threshold);
  return std::string(reinterpret_cast<char*>(buffer.data()), buffer.size());
}

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_JSON_VALUE_H_
