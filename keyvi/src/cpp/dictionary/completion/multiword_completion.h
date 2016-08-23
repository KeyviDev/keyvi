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

#ifndef MULTIWORD_COMPLETION_H_
#define MULTIWORD_COMPLETION_H_

#include "stringdistance/levenshtein.h"
#include "utf8.h"
#include "dictionary/dictionary.h"
#include "dictionary/fsa/automata.h"
#include "dictionary/match_iterator.h"
#include "dictionary/fsa/traverser_types.h"
#include "dictionary/fsa/bounded_weighted_state_traverser.h"
#include "dictionary/fsa/codepoint_state_traverser.h"
#include "dictionary/util/transform.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace completion {

class MultiWordCompletion final {
public:
  MultiWordCompletion(dictionary_t d) {
      fsa_ = d->GetFsa();
    }

    MatchIterator::MatchIteratorPair GetCompletions(
        const std::string& query, int number_of_results = 10) const {

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
        Match first_match;
        TRACE("matched prefix");

        // data which is required for the callback as well
        struct delegate_payload {
          delegate_payload(fsa::BoundedWeightedStateTraverser&& t,
                           std::vector<unsigned char>& stack)
              : traverser(std::move(t)),
                traversal_stack(std::move(stack)) {
          }

          fsa::BoundedWeightedStateTraverser traverser;
          std::vector<unsigned char> traversal_stack;
          size_t multi_word_boundary = 0;
        };

        std::shared_ptr<delegate_payload> data(
            new delegate_payload(
                fsa::BoundedWeightedStateTraverser(fsa_, state,
                                                   number_of_results),
                traversal_stack));

        if (fsa_->IsFinalState(state)) {
          TRACE("prefix matched depth %d %s", query_length + data->traverser.GetDepth(), std::string(reinterpret_cast<char*> (&data->traversal_stack[0]), query_length + data->traverser.GetDepth()).c_str());
          first_match = Match(
              0, query_length, query, 0, fsa_, fsa_->GetStateValue(state));
        }

        auto tfunc =
            [data, query_length] () {
              TRACE("prefix completion callback called");

              for (;;) {
                unsigned char label = data->traverser.GetStateLabel();

                if (label) {

                  if (label == 0x1b){
                    data->multi_word_boundary = data->traverser.GetDepth();
                    TRACE("found MW boundary at %d", data->multi_word_boundary);
                    // reset the multiword boundary if we went up
                  } else if (data->traverser.GetDepth() <= data->multi_word_boundary) {
                    data->multi_word_boundary = 0;
                    TRACE("reset MW boundary at %d", data->traverser.GetDepth());
                  }

                  // only match up to the number of tokens in input
                  // todo: refactor workaround for longer tokens
                  if (label == 0x20 && data->multi_word_boundary == 0){
                    TRACE("found space before MW boundary at %d", data->multi_word_boundary);
                    data->traverser.Prune();
                    data->traverser++;
                    continue;
                  }
                  data->traversal_stack.resize(query_length + data->traverser.GetDepth()-1);
                  data->traversal_stack.push_back(label);
                  TRACE("Current depth %d (%d)", query_length + data->traverser.GetDepth() -1, data->traversal_stack.size());
                  if (data->traverser.IsFinalState()) {
                    TRACE("found final state at depth %d %s", query_length + data->traverser.GetDepth(), std::string(reinterpret_cast<char*> (&data->traversal_stack[0]), query_length + data->traverser.GetDepth()).c_str());
                    std::string matched_entry;

                    if (data->multi_word_boundary) {
                      matched_entry = std::string(reinterpret_cast<char*> (&data->traversal_stack[query_length + data->multi_word_boundary]), data->traverser.GetDepth() - data->multi_word_boundary);
                    } else {
                      matched_entry = std::string(reinterpret_cast<char*> (&data->traversal_stack[0]), query_length + data->traverser.GetDepth());
                    }

                    Match m(0,
                        data->traverser.GetDepth() + query_length,
                        matched_entry,
                        0,
                        data->traverser.GetFsa(),
                        data->traverser.GetStateValue());

                    data->traverser++;
                    data->traverser.TryReduceResultQueue();
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

      return MatchIterator::EmptyIteratorPair();
    }

private:
 fsa::automata_t fsa_;

};

} /* namespace completion */
} /* namespace dictionary */
} /* namespace keyvi */



#endif /* MULTIWORD_COMPLETION_H_ */
