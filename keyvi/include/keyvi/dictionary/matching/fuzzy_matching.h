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
namespace index {
namespace internal {
template <class MatcherT, class DeletedT>
keyvi::dictionary::Match NextFilteredMatchSingle(const MatcherT&, const DeletedT&);
template <class MatcherT, class DeletedT>
keyvi::dictionary::Match NextFilteredMatch(const MatcherT&, const DeletedT&);
}  // namespace internal
}  // namespace index
namespace dictionary {
namespace matching {

template <class codepointInnerTraverserType = fsa::WeightedStateTraverser>
class FuzzyMatching final {
 public:
  /**
   * Create a fuzzy matcher from a single Fsa
   *
   * @param fsa the fsa
   * @param query the query
   * @param max_edit_distance the maximum allowed edit distance
   * @param minimum_exact_prefix the minimum exact prefix to match before matching approximate
   */
  template <class innerTraverserType = fsa::WeightedStateTraverser>
  static FuzzyMatching FromSingleFsa(const fsa::automata_t& fsa, const std::string& query,
                                     const int32_t max_edit_distance, const size_t minimum_exact_prefix = 2) {
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
      ++depth;
    }

    if (depth != minimum_exact_prefix) {
      return FuzzyMatching<innerTraverserType>();
    }

    return FromSingleFsa<innerTraverserType>(fsa, state, query, max_edit_distance, minimum_exact_prefix);
  }

  /**
   * Create a fuzzy matcher from a single Fsa
   *
   * @param fsa the fsa
   * @param start_state the state to start from
   * @param query the query
   * @param max_edit_distance the maximum allowed edit distance
   * @param exact_prefix the exact prefix that already matched
   */
  template <class innerTraverserType = fsa::WeightedStateTraverser>
  static FuzzyMatching FromSingleFsa(const fsa::automata_t& fsa, const uint64_t start_state, const std::string& query,
                                     const int32_t max_edit_distance, const size_t exact_prefix) {
    if (start_state == 0) {
      return FuzzyMatching<innerTraverserType>();
    }

    std::unique_ptr<stringdistance::Levenshtein> metric;
    std::unique_ptr<fsa::CodePointStateTraverser<innerTraverserType>> traverser;
    Match first_match;

    std::vector<uint32_t> codepoints;
    utf8::unchecked::utf8to32(query.begin(), query.end(), back_inserter(codepoints));

    if (start_state == 0) {
      TRACE("query lengh < minimum exact prefix, returning empty iterator");
      return FuzzyMatching<innerTraverserType>();
    }

    // initialize the distance metric with the exact prefix
    metric.reset(new stringdistance::Levenshtein(codepoints, 20, max_edit_distance));
    for (size_t i = 0; i < exact_prefix; ++i) {
      metric->Put(codepoints[i], i);
    }

    traverser.reset(new fsa::CodePointStateTraverser<innerTraverserType>(fsa, start_state));

    if (fsa->IsFinalState(start_state) && metric->GetScore() <= max_edit_distance) {
      TRACE("exact prefix matched");
      first_match =
          Match(0, exact_prefix, metric->GetCandidate(), metric->GetScore(), fsa, fsa->GetStateValue(start_state));
    }

    TRACE("create iterator");
    return FuzzyMatching<innerTraverserType>(std::move(traverser), std::move(metric), std::move(first_match),
                                             max_edit_distance, exact_prefix);
  }

  /**
   * Create a fuzzy matcher from multiple Fsas
   *
   * @param fsas a vector of fsas
   * @param query the query
   * @param max_edit_distance the maximum allowed edit distance
   * @param minimum_exact_prefix the minimum exact prefix to match before matching approximate
   */
  template <class innerTraverserType = fsa::WeightedStateTraverser>
  static FuzzyMatching<fsa::ZipStateTraverser<innerTraverserType>> FromMulipleFsas(
      const std::vector<fsa::automata_t>& fsas, const std::string& query, const int32_t max_edit_distance,
      const size_t minimum_exact_prefix = 2) {
    std::vector<std::pair<fsa::automata_t, uint64_t>> fsa_start_state_pairs =
        FilterWithExactPrefix(fsas, query, minimum_exact_prefix);

    return FromMulipleFsas<innerTraverserType>(fsa_start_state_pairs, query, max_edit_distance, minimum_exact_prefix);
  }

