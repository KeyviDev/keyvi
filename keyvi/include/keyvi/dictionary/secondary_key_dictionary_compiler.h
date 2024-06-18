/* * keyvi - A key value store.
 *
 * Copyright 2024 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * secondary_key_dictionary_compiler.h
 *
 *  Created on: Jun 2, 2024
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_COMPILER_H_
#define KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_COMPILER_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "keyvi/dictionary/dictionary_compiler.h"
#include "keyvi/dictionary/fsa/internal/constants.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

/**
 * Secondary Key Dictionary Compiler
 *
 * @tparam ValueStoreType The type of the value store to use
 * @tparam N Array size for fixed size value vectors, ignored otherwise
 */
template <keyvi::dictionary::fsa::internal::value_store_t ValueStoreType = fsa::internal::value_store_t::KEY_ONLY>
class SecondaryKeyDictionaryCompiler final {
 public:
  using ValueStoreT = typename fsa::internal::ValueStoreComponents<ValueStoreType>::value_store_writer_t;

 public:
  /**
   * Instantiate a secondary key dictionary compiler.
   *
   * @param secondary_keys a list of secondary keys
   * @param params compiler parameters
   */
  explicit SecondaryKeyDictionaryCompiler(const std::vector<std::string>& secondary_keys,
                                          const keyvi::util::parameters_t& params = keyvi::util::parameters_t())
      : params_(params), dictionary_compiler_(params), secondary_keys_(secondary_keys) {}

  SecondaryKeyDictionaryCompiler& operator=(SecondaryKeyDictionaryCompiler const&) = delete;
  SecondaryKeyDictionaryCompiler(const SecondaryKeyDictionaryCompiler& that) = delete;

  void Add(const std::string& input_key, const std::map<std::string, std::string>& meta,
           typename ValueStoreT::value_t value = ValueStoreT::no_value) {
    std::ostringstream key_buffer;

    TRACE("add %s", input_key.c_str());

    for (auto const& key : secondary_keys_) {
      const std::string& value = meta.at(key);

      if (value.size() == 0) {
        key_buffer << static_cast<char>(1);
        continue;
      }

      auto pos = secondary_key_replacements_.find(value);
      if (pos == secondary_key_replacements_.end()) {
        // create a new entry
        replacements_buffer_.clear();
        keyvi::util::encodeVarInt(current_index_++, &replacements_buffer_);

        std::string replacement = std::string(replacements_buffer_.begin(), replacements_buffer_.end());
        TRACE("replace %s with: %s", value.c_str(), replacement.c_str());
        key_buffer << replacement;
        secondary_key_replacements_.emplace(value, std::move(replacement));
      } else {
        key_buffer << pos->second;
      }
    }
    key_buffer << input_key;

    TRACE("add %s", key_buffer.str().c_str());
    dictionary_compiler_.Add(key_buffer.str(), value);
  }

  /**
   * Do the final compilation
   */
  void Compile(DictionaryCompiler<>::callback_t progress_callback = nullptr, void* user_data = nullptr) {
    dictionary_compiler_.Compile(progress_callback, user_data);
  }

  /**
   * Set a custom manifest to be embedded into the index file.
   *
   * @param manifest as string
   */
  void SetManifest(const std::string& manifest) { dictionary_compiler_.SetManifest(manifest); }

  void Write(std::ostream& stream) {
    rapidjson::StringBuffer string_buffer;

    {
      rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

      writer.StartObject();
      writer.Key(SECONDARY_KEY_DICT_KEYS_PROPERTY);
      writer.StartArray();
      for (const std::string& key : secondary_keys_) {
        writer.String(key);
      }
      writer.EndArray();
      writer.EndObject();
    }
    dictionary_compiler_.SetSpecializedDictionaryProperties(string_buffer.GetString());
    dictionary_compiler_.Write(stream);

    keyvi::dictionary::DictionaryCompiler<fsa::internal::value_store_t::STRING> secondary_key_compiler(params_);

    for (auto const& [value, replacement] : secondary_key_replacements_) {
      secondary_key_compiler.Add(value, replacement);
    }
    secondary_key_compiler.Compile();
    secondary_key_compiler.Write(stream);
  }

  void WriteToFile(const std::string& filename) {
    std::ofstream out_stream = keyvi::util::OsUtils::OpenOutFileStream(filename);

    Write(out_stream);
    out_stream.close();
  }

 private:
  const keyvi::util::parameters_t params_;
  DictionaryCompiler<ValueStoreType> dictionary_compiler_;
  const std::vector<std::string> secondary_keys_;
  std::map<std::string, std::string> secondary_key_replacements_;
  uint64_t current_index_ = 2;  //  starting from 2, 1 is reserved for empty string
  std::vector<char> replacements_buffer_;
};

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_COMPILER_H_
