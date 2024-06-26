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
 * multiword_completion.h
 *
 *  Created on: Jul 8, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_COMPLETION_MULTIWORD_COMPLETION_H_
#define KEYVI_DICTIONARY_COMPLETION_MULTIWORD_COMPLETION_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/bounded_weighted_state_traverser.h"
#include "keyvi/dictionary/fsa/codepoint_state_traverser.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/dictionary/match_iterator.h"
#include "keyvi/dictionary/util/transform.h"
#include "keyvi/stringdistance/levenshtein.h"
#include "utf8.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace completion {

class MultiWordCompletion final {
 public:
  explicit MultiWordCompletion(dictionary_t d) : fsa_(d->GetFsa()) {}

  MatchIterator::MatchIteratorPair GetCompletions(const std::string& query, size_t number_of_results = 10) const {
    uint64_t state = fsa_->GetStartState();
    size_t number_of_tokens;
    std::string query_bow = util::Transform::BagOfWordsPartial(query, number_of_tokens);

    TRACE("Query after bow: %s", query_bow.c_str());

    size_t query_length = query_bow.size();
    size_t depth = 0;

    std::vector<unsigned char> traversal_stack;

    while (state != 0 && depth != query_length) {
      state = fsa_->TryWalkTransition(state, query_bow[depth]);
      traversal_stack.push_back(query_bow[depth]);
      ++depth;
    }

    TRACE("state %d", state);

    // reserve some capacity
    traversal_stack.reserve(100);

    if (depth == query_length) {
      match_t first_match;
      TRACE("matched prefix");

      // data which is required for the callback as well
      struct delegate_payload {
        delegate_payload(fsa::BoundedWeightedStateTraverser&& t, std::vector<unsigned char>&& stack)
            : traverser(std::move(t)), traversal_stack(stack) {}

        fsa::BoundedWeightedStateTraverser traverser;
        std::vector<unsigned char> traversal_stack;
        size_t multi_word_boundary = 0;
      };

      std::shared_ptr<delegate_payload> data(new delegate_payload(
          fsa::BoundedWeightedStateTraverser(fsa_, state, number_of_results), std::move(traversal_stack)));

      if (fsa_->IsFinalState(state)) {
        TRACE("prefix matched depth %d %s", query_length + data->traverser.GetDepth(),
              std::string(reinterpret_cast<char*>(&data->traversal_stack[0]), query_length + data->traverser.GetDepth())
                  .c_str());
        first_match = std::make_shared<Match>(0, query_length, query, 0, fsa_, fsa_->GetStateValue(state));
      }

      auto tfunc = [data, query_length]() {
        TRACE("prefix completion callback called");

        for (;;) {
          if (data->traverser) {
            unsigned char label = data->traverser.GetStateLabel();
            if (label == 0x1b) {
              data->multi_word_boundary = data->traverser.GetDepth();
              TRACE("found MW boundary at %d", data->multi_word_boundary);
              // reset the multiword boundary if we went up
            } else if (data->traverser.GetDepth() <= data->multi_word_boundary) {
              data->multi_word_boundary = 0;
              TRACE("reset MW boundary at %d", data->traverser.GetDepth());
            }

            // only match up to the number of tokens in input
            // todo: refactor workaround for longer tokens
            if (label == 0x20 && data->multi_word_boundary == 0) {
              TRACE("found space before MW boundary at %d", data->multi_word_boundary);
              data->traverser.Prune();
              data->traverser++;
              continue;
            }
            data->traversal_stack.resize(query_length + data->traverser.GetDepth() - 1);
            data->traversal_stack.push_back(label);
            TRACE("Current depth %d (%d)", query_length + data->traverser.GetDepth() - 1, data->traversal_stack.size());
            if (data->traverser.IsFinalState()) {
              TRACE("found final state at depth %d %s", query_length + data->traverser.GetDepth(),
                    std::string(reinterpret_cast<char*>(&data->traversal_stack[0]),
                                query_length + data->traverser.GetDepth())
                        .c_str());
              std::string matched_entry;

              if (data->multi_word_boundary) {
                matched_entry = std::string(
                    reinterpret_cast<char*>(&data->traversal_stack[query_length + data->multi_word_boundary]),
                    data->traverser.GetDepth() - data->multi_word_boundary);
              } else {
                matched_entry = std::string(reinterpret_cast<char*>(&data->traversal_stack[0]),
                                            query_length + data->traverser.GetDepth());
              }

              match_t m = std::make_shared<Match>(0, data->traverser.GetDepth() + query_length, matched_entry, 0,
                                                  data->traverser.GetFsa(), data->traverser.GetStateValue());

              data->traverser++;
              data->traverser.TryReduceResultQueue();
              return m;
            }
            data->traverser++;
          } else {
            TRACE("StateTraverser exhausted.");
            return match_t();
          }
        }
      };

      return MatchIterator::MakeIteratorPair(tfunc, std::move(first_match));
    }

    return MatchIterator::EmptyIteratorPair();
  }

 private:
  fsa::automata_t fsa_;
};

} /* namespace completion */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_COMPLETION_MULTIWORD_COMPLETION_H_
