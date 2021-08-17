/* * keyvi - A key value store.
 *
 * Copyright 2018   Narek Gharibyan<narekgharibyan@gmail.com>
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
 *  vector_file.h
 *
 *  Created on: March 17, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#ifndef KEYVI_VECTOR_VECTOR_FILE_H_
#define KEYVI_VECTOR_VECTOR_FILE_H_

#include <exception>
#include <fstream>
#include <memory>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "keyvi/dictionary/fsa/internal/value_store_factory.h"
#include "keyvi/dictionary/fsa/internal/value_store_properties.h"
#include "keyvi/util/os_utils.h"
#include "keyvi/util/serialization_utils.h"
#include "keyvi/vector/types.h"

static const char KEYVI_VECTOR_BEGIN[] = "KEYVI_VECTOR_BEGIN";
static const size_t KEYVI_VECTOR_BEGIN_LEN = 18;
static const char KEYVI_VECTOR_END[] = "KEYVI_VECTOR_END";
static const size_t KEYVI_VECTOR_END_LEN = 16;
static const char MANIFEST_LABEL[] = "manifest";
static const char SIZE_LABEL[] = "size";
static const char VALUE_STORE_TYPE_LABEl[] = "value_store_type";

namespace keyvi {
namespace vector {

template <value_store_t>
class Vector;

class VectorFile {
  template <value_store_t>
  friend class Vector;

  using mapped_region = boost::interprocess::mapped_region;
  using IValueStoreReader = dictionary::fsa::internal::IValueStoreReader;

 public:
  explicit VectorFile(const std::string& filename, const value_store_t value_store_type) {
    const auto loading_strategy = dictionary::loading_strategy_types::lazy_no_readahead;
    std::ifstream in_stream(filename, std::ios::binary);

    CheckValidity(&in_stream);

    in_stream.seekg(KEYVI_VECTOR_BEGIN_LEN);

    rapidjson::Document file_properties;
    keyvi::util::SerializationUtils::ReadLengthPrefixedJsonRecord(in_stream, &file_properties);

    value_store_t value_store_type_from_file = static_cast<value_store_t>(
        keyvi::util::SerializationUtils::GetUint64FromValueOrString(file_properties, VALUE_STORE_TYPE_LABEl));

    if (value_store_type != value_store_type_from_file) {
      throw std::invalid_argument("wrong vector file");
    }

    if (file_properties.HasMember(MANIFEST_LABEL)) {
      if (file_properties[MANIFEST_LABEL].IsString()) {
        // manifest is a string
        manifest_ = file_properties[MANIFEST_LABEL].GetString();
      }
    }

    rapidjson::Document index_properties;
    keyvi::util::SerializationUtils::ReadLengthPrefixedJsonRecord(in_stream, &index_properties);

    size_ = keyvi::util::SerializationUtils::GetOptionalSizeFromValueOrString(index_properties, SIZE_LABEL, 0);
    const auto index_size = size_ * sizeof(offset_type);

    auto file_mapping = boost::interprocess::file_mapping(filename.c_str(), boost::interprocess::read_only);
    const auto map_options = dictionary::fsa::internal::MemoryMapFlags::FSAGetMemoryMapOptions(loading_strategy);
    index_region_ = mapped_region(file_mapping, boost::interprocess::read_only, in_stream.tellg(), index_size, nullptr,
                                  map_options);
    const auto advise = dictionary::fsa::internal::MemoryMapFlags::FSAGetMemoryMapAdvices(loading_strategy);
    index_region_.advise(advise);

    in_stream.seekg(size_t(in_stream.tellg()) + index_size);

    dictionary::fsa::internal::ValueStoreProperties value_store_properties;
    // not all value stores have properties
    if (in_stream.peek() != EOF) {
      value_store_properties = dictionary::fsa::internal::ValueStoreProperties::FromJson(in_stream);
    }

    value_store_reader_.reset(dictionary::fsa::internal::ValueStoreFactory::MakeReader(
        value_store_type, &file_mapping, value_store_properties, loading_strategy));
  }

  template <typename ValueStoreT>
  static void WriteToFile(const std::string& filename, const std::string& manifest,
                          const std::unique_ptr<MemoryMapManager>& index_store, const size_t size,
                          const std::unique_ptr<ValueStoreT>& value_store) {
    std::ofstream out_stream = keyvi::util::OsUtils::OpenOutFileStream(filename);

    out_stream.write(KEYVI_VECTOR_BEGIN, KEYVI_VECTOR_BEGIN_LEN);

    rapidjson::StringBuffer string_buffer;
    {
      rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

      writer.StartObject();
      writer.Key("file_version");
      writer.String(std::to_string(1));
      writer.Key(VALUE_STORE_TYPE_LABEl);
      writer.String(std::to_string(static_cast<int>(value_store->GetValueStoreType())));
      writer.Key("index_version");
      writer.String(std::to_string(1));

      // manifest
      writer.Key(MANIFEST_LABEL);
      writer.String(manifest);
      writer.EndObject();
    }

    uint32_t header_size = htobe32(string_buffer.GetLength());
    out_stream.write(reinterpret_cast<const char*>(&header_size), sizeof(uint32_t));
    out_stream.write(string_buffer.GetString(), string_buffer.GetLength());

    string_buffer.Clear();
    {
      rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

      writer.StartObject();
      writer.Key(SIZE_LABEL);
      writer.String(std::to_string(size));
      writer.EndObject();
    }

    header_size = htobe32(string_buffer.GetLength());
    out_stream.write(reinterpret_cast<const char*>(&header_size), sizeof(uint32_t));
    out_stream.write(string_buffer.GetString(), string_buffer.GetLength());

    index_store->Write(out_stream, index_store->GetSize());
    value_store->Write(out_stream);

    out_stream.write(KEYVI_VECTOR_END, KEYVI_VECTOR_END_LEN);

    out_stream.close();
  }

 private:
  void CheckValidity(std::ifstream* in_stream) const {
    if (!in_stream->good()) {
      throw std::invalid_argument("vector file not found");
    }
    char magic_start[KEYVI_VECTOR_BEGIN_LEN];
    in_stream->read(magic_start, KEYVI_VECTOR_BEGIN_LEN);
    if (std::strncmp(magic_start, KEYVI_VECTOR_BEGIN, KEYVI_VECTOR_BEGIN_LEN)) {
      throw std::invalid_argument("not a keyvi vector file");
    }

    in_stream->seekg(-KEYVI_VECTOR_END_LEN, std::ios_base::end);
    char magic_end[KEYVI_VECTOR_END_LEN];
    in_stream->read(magic_end, KEYVI_VECTOR_END_LEN);
    if (std::strncmp(magic_end, KEYVI_VECTOR_END, KEYVI_VECTOR_END_LEN)) {
      throw std::invalid_argument("the file is corrupt(truncated)");
    }
  }

 private:
  mapped_region index_region_;
  std::unique_ptr<IValueStoreReader> value_store_reader_;

  size_t size_;

  std::string manifest_;
};

} /* namespace vector */
} /* namespace keyvi */

#endif  //  KEYVI_VECTOR_VECTOR_FILE_H_
