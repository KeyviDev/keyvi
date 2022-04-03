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
 * dictionary.h
 *
 *  Created on: Jun 3, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_DICTIONARY_H_
#define KEYVI_DICTIONARY_DICTIONARY_H_

#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/state_traverser.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/match_iterator.h"
#include "keyvi/dictionary/matching/fuzzy_matching.h"
#include "keyvi/dictionary/matching/near_matching.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

class Dictionary final {
 public:
  /**
   * Initialize a dictionary from a file.
   *
   * @param filename filename to load keyvi file from.
   * @param loading_strategy optional: Loading strategy to use.
   */
  explicit Dictionary(const std::string& filename,
                      loading_strategy_types loading_strategy = loading_strategy_types::lazy)
      : fsa_(std::make_shared<fsa::Automata>(filename, loading_strategy)) {
    TRACE("Dictionary from file %s", filename.c_str());
  }

  explicit Dictionary(fsa::automata_t f) : fsa_(f) {}

  fsa::automata_t GetFsa() const { return fsa_; }

  std::string GetStatistics() const { return fsa_->GetStatistics(); }

  uint64_t GetSize() const { return fsa_->GetNumberOfKeys(); }

  /**
   * A simple Contains method to check whether a key is in the dictionary.
   *
   * @param key The key
   * @return True if key is in the dictionary, False otherwise.
   */
  bool Contains(const std::string& key) const {
    uint64_t state = fsa_->GetStartState();
    const size_t key_length = key.size();

    TRACE("Contains for %s", key.c_str());
    for (size_t i = 0; i < key_length; ++i) {
      state = fsa_->TryWalkTransition(state, key[i]);

      if (!state) {
        return false;
      }
      TRACE("Contains matched %d/%d", i + 1, key_length);
    }

    TRACE("Contains matched key, looking for Final State (%d)", state);
    if (state && fsa_->IsFinalState(state)) {
      TRACE("Contains matched final state.");
      return true;
    }

    return false;
  }

  Match operator[](const std::string& key) const {
    uint64_t state = fsa_->GetStartState();
    const size_t text_length = key.size();

    for (size_t i = 0; i < text_length; ++i) {
      state = fsa_->TryWalkTransition(state, key[i]);

      if (!state) {
        break;
      }
    }

    if (!fsa_->IsFinalState(state)) {
      return Match();
    }

    return Match(0, text_length, key, 0, fsa_, fsa_->GetStateValue(state));
  }

  /**
   * Exact Match function.
   *
   * @param key the key to lookup.
   * @return a match iterator
   */
  MatchIterator::MatchIteratorPair Get(const std::string& key) const {
    uint64_t state = fsa_->GetStartState();
    const size_t text_length = key.size();

    for (size_t i = 0; i < text_length; ++i) {
      state = fsa_->TryWalkTransition(state, key[i]);

      if (!state) {
        break;
      }
    }

    if (!fsa_->IsFinalState(state)) {
      return MatchIterator::EmptyIteratorPair();
    }

    Match m;
    bool has_run = false;

    // right now this is returning just 1 match, but it could be more if it is a multi-value dictionary
    m = Match(0, text_length, key, 0, fsa_, fsa_->GetStateValue(state));

    auto func = [m, has_run]() mutable {
      if (!has_run) {
        has_run = true;
        return m;
      }

      return Match();
    };

    return MatchIterator::MakeIteratorPair(func);
  }

  /**
   * All the items in the dictionary.
   *
   * @return a match iterator of all the items
   */
  MatchIterator::MatchIteratorPair GetAllItems() const {
    uint64_t state = fsa_->GetStartState();
    std::vector<unsigned char> traversal_stack;
    traversal_stack.reserve(1024);

    Match first_match;

    // data which is required for the callback as well
    struct delegate_payload {
      delegate_payload(fsa::StateTraverser<>&& t, const std::vector<unsigned char>& stack)
          : traverser(std::move(t)), traversal_stack(std::move(stack)) {}

      fsa::StateTraverser<> traverser;
      std::vector<unsigned char> traversal_stack;
    };

    std::shared_ptr<delegate_payload> data(new delegate_payload(fsa::StateTraverser<>(fsa_, state), traversal_stack));

    std::function<Match()> tfunc = [data]() {
      TRACE("GetAllKeys callback called");

      for (;;) {
        if (!data->traverser.AtEnd()) {
          data->traversal_stack.resize(data->traverser.GetDepth() - 1);
          data->traversal_stack.push_back(data->traverser.GetStateLabel());
          TRACE("Current depth %d (%d)", data->traverser.GetDepth() - 1, data->traversal_stack.size());

          if (data->traverser.IsFinalState()) {
            std::string match_str =
                std::string(reinterpret_cast<char*>(&data->traversal_stack[0]), data->traverser.GetDepth());
            TRACE("found final state at depth %d %s", data->traverser.GetDepth(), match_str.c_str());
            Match m(0, data->traverser.GetDepth(), match_str, 0, data->traverser.GetFsa(),
                    data->traverser.GetStateValue());

            data->traverser++;
            return m;
          }
          data->traverser++;
        } else {
          TRACE("StateTraverser exhausted.");
          return Match();
        }
      }
    };
    return MatchIterator::MakeIteratorPair(tfunc, first_match);
  }