  /**
   * Create a fuzzy matcher with already matched exact prefix.
   *
   * @param fsa_start_state_pairs pairs of fsa and current state
   * @param query the query
   * @param max_edit_distance the maximum allowed edit distance
   * @param exact_prefix the exact prefix that already matched
   */
  template <class innerTraverserType = fsa::WeightedStateTraverser>
  static FuzzyMatching<fsa::ZipStateTraverser<innerTraverserType>> FromMulipleFsas(
      const std::vector<std::pair<fsa::automata_t, uint64_t>>& fsa_start_state_pairs, const std::string& query,
      const int32_t max_edit_distance, const size_t exact_prefix) {
    // if the list of fsa's is empty return an empty matcher
    if (fsa_start_state_pairs.size() == 0) {
      return FuzzyMatching<fsa::ZipStateTraverser<innerTraverserType>>();
    }

    std::unique_ptr<stringdistance::Levenshtein> metric;
    std::unique_ptr<fsa::CodePointStateTraverser<fsa::ZipStateTraverser<innerTraverserType>>> traverser;
    Match first_match;

    // decode the utf8 query into single codepoints
    std::vector<uint32_t> codepoints;
    utf8::unchecked::utf8to32(query.begin(), query.end(), back_inserter(codepoints));

    // initialize the distance metric with the exact prefix
    metric.reset(new stringdistance::Levenshtein(codepoints, 20, max_edit_distance));
    for (size_t i = 0; i < exact_prefix; ++i) {
      metric->Put(codepoints[i], i);
    }

    // check for a match given the exact prefix
    for (const auto& fsa_state : fsa_start_state_pairs) {
      if (fsa_state.first->IsFinalState(fsa_state.second) && metric->GetScore() <= max_edit_distance) {
        first_match = Match(0, exact_prefix, metric->GetCandidate(), metric->GetScore(), fsa_state.first,
                            fsa_state.first->GetStateValue(fsa_state.second));
        break;
      }
    }

    TRACE("create zip traverser with %ul inner traversers", fsa_start_state_pairs.size());
    fsa::ZipStateTraverser<innerTraverserType> zip_state_traverser(fsa_start_state_pairs, false);
    traverser.reset(
        new fsa::CodePointStateTraverser<fsa::ZipStateTraverser<innerTraverserType>>(std::move(zip_state_traverser)));

    TRACE("create iterator");
    return FuzzyMatching<fsa::ZipStateTraverser<innerTraverserType>>(
        std::move(traverser), std::move(metric), std::move(first_match), max_edit_distance, exact_prefix);
  }

  static inline std::vector<std::pair<fsa::automata_t, uint64_t>> FilterWithExactPrefix(
      const std::vector<fsa::automata_t>& fsas, const std::string& query, const size_t minimum_exact_prefix = 2) {
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
        ++depth;
      }

      if (state && depth == minimum_exact_prefix) {
        fsa_start_state_pairs.emplace_back(fsa, state);
      }
    }
    return fsa_start_state_pairs;
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
        TRACE("found match %s %lu", metric_ptr_->GetCandidate().c_str(), traverser_ptr_->GetStateValue());
        Match m = Match(0, candidate_length(), metric_ptr_->GetCandidate(), metric_ptr_->GetScore(),
                        traverser_ptr_->GetFsa(), traverser_ptr_->GetStateValue());
        (*traverser_ptr_)++;
        return m;
      }
    }
    return Match();
  }

 private:
  FuzzyMatching(std::unique_ptr<fsa::CodePointStateTraverser<codepointInnerTraverserType>>&& traverser,
                std::unique_ptr<stringdistance::Levenshtein>&& metric, Match&& first_match,
                const int32_t max_edit_distance, const size_t minimum_exact_prefix)
      : metric_ptr_(std::move(metric)),
        traverser_ptr_(std::move(traverser)),
        max_edit_distance_(max_edit_distance),
        exact_prefix_(minimum_exact_prefix),
        first_match_(std::move(first_match)) {}

  size_t candidate_length() const {
    assert(traverser_ptr_);
    return exact_prefix_ + traverser_ptr_->GetDepth();
  }

  FuzzyMatching() : max_edit_distance_(0), exact_prefix_(0) {}

 private:
  std::unique_ptr<stringdistance::Levenshtein> metric_ptr_;
  std::unique_ptr<fsa::CodePointStateTraverser<codepointInnerTraverserType>> traverser_ptr_;
  const int32_t max_edit_distance_;
  const size_t exact_prefix_;
  const Match first_match_;

  // reset method for the index in the special case the match is deleted
  template <class MatcherT, class DeletedT>
  friend Match index::internal::NextFilteredMatchSingle(const MatcherT&, const DeletedT&);
  template <class MatcherT, class DeletedT>
  friend Match index::internal::NextFilteredMatch(const MatcherT&, const DeletedT&);

  void ResetLastMatch() {}
};

} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
#endif  // KEYVI_DICTIONARY_MATCHING_FUZZY_MATCHING_H_
