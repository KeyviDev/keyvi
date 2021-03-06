/* * keyvi - A key value store.
 *
 * Copyright 2018   Narek Gharibyan<narekgharibyan@gmail.com>
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
 * fuzzy_match.h
 *
 *  Created on: February 11, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#ifndef KEYVI_DICTIONARY_MATCHING_FUZZY_MATCHING_H_
#define KEYVI_DICTIONARY_MATCHING_FUZZY_MATCHING_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "utf8.h"

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/codepoint_state_traverser.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/dictionary/fsa/zip_state_traverser.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/util/utf8_utils.h"
#include "keyvi/stringdistance/levenshtein.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace matching {

template <class codepointInnterTraverserType = fsa::WeightedStateTraverser>
class FuzzyMatching final {
 public:
  FuzzyMatching(std::unique_ptr<fsa::CodePointStateTraverser<codepointInnterTraverserType>>&& traverser,
                std::unique_ptr<stringdistance::Levenshtein>&& metric, Match&& first_match,
                const size_t minimum_exact_prefix, int32_t max_edit_distance)
      : metric_ptr_(std::move(metric)),
        traverser_ptr_(std::move(traverser)),
        minimum_exact_prefix_(minimum_exact_prefix),
        max_edit_distance_(max_edit_distance),
        first_match_(std::move(first_match)) {}

  template <class innerTraverserType = fsa::WeightedStateTraverser>
  static FuzzyMatching FromSingleFsa(const fsa::automata_t& fsa, const std::string& query,
                                     const int32_t max_edit_distance, const size_t minimum_exact_prefix = 2) {
    std::unique_ptr<stringdistance::Levenshtein> metric;
    std::unique_ptr<fsa::CodePointStateTraverser<innerTraverserType>> traverser;
    Match first_match;

    std::vector<uint32_t> codepoints;
    utf8::unchecked::utf8to32(query.begin(), query.end(), back_inserter(codepoints));

    size_t query_char_length = codepoints.size();
    if (query_char_length < minimum_exact_prefix) {
      TRACE("query lengh < minimum exact prefix, returning empty iterator");
      return FuzzyMatching<innerTraverserType>(std::move(traverser), std::move(metric), std::move(first_match),
                                               minimum_exact_prefix, max_edit_distance);
    }
    metric.reset(new stringdistance::Levenshtein(codepoints, 20, max_edit_distance));

    // match exact prefix
    uint64_t state = fsa->GetStartState();
    size_t depth = 0;
    size_t utf8_depth = 0;
    while (state != 0 && depth < minimum_exact_prefix) {
      const size_t code_point_length = util::Utf8Utils::GetCharLength(query[utf8_depth]);
      for (size_t i = 0; i < code_point_length; ++i, ++utf8_depth) {
        state = fsa->TryWalkTransition(state, query[utf8_depth]);
        if (0 == state) {
          break;
        }
      }
      metric->Put(codepoints[depth], depth);
      ++depth;
    }

    if (state) {
      if (depth == query_char_length && fsa->IsFinalState(state)) {
        first_match = Match(0, query_char_length, query, 0, fsa, fsa->GetStateValue(state));
      }
      traverser.reset(new fsa::CodePointStateTraverser<innerTraverserType>(fsa, state));
    }

    TRACE("create iterator");
    return FuzzyMatching<innerTraverserType>(std::move(traverser), std::move(metric), std::move(first_match),
                                             minimum_exact_prefix, max_edit_distance);
  }

  template <class innerTraverserType = fsa::WeightedStateTraverser>
  static FuzzyMatching<fsa::ZipStateTraverser<innerTraverserType>> FromMulipleFsas(
      const std::vector<fsa::automata_t>& fsas, const std::string& query, const int32_t max_edit_distance,
      const size_t minimum_exact_prefix = 2) {
    std::unique_ptr<stringdistance::Levenshtein> metric;
    std::unique_ptr<fsa::CodePointStateTraverser<fsa::ZipStateTraverser<innerTraverserType>>> traverser;
    Match first_match;

    std::vector<uint32_t> codepoints;
    utf8::unchecked::utf8to32(query.begin(), query.end(), back_inserter(codepoints));

    size_t query_char_length = codepoints.size();
    if (query_char_length < minimum_exact_prefix) {
      TRACE("query lengh < minimum exact prefix, returning empty iterator");
      return FuzzyMatching<fsa::ZipStateTraverser<innerTraverserType>>(
          std::move(traverser), std::move(metric), std::move(first_match), minimum_exact_prefix, max_edit_distance);
    }

    metric.reset(new stringdistance::Levenshtein(codepoints, 20, max_edit_distance));

    std::vector<std::pair<fsa::automata_t, uint64_t>> fsa_start_state_pairs;

    for (const fsa::automata_t& fsa : fsas) {
      uint64_t state = fsa->GetStartState();
      size_t depth = 0;
      size_t utf8_depth = 0;
      while (state != 0 && depth < minimum_exact_prefix) {
        const size_t code_point_length = util::Utf8Utils::GetCharLength(query[utf8_depth]);
        for (size_t i = 0; i < code_point_length; ++i, ++utf8_depth) {
          state = fsa->TryWalkTransition(state, query[utf8_depth]);
          if (0 == state) {
            break;
          }
        }
        TRACE("metric->put %lu  depth: %lu", codepoints[depth], depth);
        metric->Put(codepoints[depth], depth);
        ++depth;
      }

      if (state) {
        if (depth == query_char_length && fsa->IsFinalState(state) && first_match.IsEmpty() == false) {
          first_match = Match(0, query_char_length, query, 0, fsa, fsa->GetStateValue(state));
        }

        fsa_start_state_pairs.emplace_back(fsa, state);
      }
    }

    if (fsa_start_state_pairs.size() > 0) {
      fsa::ZipStateTraverser<innerTraverserType> zip_state_traverser(fsa_start_state_pairs, false);
      traverser.reset(
          new fsa::CodePointStateTraverser<fsa::ZipStateTraverser<innerTraverserType>>(std::move(zip_state_traverser)));
    }

    TRACE("create iterator");
    return FuzzyMatching<fsa::ZipStateTraverser<innerTraverserType>>(
        std::move(traverser), std::move(metric), std::move(first_match), minimum_exact_prefix, max_edit_distance);
  }

  Match FirstMatch() const { return first_match_; }

  Match NextMatch() {
    for (; traverser_ptr_ && *traverser_ptr_; (*traverser_ptr_)++) {
      TRACE("metric->put %lu  depth: %lu", traverser_ptr_->GetStateLabel(), candidate_length() - 1);
      const int32_t intermediate_score = metric_ptr_->Put(traverser_ptr_->GetStateLabel(), candidate_length() - 1);
      // don't consider subtrees which can not be matched anyways
      if (metric_ptr_->GetInputSequence().size() > candidate_length() && intermediate_score > max_edit_distance_) {
        traverser_ptr_->Prune();
        continue;
      }

      if (metric_ptr_->GetInputSequence().size() + max_edit_distance_ < candidate_length()) {
        traverser_ptr_->Prune();
        continue;
      }

      if (traverser_ptr_->IsFinalState() && metric_ptr_->GetScore() <= max_edit_distance_) {
        Match m = Match(0, candidate_length(), metric_ptr_->GetCandidate(), metric_ptr_->GetScore(),
                        traverser_ptr_->GetFsa(), traverser_ptr_->GetStateValue());
        (*traverser_ptr_)++;
        return m;
      }
    }
    return Match();
  }

 private:
  size_t candidate_length() const {
    assert(traverser_ptr_);
    return minimum_exact_prefix_ + traverser_ptr_->GetDepth();
  }

 private:
  std::unique_ptr<stringdistance::Levenshtein> metric_ptr_;
  std::unique_ptr<fsa::CodePointStateTraverser<codepointInnterTraverserType>> traverser_ptr_;
  const size_t minimum_exact_prefix_;
  const int32_t max_edit_distance_;
  Match first_match_;
};

} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
#endif  // KEYVI_DICTIONARY_MATCHING_FUZZY_MATCHING_H_