  /**
   * A simple leftmostlongest lookup function.
   *
   * @param text the input
   * @return a match iterator.
   */
  MatchIterator::MatchIteratorPair Lookup(const std::string& text, size_t offset = 0) {
    uint64_t state = fsa_->GetStartState();
    const size_t text_length = text.size();
    uint64_t last_final_state = 0;
    size_t last_final_state_position = 0;

    for (size_t i = offset; i < text_length; ++i) {
      state = fsa_->TryWalkTransition(state, text[i]);

      if (!state) {
        break;
      }

      if (fsa_->IsFinalState(state)) {
        if (i + 1 == text_length || text[i + 1] == ' ') {
          last_final_state = state;
          last_final_state_position = i + 1;
        }
      }
    }

    Match m;
    bool has_run = false;

    if (last_final_state) {
      // right now this is returning just 1 match, but it could do more
      m = Match(offset, last_final_state_position, text.substr(offset, last_final_state_position - offset), 0, fsa_,
                fsa_->GetStateValue(last_final_state));
    }

    auto func = [m, has_run]() mutable {
      if (!has_run) {
        has_run = true;
        return m;
      }

      return Match();
    };

    return MatchIterator::MakeIteratorPair(func);
  }

  /**
   * A simple leftmostlongest lookup function.
   *
   * @param text the input
   * @return a match iterator.
   */
  MatchIterator::MatchIteratorPair LookupText(const std::string& text) {
    const size_t text_length = text.size();
    std::queue<MatchIterator> iterators;

    TRACE("LookupText, 1st lookup for: %s", text.c_str());

    iterators.push(Lookup(text).begin());
    size_t position = 1;

    while (position < text_length) {
      if (text[position] != ' ') {
        ++position;
        continue;
      }

      ++position;
      TRACE("LookupText, starting lookup for: %s", text.c_str() + position);
      iterators.push(Lookup(text, position).begin());
    }

    MatchIterator current_it = iterators.front();
    iterators.pop();

    auto func = [iterators, current_it]() mutable {
      while (iterators.size() && (*current_it).IsEmpty()) {
        current_it = iterators.front();
        iterators.pop();
      }

      return *current_it++;
    };

    return MatchIterator::MakeIteratorPair(func);
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
  MatchIterator::MatchIteratorPair GetNear(const std::string& key, const size_t minimum_prefix_length,
                                           const bool greedy = false) const {
    auto data = std::make_shared<matching::NearMatching<>>(
        matching::NearMatching<>::FromSingleFsa(fsa_, key, minimum_prefix_length, greedy));

    auto func = [data]() { return data->NextMatch(); };
    return MatchIterator::MakeIteratorPair(func, data->FirstMatch());
  }

  MatchIterator::MatchIteratorPair GetFuzzy(const std::string& query, const int32_t max_edit_distance,
                                            const size_t minimum_exact_prefix = 2) const {
    auto data = std::make_shared<matching::FuzzyMatching<>>(
        matching::FuzzyMatching<>::FromSingleFsa(fsa_, query, max_edit_distance, minimum_exact_prefix));

    auto func = [data]() { return data->NextMatch(); };
    return MatchIterator::MakeIteratorPair(func, data->FirstMatch());
  }

  std::string GetManifest() const { return fsa_->GetManifest(); }

 private:
  fsa::automata_t fsa_;
};

// shared pointer
typedef std::shared_ptr<Dictionary> dictionary_t;

} /* namespace dictionary */
} /* namespace keyvi */

#endif  //  KEYVI_DICTIONARY_DICTIONARY_H_
