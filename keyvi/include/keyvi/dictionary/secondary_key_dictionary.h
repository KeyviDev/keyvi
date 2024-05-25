/** keyvi - A key value store.
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
 * secondary_key_dictionary.h
 *
 *  Created on: May 25, 2024
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_H_
#define KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_H_

#include "keyvi/dictionary/dictionary.h"

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
                                  loading_strategy_types loading_strategy = loading_strategy_types::lazy)
      : SecondaryKeyDictionary(std::make_shared<fsa::Automata>(filename, loading_strategy)) {}

  //
  explicit SecondaryKeyDictionary(fsa::automata_t f) : dictionary_(std::make_shared<Dictionary>(f)) {
    // TODO: load map of secondary keys from dictionary properties and initialize lookup table
  }

  /**
   * A simple Contains method to check whether a key is in the dictionary.
   *
   * @param key The key
   * @return True if key is in the dictionary, False otherwise.
   */
  bool Contains(const std::string& key, const std::map<std::string, std::string>& meta) const { return false; }

  Match GetFirst(const std::string& key, const std::map<std::string, std::string>& meta) const {
    // TODO: construct the secondary key from the given meta data and move the start state accordingly
    uint64_t start_state = dictionary_->GetFsa()->GetStartState();
    return dictionary_->GetSubscript(key, start_state);
  }

  /**
   * Exact Match function.
   *
   * @param key the key to lookup.
   * @return a match iterator
   */
  MatchIterator::MatchIteratorPair Get(const std::string& key, const std::map<std::string, std::string>& meta) const {
    return MatchIterator::EmptyIteratorPair();
  }

  /**
   * All the items in the dictionary.
   *
   * @return a match iterator of all the items
   */
  MatchIterator::MatchIteratorPair GetAllItems(const std::map<std::string, std::string>& meta) const {
    return MatchIterator::EmptyIteratorPair();
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
    return MatchIterator::EmptyIteratorPair();
  }

  MatchIterator::MatchIteratorPair GetFuzzy(const std::string& query, const std::map<std::string, std::string>& meta,
                                            const int32_t max_edit_distance,
                                            const size_t minimum_exact_prefix = 2) const {
    return MatchIterator::EmptyIteratorPair();
  }

  MatchIterator::MatchIteratorPair GetPrefixCompletion(const std::string& query,
                                                       const std::map<std::string, std::string>& meta) const {
    return MatchIterator::EmptyIteratorPair();
  }

  MatchIterator::MatchIteratorPair GetPrefixCompletion(const std::string& query,
                                                       const std::map<std::string, std::string>& meta,
                                                       size_t top_n) const {
    return MatchIterator::EmptyIteratorPair();
  }

  MatchIterator::MatchIteratorPair GetMultiwordCompletion(const std::string& query,
                                                          const std::map<std::string, std::string>& meta,
                                                          const unsigned char multiword_separator = 0x1b) const {
    // TODO: construct the secondary key from the given meta data and move the start state accordingly
    uint64_t start_state = dictionary_->GetFsa()->GetStartState();
    return dictionary_->GetMultiWordCompletion(start_state, query, multiword_separator);
  }

  MatchIterator::MatchIteratorPair GetMultiwordCompletion(const std::string& query,
                                                          const std::map<std::string, std::string>& meta, size_t top_n,
                                                          const unsigned char multiword_separator = 0x1b) const {
    return MatchIterator::EmptyIteratorPair();
  }

  MatchIterator::MatchIteratorPair GetFuzzyMultiwordCompletion(const std::string& query,
                                                               const std::map<std::string, std::string>& meta,
                                                               const int32_t max_edit_distance,
                                                               const size_t minimum_exact_prefix = 0,
                                                               const unsigned char multiword_separator = 0x1b) const {
    return MatchIterator::EmptyIteratorPair();
  }

  std::string GetManifest() const { return dictionary_->GetManifest(); }

 private:
  dictionary_t dictionary_;
};

// shared pointer
typedef std::shared_ptr<SecondaryKeyDictionary> secondary_key_dictionary_t;

} /* namespace dictionary */
} /* namespace keyvi */

#endif  //  KEYVI_DICTIONARY_SECONDARY_KEY_DICTIONARY_H_
