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
 * serialization_utils.h
 *
 *  Created on: May 12, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_UTIL_SERIALIZATION_UTILS_H_
#define KEYVI_UTIL_SERIALIZATION_UTILS_H_

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

#include <boost/lexical_cast.hpp>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "keyvi/dictionary/util/endian.h"

namespace keyvi {
namespace util {

class SerializationUtils {
 public:
  static void ReadLengthPrefixedJsonRecord(std::istream& stream, rapidjson::Document* record) {
    uint32_t header_size;
    stream.read(reinterpret_cast<char*>(&header_size), sizeof(int));
    header_size = be32toh(header_size);
    auto buffer_ptr = std::unique_ptr<char[]>(new char[header_size]);
    stream.read(buffer_ptr.get(), header_size);
    record->Parse(buffer_ptr.get(), header_size);
  }

  // utility methods to retrieve numeric values
  // backwards compatibility: when using boost::property_tree numbers have been stored as string
  static uint64_t GetUint64FromValueOrString(const rapidjson::Document& record, const char* key) {
    if (record.HasMember(key)) {
      if (record[key].IsString()) {
        return boost::lexical_cast<uint64_t>(record[key].GetString());
      } else {
        return static_cast<uint64_t>(record[key].GetUint64());
      }
    }

    throw std::invalid_argument("key not found");
  }

  static uint64_t GetOptionalUInt64FromValueOrString(const rapidjson::Document& record, const char* key,
                                                     const size_t defaultValue) {
    if (record.HasMember(key)) {
      if (record[key].IsString()) {
        return boost::lexical_cast<uint64_t>(record[key].GetString());
      } else {
        return static_cast<uint64_t>(record[key].GetUint64());
      }
    }

    return defaultValue;
  }

  static size_t GetOptionalSizeFromValueOrString(const rapidjson::Document& record, const char* key,
                                                 const size_t defaultValue) {
    if (record.HasMember(key)) {
      if (record[key].IsString()) {
        return boost::lexical_cast<size_t>(record[key].GetString());
      } else {
        return static_cast<size_t>(record[key].GetUint64());
      }
    }

    return defaultValue;
  }
};

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_SERIALIZATION_UTILS_H_
