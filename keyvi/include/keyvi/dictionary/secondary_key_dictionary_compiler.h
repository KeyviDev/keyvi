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
#include <vector>

#include "keyvi/dictionary/dictionary_compiler.h"

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
  explicit SecondaryKeyDictionaryCompiler(const std::vector<std::string> secondary_keys,
                                          const keyvi::util::parameters_t& params = keyvi::util::parameters_t())
      : dictionary_compiler_(params), secondary_keys_(secondary_keys) {}

  SecondaryKeyDictionaryCompiler& operator=(SecondaryKeyDictionaryCompiler const&) = delete;
  SecondaryKeyDictionaryCompiler(const SecondaryKeyDictionaryCompiler& that) = delete;

  void Add(const std::string& input_key, const std::map<std::string, std::string>& meta,
           typename ValueStoreT::value_t value = ValueStoreT::no_value) {
    dictionary_compiler_.Add(input_key, value);
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
  void SetManifest(const std::string& manifest) { dictionary_compiler_.SetManifest; }

  void Write(std::ostream& stream) { dictionary_compiler_.Write(stream); }

  void WriteToFile(const std::string& filename) { dictionary_compiler_.WriteToFile(filename); }

 private:
  DictionaryCompiler<ValueStoreType> dictionary_compiler_;
  std::vector<std::string> secondary_keys_;
  std::map<std::string, std::string> secondary_key_replacements_;
};

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_COMPILER_H_
