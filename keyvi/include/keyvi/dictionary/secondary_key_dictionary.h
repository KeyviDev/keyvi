/** keyvi - A key value store.
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
 * secondary_key_dictionary.h
 *
 *  Created on: May 25, 2024
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_H_
#define KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "rapidjson/document.h"

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/transform/fsa_transform.h"
#include "keyvi/util/vint.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

class SecondaryKeyDictionary final {
 public:
  /**
   * Initialize a dictionary from a file.
   *
   * @param filename filename to load keyvi file from.
   * @param loading_strategy optional: Loading strategy to use.
   */
  explicit SecondaryKeyDictionary(const std::string& filename,
                                  const loading_strategy_types loading_strategy = loading_strategy_types::lazy)
      : SecondaryKeyDictionary(std::make_shared<fsa::Automata>(filename, loading_strategy), loading_strategy) {}

  explicit SecondaryKeyDictionary(const fsa::automata_t& f,
                                  const loading_strategy_types loading_strategy = loading_strategy_types::lazy)
      : dictionary_(std::make_shared<Dictionary>(f)) {
    std::string properties = dictionary_->GetFsa()->GetDictionaryProperties()->GetSpecializedDictionaryProperties();

    TRACE("Specialized properties: %s", properties.c_str());

    rapidjson::Document parsed_properties;
    parsed_properties.Parse(properties);
    if (!parsed_properties.IsObject() || !parsed_properties.HasMember(SECONDARY_KEY_DICT_KEYS_PROPERTY) ||
        !parsed_properties[SECONDARY_KEY_DICT_KEYS_PROPERTY].IsArray()) {
      throw std::invalid_argument("not a secondary key dict");
    }

    secondary_key_replacement_dict_ = std::make_shared<Dictionary>(std::make_shared<fsa::Automata>(
        dictionary_->GetFsa()->GetDictionaryProperties()->GetFileName(),
        dictionary_->GetFsa()->GetDictionaryProperties()->GetEndOffset(), loading_strategy));

    for (auto const& value : parsed_properties[SECONDARY_KEY_DICT_KEYS_PROPERTY].GetArray()) {
      secondary_keys_.push_back(value.GetString());
    }
  }

  std::string GetStatistics() const { return dictionary_->GetStatistics(); }

  uint64_t GetSize() const { return dictionary_->GetSize(); }

  /**
   * A simple Contains method to check whether a key is in the dictionary.
   *
   * @param key The key
   * @return True if key is in the dictionary, False otherwise.
   */
  bool Contains(const std::string& key, const std::map<std::string, std::string>& meta) const {
    return dictionary_->Contains(GetStartState(meta), key);
  }

  match_t GetFirst(const std::string& key, const std::map<std::string, std::string>& meta) const {
    return dictionary_->GetSubscript(GetStartState(meta), key);
  }

  /**
   * Exact Match function.
   *
   * @param key the key to lookup.
   * @return a match iterator
   */
  MatchIterator::MatchIteratorPair Get(const std::string& key, const std::map<std::string, std::string>& meta) const {
    return dictionary_->Get(GetStartState(meta), key);
  }

  /**
   * All the items in the dictionary.
   *
   * @return a match iterator of all the items
   */
  MatchIterator::MatchIteratorPair GetAllItems(const std::map<std::string, std::string>& meta) const {
    return dictionary_->GetAllItems(GetStartState(meta));
  }

  /**
   * Match a key near: Match as much as possible exact given the minimum prefix length and then return everything below.
   *
   * If greedy is True it matches everything below the minimum_prefix_length, but in the order of exact first.
   *
   * @param key
   * @param minimum_prefix_length
   * @param greedy if true matches everything below minimum prefix
   * @return
   */
  MatchIterator::MatchIteratorPair GetNear(const std::string& key, const std::map<std::string, std::string>& meta,
                                           const size_t minimum_prefix_length, const bool greedy = false) const {
    return dictionary_->GetNear(GetStartState(meta), key, minimum_prefix_length, greedy);
  }

  MatchIterator::MatchIteratorPair GetFuzzy(const std::string& query, const std::map<std::string, std::string>& meta,
                                            const int32_t max_edit_distance,
                                            const size_t minimum_exact_prefix = 2) const {
    return dictionary_->GetFuzzy(GetStartState(meta), query, max_edit_distance, minimum_exact_prefix);
  }

  MatchIterator::MatchIteratorPair GetPrefixCompletion(const std::string& query,
                                                       const std::map<std::string, std::string>& meta) const {
    return dictionary_->GetPrefixCompletion(GetStartState(meta), query);
  }

  MatchIterator::MatchIteratorPair GetPrefixCompletion(const std::string& query,
                                                       const std::map<std::string, std::string>& meta,
                                                       size_t top_n) const {
    return dictionary_->GetPrefixCompletion(GetStartState(meta), query, top_n);
  }

  MatchIterator::MatchIteratorPair GetMultiwordCompletion(const std::string& query,
                                                          const std::map<std::string, std::string>& meta,
                                                          const unsigned char multiword_separator = 0x1b) const {
    return dictionary_->GetMultiwordCompletion(GetStartState(meta), query, multiword_separator);
  }

  MatchIterator::MatchIteratorPair GetMultiwordCompletion(const std::string& query,
                                                          const std::map<std::string, std::string>& meta, size_t top_n,
                                                          const unsigned char multiword_separator = 0x1b) const {
    return dictionary_->GetMultiwordCompletion(GetStartState(meta), query, top_n, multiword_separator);
  }

  MatchIterator::MatchIteratorPair GetFuzzyMultiwordCompletion(const std::string& query,
                                                               const std::map<std::string, std::string>& meta,
                                                               const int32_t max_edit_distance,
                                                               const size_t minimum_exact_prefix = 0,
                                                               const unsigned char multiword_separator = 0x1b) const {
    return dictionary_->GetFuzzyMultiwordCompletion(GetStartState(meta), query, max_edit_distance, minimum_exact_prefix,
                                                    multiword_separator);
  }

  std::string GetManifest() const { return dictionary_->GetManifest(); }

 private:
  dictionary_t dictionary_;
  dictionary_t secondary_key_replacement_dict_;
  std::vector<std::string> secondary_keys_;

  uint64_t GetStartState(const std::map<std::string, std::string>& meta) const {
    uint64_t state = dictionary_->GetFsa()->GetStartState();

    for (auto const& key : secondary_keys_) {
      TRACE("match secondary key: %s", key.c_str());
      auto pos = meta.find(key);
      if (pos == meta.end()) {
        return 0;
      }

      // edge case: empty value
      if (pos->second.size() == 0) {
        TRACE("match empty string");
        state = dictionary_->GetFsa()->TryWalkTransition(state, static_cast<char>(1));
        continue;
      }

      match_t m = secondary_key_replacement_dict_->operator[](pos->second);
      if (!m) {
        return 0;
      }

      TRACE("found secondary replacement");

      for (auto c : m->GetValueAsString()) {
        state = dictionary_->GetFsa()->TryWalkTransition(state, c);
        if (!state) {
          return 0;
        }
      }
    }

    TRACE("state after secondary key matches: %d", state);

    return state;
  }
};

// shared pointer
typedef std::shared_ptr<SecondaryKeyDictionary> secondary_key_dictionary_t;

} /* namespace dictionary */
} /* namespace keyvi */

#endif  //  KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_H_
