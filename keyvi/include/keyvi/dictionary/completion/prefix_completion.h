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

#ifndef KEYVI_DICTIONARY_COMPLETION_PREFIX_COMPLETION_H_
#define KEYVI_DICTIONARY_COMPLETION_PREFIX_COMPLETION_H_

#include <algorithm>
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
#include "keyvi/stringdistance/levenshtein.h"
#include "utf8.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace completion {

/**
 * A prefix completion implementation. Given a query returns the n most relevant completions.
 */
class PrefixCompletion final {
 public:
  explicit PrefixCompletion(dictionary_t d) : fsa_(d->GetFsa()) {}

  MatchIterator::MatchIteratorPair GetCompletions(const std::string& query, size_t number_of_results = 10) {
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
        delegate_payload(fsa::BoundedWeightedStateTraverser&& t, std::vector<unsigned char>&& stack)
            : traverser(std::move(t)), traversal_stack(stack) {}

        fsa::BoundedWeightedStateTraverser traverser;
        std::vector<unsigned char> traversal_stack;
      };

      std::shared_ptr<delegate_payload> data(new delegate_payload(
          fsa::BoundedWeightedStateTraverser(fsa_, state, number_of_results), std::move(traversal_stack)));

      if (fsa_->IsFinalState(state)) {
        TRACE("prefix matched depth %d %s", query_length + data->traverser.GetDepth(),
              std::string(reinterpret_cast<char*>(&data->traversal_stack[0]), query_length + data->traverser.GetDepth())
                  .c_str());
        first_match = Match(0, query_length, query, 0, fsa_, fsa_->GetStateValue(state));
      }

      auto tfunc = [data, query_length]() {
        TRACE("prefix completion callback called");

        for (;;) {
          if (data->traverser) {
            data->traversal_stack.resize(query_length + data->traverser.GetDepth() - 1);
            data->traversal_stack.push_back(data->traverser.GetStateLabel());
            TRACE("Current depth %d (%d)", query_length + data->traverser.GetDepth() - 1, data->traversal_stack.size());
            if (data->traverser.IsFinalState()) {
              std::string match_str = std::string(reinterpret_cast<char*>(&data->traversal_stack[0]),
                                                  query_length + data->traverser.GetDepth());
              TRACE("found final state at depth %d %s", query_length + data->traverser.GetDepth(), match_str.c_str());
              Match m(0, data->traverser.GetDepth() + query_length, match_str, 0, data->traverser.GetFsa(),
                      data->traverser.GetStateValue());

              data->traverser++;
              // data->traverser.TryReduceResultQueue();
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

  MatchIterator::MatchIteratorPair GetFuzzyCompletions(const std::string& query, const int32_t max_edit_distance,
                                                       const size_t minimum_exact_prefix = 2) {
    uint64_t state = fsa_->GetStartState();
    size_t depth = 0;
    std::vector<uint32_t> codepoints;

    utf8::unchecked::utf8to32(query.begin(), query.end(), back_inserter(codepoints));
    const size_t query_length = codepoints.size();
    size_t exact_prefix = std::min(query_length, minimum_exact_prefix);

    TRACE("Query: [%s] length: %d", query.c_str(), query_length);

    stringdistance::LevenshteinCompletion metric(codepoints, 20, max_edit_distance);

    size_t utf8_depth = 0;
    // match exact
    while (state != 0 && depth != exact_prefix) {
      const size_t code_point_length = util::Utf8Utils::GetCharLength(query[utf8_depth]);
      for (size_t i = 0; i < code_point_length; ++i, ++utf8_depth) {
        state = fsa_->TryWalkTransition(state, query[utf8_depth]);
        if (0 == state) {
          break;
        }
      }
      metric.Put(codepoints[depth], depth);
      ++depth;
    }

    if (!state) {
      return MatchIterator::EmptyIteratorPair();
    }

    struct data_delegate_fuzzy {
      data_delegate_fuzzy(fsa::CodePointStateTraverser<fsa::WeightedStateTraverser>&& t,
                          stringdistance::LevenshteinCompletion&& m)
          : traverser(std::move(t)), metric(std::move(m)) {}

      fsa::CodePointStateTraverser<fsa::WeightedStateTraverser> traverser;
      stringdistance::LevenshteinCompletion metric;
    };

    std::shared_ptr<data_delegate_fuzzy> data(new data_delegate_fuzzy(
        fsa::CodePointStateTraverser<fsa::WeightedStateTraverser>(fsa_, state), std::move(metric)));

    TRACE("state %d", state);

    Match first_match;
    TRACE("matched prefix");

    if (depth == query_length && fsa_->IsFinalState(state)) {
      TRACE("prefix matched depth %d %s", query_length + data->traverser.GetDepth(),
            std::string(query, query_length).c_str());
      first_match = Match(0, query_length, query, 0, fsa_, fsa_->GetStateValue(state));
    }

    auto tfunc = [data, query_length, max_edit_distance, exact_prefix]() {
      TRACE("prefix completion callback called");
      for (;;) {
        if (data->traverser) {
          size_t depth = exact_prefix + data->traverser.GetDepth() - 1;
          TRACE("Current depth %d", depth);

          int32_t intermediate_score = data->metric.Put(data->traverser.GetStateLabel(), depth);
          depth++;

          TRACE("Candidate: [%s] %ld/%ld intermediate score: %d", data->metric.GetCandidate().c_str(), query_length,
                depth, intermediate_score);

          if (intermediate_score > max_edit_distance) {
            data->traverser.Prune();
            data->traverser++;
            continue;
          }

          if (data->traverser.IsFinalState()) {
            if (query_length < depth || data->metric.GetScore() <= max_edit_distance) {
              TRACE("found final state at depth %d %s", depth, data->metric.GetCandidate().c_str());
              Match m(0, depth, data->metric.GetCandidate(), data->metric.GetScore(), data->traverser.GetFsa(),
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

 private:
  fsa::automata_t fsa_;
};

} /* namespace completion */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_COMPLETION_PREFIX_COMPLETION_H_
