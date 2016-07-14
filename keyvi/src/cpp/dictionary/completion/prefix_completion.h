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
 * prefix_completion.h
 *
 *  Created on: Jun 3, 2014
 *      Author: hendrik
 */

#ifndef PREFIX_COMPLETION_H_
#define PREFIX_COMPLETION_H_

#include "stringdistance/levenshtein.h"
#include "utf8.h"
#include "dictionary/dictionary.h"
#include "dictionary/fsa/automata.h"
#include "dictionary/match_iterator.h"
#include "dictionary/fsa/traverser_types.h"
#include "dictionary/fsa/bounded_weighted_state_traverser.h"
#include "dictionary/fsa/codepoint_state_traverser.h"
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace completion {

/**
 * A prefix completion implementation. Given a query returns the n most relevant completions.
 */
class PrefixCompletion
final {
   public:
    PrefixCompletion(dictionary_t d) {
      fsa_ = d->GetFsa();
    }

    MatchIterator::MatchIteratorPair GetCompletions(
        const std::string& query, int number_of_results = 10) {

      uint64_t state = fsa_->GetStartState();
      const size_t query_length = query.size();
      size_t depth = 0;

      std::vector<unsigned char> traversal_stack;

      while (state != 0 && depth != query_length) {
        state = fsa_->TryWalkTransition(state, query[depth]);
        traversal_stack.push_back(query[depth]);
        ++depth;
      }

      TRACE("state %d", state);

      traversal_stack.reserve(1024);

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

                  data->traversal_stack.resize(query_length+data->traverser.GetDepth()-1);
                  data->traversal_stack.push_back(label);
                  TRACE("Current depth %d (%d)", query_length + data->traverser.GetDepth() -1, data->traversal_stack.size());
                  if (data->traverser.IsFinalState()) {
                    std::string match_str = std::string(reinterpret_cast<char*> (&data->traversal_stack[0]), query_length + data->traverser.GetDepth())
                    TRACE("found final state at depth %d %s", query_length + data->traverser.GetDepth(), match_str.c_str());
                    Match m(0,
                        data->traverser.GetDepth() + query_length,
                        match_str,
                        0,
                        data->traverser.GetFsa(),
                        data->traverser.GetStateValue());

                    data->traverser++;
                    //data->traverser.TryReduceResultQueue();
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

    MatchIterator::MatchIteratorPair GetFuzzyCompletions(
        const std::string& query, int max_edit_distance) {

      uint64_t state = fsa_->GetStartState();
      const size_t query_length = query.size();
      size_t depth = 0;
      const size_t minimum_exact_prefix = 2;
      size_t exact_prefix = std::min(query_length, minimum_exact_prefix);
      std::vector<int> codepoints;

      utf8::unchecked::utf8to32(query.c_str(), query.c_str() + query_length,
                                back_inserter(codepoints));

      stringdistance::Levenshtein metric(codepoints, 20, 3);

      // match exact
      while (state != 0 && depth != exact_prefix) {
        state = fsa_->TryWalkTransition(state, query[depth]);
        metric.Put(codepoints[depth], depth);
        ++depth;
      }

      struct data_delegate_fuzzy {
        data_delegate_fuzzy(
            fsa::CodePointStateTraverser<fsa::WeightedStateTraverser>&& t,
            stringdistance::Levenshtein& m)
            : traverser(std::move(t)),
              metric(std::move(m)) {
        }

        fsa::CodePointStateTraverser<fsa::WeightedStateTraverser> traverser;
        stringdistance::Levenshtein metric;
      };

      std::shared_ptr<data_delegate_fuzzy> data(
          new data_delegate_fuzzy(
              fsa::CodePointStateTraverser<fsa::WeightedStateTraverser>(fsa_,
                                                                        state),
              metric));

      TRACE("state %d", state);

      if (state) {
        Match first_match;
        TRACE("matched prefix");

        if (depth == query_length && fsa_->IsFinalState(state)) {
          TRACE("prefix matched depth %d %s", query_length + data->traverser.GetDepth(), std::string(query, query_length).c_str());
          first_match = Match(
              0, query_length, query, 0, fsa_, fsa_->GetStateValue(state));
        }

        auto tfunc =
            [data, query_length, max_edit_distance, exact_prefix] () {
              TRACE("prefix completion callback called");
              for (;;) {
                int label = data->traverser.GetStateLabel();

                if (label) {

                  TRACE("Current depth %d", exact_prefix + data->traverser.GetDepth() -1);

                  int score = data->metric.Put(label, exact_prefix + data->traverser.GetDepth() - 1);

                  TRACE("Intermediate score %d", score);

                  if (query_length > data->traverser.GetDepth() && score > max_edit_distance) {
                    data->traverser.Prune();
                    data->traverser++;
                    continue;
                  }

                  if (data->traverser.IsFinalState()) {
                    if (query_length < data->traverser.GetDepth() || data->metric.GetScore() <= max_edit_distance) {

                      TRACE("found final state at depth %d %s", exact_prefix + data->traverser.GetDepth(), data->metric.GetCandidate().c_str());
                      Match m(0,
                          data->traverser.GetDepth() + query_length,
                          data->metric.GetCandidate(),
                          data->metric.GetScore(),
                          data->traverser.GetFsa(),
                          data->traverser.GetStateValue());

                      data->traverser++;

                      return m;
                    }
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

#endif /* PREFIX_COMPLETION_H_ */
