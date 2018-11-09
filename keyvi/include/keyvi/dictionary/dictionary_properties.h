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

#include "dictionary/fsa/internal/constants.h"
#include "dictionary/fsa/internal/value_store_properties.h"
#include "dictionary/fsa/internal/value_store_types.h"

#include "util/serialization_utils.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

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

  std::string GetStatistics() const { return ""; }

 private:
  std::string file_name_;
  uint64_t start_state_ = 0;
  uint64_t number_of_keys_ = 0;
  fsa::internal::value_store_t value_store_type_ = fsa::internal::value_store_t::KEY_ONLY;
  size_t sparse_array_size_ = 0;
  size_t persistence_offset_ = 0;
  size_t transitions_offset_ = 0;
  fsa::internal::ValueStoreProperties value_store_properties_;
  std::string manifest_;

  void ReadJsonFormat(std::ifstream& file_stream) {
    rapidjson::Document automata_properties;

    keyvi::util::SerializationUtils::ReadJsonRecord(file_stream, automata_properties);

    if (boost::lexical_cast<int>(automata_properties["version"].GetString()) < KEYVI_FILE_VERSION_MIN) {
      throw std::invalid_argument("this version of keyvi file is unsupported");
    }

    start_state_ = boost::lexical_cast<uint64_t>(automata_properties["start_state"].GetString());

    TRACE("start state %d", start_state_);

    number_of_keys_ = boost::lexical_cast<uint64_t>(automata_properties["number_of_keys"].GetString());
    value_store_type_ = static_cast<fsa::internal::value_store_t>(
        boost::lexical_cast<int>(automata_properties["value_store_type"].GetString()));

    if (automata_properties.HasMember("manifest")) {
      if (automata_properties["manifest"].IsString()) {
        // normally manifest is a string
        manifest_ = automata_properties["manifest"].GetString();
      } else if (automata_properties["manifest"].IsObject()) {
        // backwards compatibility: manifest might be a subtree
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        automata_properties["manifest"].Accept(writer);
        manifest_ = sb.GetString();
      }
    }

    rapidjson::Document sparse_array_properties;
    keyvi::util::SerializationUtils::ReadJsonRecord(file_stream, sparse_array_properties);

    if (boost::lexical_cast<int>(sparse_array_properties["version"].GetString()) < KEYVI_FILE_PERSISTENCE_VERSION_MIN) {
      throw std::invalid_argument("unsupported keyvi file version");
    }

    persistence_offset_ = file_stream.tellg();

    const size_t bucket_size = sizeof(uint16_t);
    sparse_array_size_ = boost::lexical_cast<size_t>(sparse_array_properties["size"].GetString());

    transitions_offset_ = persistence_offset_ + sparse_array_size_;

    // check for file truncation
    file_stream.seekg((size_t)file_stream.tellg() + sparse_array_size_ + bucket_size * sparse_array_size_ - 1);
    if (file_stream.peek() == EOF) {
      throw std::invalid_argument("file is corrupt(truncated)");
    }

    file_stream.get();

    // not all value stores have properties
    if (file_stream.peek() != EOF) {
      value_store_properties_ = fsa::internal::ValueStoreProperties::FromJsonStream(file_stream);
    }
  }
};
}  // namespace dictionary
}  // namespace keyvi

#endif  // KEYVI_DICTIONARY_DICTIONARY_PROPERTIES_H_
