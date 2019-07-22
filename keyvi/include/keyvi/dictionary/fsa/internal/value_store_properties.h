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
 * value_store_properties.h
 *
 *  Created on: Nov 9, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_PROPERTIES_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_PROPERTIES_H_

#include <cstddef>
#include <fstream>
#include <string>

#include <boost/lexical_cast.hpp>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "keyvi/util/serialization_utils.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

static const char COMPRESSION_PROPERTY[] = "__compression";
static const char SIZE_PROPERTY[] = "size";
static const char UNIQUE_VALUES_PROPERTY[] = "unique_values";
static const char VALUES_PROPERTY[] = "values";

class ValueStoreProperties final {
 public:
  ValueStoreProperties() {}

  ValueStoreProperties(const size_t offset, const size_t size, const size_t number_of_values,
                       const size_t number_of_unique_values, const std::string& compression) {
    offset_ = offset;
    size_ = size;
    number_of_values_ = number_of_values;
    number_of_unique_values_ = number_of_unique_values;
    compression_ = compression;
  }

  size_t GetSize() const { return size_; }

  size_t GetOffset() const { return offset_; }

  size_t GetNumberOfValues() const { return number_of_values_; }

  size_t GetNumberOfUniqueValues() const { return number_of_unique_values_; }

  std::string GetStatistics() const {
    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

    writer.StartObject();
    GetStatistics(&writer);
    writer.EndObject();

    return string_buffer.GetString();
  }

  void GetStatistics(rapidjson::Writer<rapidjson::StringBuffer>* writer) const {
    writer->Key("Value Store");
    writer->StartObject();
    writer->Key(SIZE_PROPERTY);
    writer->Uint64(size_);
    writer->Key(VALUES_PROPERTY);
    writer->Uint64(number_of_values_);
    writer->Key(UNIQUE_VALUES_PROPERTY);
    writer->Uint64(number_of_unique_values_);
    if (compression_.size() > 0) {
      writer->Key(COMPRESSION_PROPERTY);
      writer->String(compression_);
    }
    writer->EndObject();
  }

  /**
   * Write as json using the version 2 binary format.
   *
   * Historically, version 2 used boost property trees which serialize everything as string,
   * that's why we do here as well.
   */
  void WriteAsJsonV2(std::ostream& stream) {
    rapidjson::StringBuffer string_buffer;

    {
      rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

      writer.StartObject();
      writer.Key(SIZE_PROPERTY);
      writer.String(std::to_string(size_));
      writer.Key(VALUES_PROPERTY);
      writer.String(std::to_string(number_of_values_));
      writer.Key(UNIQUE_VALUES_PROPERTY);
      writer.String(std::to_string(number_of_unique_values_));
      if (compression_.size() > 0) {
        writer.Key(COMPRESSION_PROPERTY);
        writer.String(compression_);
      }
      writer.EndObject();
    }

    uint32_t size = htobe32(string_buffer.GetLength());
    stream.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    stream.write(string_buffer.GetString(), string_buffer.GetLength());
  }

  static ValueStoreProperties FromJson(std::istream& stream) {
    rapidjson::Document value_store_properties;
    keyvi::util::SerializationUtils::ReadLengthPrefixedJsonRecord(stream, &value_store_properties);
    const size_t offset = stream.tellg();
    const size_t size =
        keyvi::util::SerializationUtils::GetOptionalSizeFromValueOrString(value_store_properties, SIZE_PROPERTY, 0);

    // check for file truncation
    if (size > 0) {
      stream.seekg(size - 1, stream.cur);
      if (stream.peek() == EOF) {
        throw std::invalid_argument("file is corrupt(truncated)");
      }
    }

    const size_t number_of_values =
        keyvi::util::SerializationUtils::GetOptionalUInt64FromValueOrString(value_store_properties, VALUES_PROPERTY, 0);
    const size_t number_of_unique_values = keyvi::util::SerializationUtils::GetOptionalUInt64FromValueOrString(
        value_store_properties, UNIQUE_VALUES_PROPERTY, 0);

    std::string compression;
    if (value_store_properties.HasMember(COMPRESSION_PROPERTY)) {
      compression = value_store_properties[COMPRESSION_PROPERTY].GetString();
    }

    return ValueStoreProperties(offset, size, number_of_values, number_of_unique_values, compression);
  }

 private:
  size_t offset_ = 0;
  size_t size_ = 0;
  size_t number_of_values_ = 0;
  size_t number_of_unique_values_ = 0;
  std::string compression_;
  std::string compression_threshold_;
};  // namespace internal

}  // namespace internal
}  // namespace fsa
}  // namespace dictionary
} /* namespace keyvi */

#endif  //  KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_PROPERTIES_H_
