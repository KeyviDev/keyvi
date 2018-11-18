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
 * dictionary_properties.h
 *
 *  Created on: Sep 24, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_DICTIONARY_PROPERTIES_H_
#define KEYVI_DICTIONARY_DICTIONARY_PROPERTIES_H_

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

#include <boost/lexical_cast.hpp>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "dictionary/fsa/internal/constants.h"
#include "dictionary/fsa/internal/value_store_properties.h"
#include "dictionary/fsa/internal/value_store_types.h"

#include "util/serialization_utils.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

static const char START_STATE_PROPERTY[] = "start_state";
static const char VERSION_PROPERTY[] = "version";
static const char MANIFEST_PROPERTY[] = "manifest";
static const char NUMBER_OF_KEYS_PROPERTY[] = "number_of_keys";
static const char VALUE_STORE_TYPE_PROPERTY[] = "value_store_type";
static const char NUMBER_OF_STATES_PROPERTY[] = "number_of_states";
static const char SIZE_PROPERTY[] = "size";

class DictionaryProperties {
 public:
  DictionaryProperties() {}

  explicit DictionaryProperties(const std::string& file_name) : file_name_(file_name) {
    std::ifstream file_stream(file_name, std::ios::binary);

    if (!file_stream.good()) {
      throw std::invalid_argument("file not found");
    }

    char magic[KEYVI_FILE_MAGIC_LEN];
    file_stream.read(magic, KEYVI_FILE_MAGIC_LEN);

    // check magic
    if (std::strncmp(magic, KEYVI_FILE_MAGIC, KEYVI_FILE_MAGIC_LEN) == 0) {
      ReadJsonFormat(file_stream);

      return;
    }
    throw std::invalid_argument("not a keyvi file");
  }

  const std::string& GetFileName() const { return file_name_; }

  uint64_t GetStartState() const { return start_state_; }

  uint64_t GetNumberOfKeys() const { return number_of_keys_; }

  fsa::internal::value_store_t GetValueStoreType() const { return value_store_type_; }

  size_t GetSparseArraySize() const { return sparse_array_size_; }

  size_t GetPersistenceOffset() const { return persistence_offset_; }

  size_t GetTransitionsOffset() const { return transitions_offset_; }

  size_t GetTransitionsSize() const { return sparse_array_size_ * 2; }

  const fsa::internal::ValueStoreProperties& GetValueStoreProperties() const { return value_store_properties_; }

  const std::string GetManifest() const { return manifest_; }

  std::string GetStatistics() const {
    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

    writer.StartObject();

    writer.Key("General");
    writer.StartObject();
    writer.Key(VERSION_PROPERTY);
    writer.Uint64(version_);
    writer.Key(START_STATE_PROPERTY);
    writer.Uint64(start_state_);
    writer.Key(NUMBER_OF_KEYS_PROPERTY);
    writer.Uint64(number_of_keys_);
    writer.Key(VALUE_STORE_TYPE_PROPERTY);
    writer.Uint64(static_cast<uint64_t>(value_store_type_));
    writer.Key(NUMBER_OF_STATES_PROPERTY);
    writer.Uint64(number_of_states_);
    writer.EndObject();

    writer.Key("Persistence");
    writer.StartObject();
    writer.Key(VERSION_PROPERTY);
    writer.Uint64(sparse_array_version_);
    writer.Key(SIZE_PROPERTY);
    writer.Uint64(sparse_array_size_);
    writer.EndObject();

    value_store_properties_.GetStatistics(&writer);
    writer.EndObject();
    return string_buffer.GetString();
  }

 private:
  std::string file_name_;
  uint64_t version_ = 0;
  uint64_t start_state_ = 0;
  uint64_t number_of_keys_ = 0;
  uint64_t number_of_states_ = 0;
  fsa::internal::value_store_t value_store_type_ = fsa::internal::value_store_t::KEY_ONLY;
  uint64_t sparse_array_version_ = 0;
  size_t sparse_array_size_ = 0;
  size_t persistence_offset_ = 0;
  size_t transitions_offset_ = 0;
  fsa::internal::ValueStoreProperties value_store_properties_;
  std::string manifest_;

  void ReadJsonFormat(std::ifstream& file_stream) {
    rapidjson::Document automata_properties;

    keyvi::util::SerializationUtils::ReadJsonRecord(file_stream, &automata_properties);

    version_ = keyvi::util::SerializationUtils::GetUint64FromValueOrString(automata_properties, VERSION_PROPERTY);
    if (version_ < KEYVI_FILE_VERSION_MIN) {
      throw std::invalid_argument("this version of keyvi file is unsupported");
    }

    start_state_ =
        keyvi::util::SerializationUtils::GetUint64FromValueOrString(automata_properties, START_STATE_PROPERTY);

    TRACE("start state %d", start_state_);

    number_of_keys_ =
        keyvi::util::SerializationUtils::GetUint64FromValueOrString(automata_properties, NUMBER_OF_KEYS_PROPERTY);
    value_store_type_ = static_cast<fsa::internal::value_store_t>(
        keyvi::util::SerializationUtils::GetUint64FromValueOrString(automata_properties, VALUE_STORE_TYPE_PROPERTY));
    number_of_states_ =
        keyvi::util::SerializationUtils::GetUint64FromValueOrString(automata_properties, NUMBER_OF_STATES_PROPERTY);

    if (automata_properties.HasMember(MANIFEST_PROPERTY)) {
      if (automata_properties[MANIFEST_PROPERTY].IsString()) {
        // normally manifest is a string
        manifest_ = automata_properties[MANIFEST_PROPERTY].GetString();
      } else if (automata_properties[MANIFEST_PROPERTY].IsObject()) {
        // backwards compatibility: manifest might be a subtree
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        automata_properties[MANIFEST_PROPERTY].Accept(writer);
        manifest_ = sb.GetString();
      }
    }

    rapidjson::Document sparse_array_properties;
    keyvi::util::SerializationUtils::ReadJsonRecord(file_stream, &sparse_array_properties);

    sparse_array_version_ =
        keyvi::util::SerializationUtils::GetUint64FromValueOrString(sparse_array_properties, VERSION_PROPERTY);
    if (sparse_array_version_ < KEYVI_FILE_PERSISTENCE_VERSION_MIN) {
      throw std::invalid_argument("unsupported keyvi file version");
    }

    persistence_offset_ = file_stream.tellg();

    const size_t bucket_size = sizeof(uint16_t);
    sparse_array_size_ =
        keyvi::util::SerializationUtils::GetOptionalSizeFromValueOrString(sparse_array_properties, SIZE_PROPERTY, 0);

    transitions_offset_ = persistence_offset_ + sparse_array_size_;

    // check for file truncation
    file_stream.seekg((size_t)file_stream.tellg() + sparse_array_size_ + bucket_size * sparse_array_size_ - 1);
    if (file_stream.peek() == EOF) {
      throw std::invalid_argument("file is corrupt(truncated)");
    }

    file_stream.get();

    // not all value stores have properties
    if (file_stream.peek() != EOF) {
      value_store_properties_ = fsa::internal::ValueStoreProperties::FromJson(file_stream);
    }
  }
};
}  // namespace dictionary
}  // namespace keyvi

#endif  // KEYVI_DICTIONARY_DICTIONARY_PROPERTIES_H_
