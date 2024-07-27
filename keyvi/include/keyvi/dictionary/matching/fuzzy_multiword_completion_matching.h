/* keyvi - A key value store.
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
 * fuzzy_multiword_completion_matching.h
 */

#ifndef KEYVI_DICTIONARY_MATCHING_FUZZY_MULTIWORD_COMPLETION_MATCHING_H_
#define KEYVI_DICTIONARY_MATCHING_FUZZY_MULTIWORD_COMPLETION_MATCHING_H_

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/codepoint_state_traverser.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/dictionary/fsa/zip_state_traverser.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/util/transform.h"
#include "keyvi/dictionary/util/utf8_utils.h"
#include "keyvi/stringdistance/levenshtein.h"
#include "utf8.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {
template <class MatcherT, class DeletedT>
keyvi::dictionary::match_t NextFilteredMatchSingle(const MatcherT&, const DeletedT&);
template <class MatcherT, class DeletedT>
keyvi::dictionary::match_t NextFilteredMatch(const MatcherT&, const DeletedT&);
}  // namespace internal
}  // namespace index
namespace dictionary {
namespace matching {

template <class innerTraverserType = fsa::CodePointStateTraverser<fsa::WeightedStateTraverser>>
class FuzzyMultiwordCompletionMatching final {
 public:
  /**
   * Create a fuzzy multiword completer from a single Fsa
   *
   * @param fsa the fsa
   * @param query the query
   */
  static FuzzyMultiwordCompletionMatching FromSingleFsa(const fsa::automata_t& fsa, const std::string& query,
                                                        const int32_t max_edit_distance,
                                                        const size_t minimum_exact_prefix = 0,
                                                        const unsigned char multiword_separator = 0x1b) {
    return FromSingleFsa(fsa, fsa->GetStartState(), query, max_edit_distance, minimum_exact_prefix,
                         multiword_separator);
  }

  /**
   * Create a fuzzy multiword completer from a single Fsa
   *
   * @param fsa the fsa
   * @param start_state the state to start from
   * @param query the query
   */
  static FuzzyMultiwordCompletionMatching FromSingleFsa(const fsa::automata_t& fsa, const uint64_t start_state,
                                                        const std::string& query, const int32_t max_edit_distance,
                                                        const size_t minimum_exact_prefix = 0,
                                                        const unsigned char multiword_separator = 0x1b) {
    if (start_state == 0) {
      return FuzzyMultiwordCompletionMatching();
    }
    size_t number_of_tokens;
    std::string query_bow = util::Transform::BagOfWordsPartial(query, number_of_tokens);

    std::vector<uint32_t> codepoints;
    utf8::unchecked::utf8to32(query_bow.begin(), query_bow.end(), back_inserter(codepoints));
    const size_t utf8_query_length = codepoints.size();

    if (utf8_query_length < minimum_exact_prefix) {
      return FuzzyMultiwordCompletionMatching();
    }

    std::unique_ptr<stringdistance::LevenshteinCompletion> metric =
        std::make_unique<stringdistance::LevenshteinCompletion>(codepoints, 20, max_edit_distance);

    size_t depth = 0;
    uint64_t state = start_state;
    size_t utf8_depth = 0;
    // match exact
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

    if (state == 0 || depth != minimum_exact_prefix) {
      return FuzzyMultiwordCompletionMatching();
    }

    TRACE("matched prefix, length %d", depth);

    std::unique_ptr<innerTraverserType> traverser = std::make_unique<innerTraverserType>(fsa, state);

    match_t first_match;
    if (depth == utf8_query_length && fsa->IsFinalState(state)) {
      TRACE("first_match %d %s", utf8_query_length, query);
      first_match = std::make_shared<Match>(0, utf8_query_length, query, 0, fsa, fsa->GetStateValue(state));
    }

    return FuzzyMultiwordCompletionMatching(std::move(traverser), std::move(first_match), std::move(metric),
                                            max_edit_distance, minimum_exact_prefix, number_of_tokens,
                                            multiword_separator);
  }

  /**
   * Create a fuzzy multiword completer from multiple Fsas
   *
   * @param fsas a vector of fsas
   * @param query the query
   */
  static FuzzyMultiwordCompletionMatching FromMulipleFsas(const std::vector<fsa::automata_t>& fsas,
                                                          const std::string& query, const int32_t max_edit_distance,
                                                          const size_t minimum_exact_prefix = 0,
                                                          const unsigned char multiword_separator = 0x1b) {
    size_t number_of_tokens;
    std::string query_bow = util::Transform::BagOfWordsPartial(query, number_of_tokens);

    std::vector<uint32_t> codepoints;
    utf8::unchecked::utf8to32(query_bow.begin(), query_bow.end(), back_inserter(codepoints));
    const size_t query_length = codepoints.size();

    std::unique_ptr<stringdistance::LevenshteinCompletion> metric =
        std::make_unique<stringdistance::LevenshteinCompletion>(codepoints, 20, max_edit_distance);

    std::vector<std::pair<fsa::automata_t, uint64_t>> fsa_start_state_pairs;

    // match the exact prefix on all fsas
    for (const fsa::automata_t& fsa : fsas) {
      uint64_t state = fsa->GetStartState();
      size_t depth, utf8_depth = 0;

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

      if (state != 0 && depth == minimum_exact_prefix) {
        fsa_start_state_pairs.emplace_back(fsa, state);
      }
    }

    if (fsa_start_state_pairs.size() == 0) {
      return FuzzyMultiwordCompletionMatching();
    }

    // fill the metric
    for (size_t utf8_depth = 0; utf8_depth < minimum_exact_prefix; ++utf8_depth) {
      metric->Put(codepoints[utf8_depth], utf8_depth);
    }

    match_t first_match;
    // check for a match given the exact prefix
    for (const auto& fsa_state : fsa_start_state_pairs) {
      if (fsa_state.first->IsFinalState(fsa_state.second)) {
        first_match = std::make_shared<Match>(0, query_length, query, 0, fsa_state.first,
                                              fsa_state.first->GetStateValue(fsa_state.second));
        break;
      }
    }

    std::unique_ptr<innerTraverserType> traverser = std::make_unique<innerTraverserType>(fsa_start_state_pairs);

    return FuzzyMultiwordCompletionMatching(std::move(traverser), std::move(first_match), std::move(metric),
                                            minimum_exact_prefix, number_of_tokens, multiword_separator);
  }

  match_t& FirstMatch() { return first_match_; }

  match_t NextMatch() {
    for (; traverser_ptr_ && *traverser_ptr_; (*traverser_ptr_)++) {
      uint64_t label = traverser_ptr_->GetStateLabel();
      TRACE("label [%c] prefix length %ld traverser depth: %ld", label, prefix_length_, traverser_ptr_->GetDepth());

      while (token_start_positions_.size() > 0 && traverser_ptr_->GetDepth() <= token_start_positions_.back()) {
        TRACE("pop token stack");
        token_start_positions_.pop_back();
      }

      if (label == multiword_separator_) {
        TRACE("found MW boundary at %d", traverser_ptr_->GetDepth());
        if (token_start_positions_.size() != number_of_tokens_ - 1) {
          TRACE("found MW boundary before seeing enough tokens (%d/%d)", token_start_positions_.size(),
                number_of_tokens_);
          traverser_ptr_->Prune();
          TRACE("pruned, now at %d", traverser_ptr_->GetDepth());
          continue;
        }

        multiword_boundary_ = traverser_ptr_->GetDepth();
      } else if (traverser_ptr_->GetDepth() <= multiword_boundary_) {
        // reset the multiword boundary if we went up
        multiword_boundary_ = 0;
        TRACE("reset MW boundary at %d %d", traverser_ptr_->GetDepth(), multiword_boundary_);
      }

      // only match up to the number of tokens in input
      if (label == 0x20 && multiword_boundary_ == 0) {
        // todo: should every token be matched with the exact prefix, except for the last token?
        TRACE("push space(%d)", token_start_positions_.size());
        token_start_positions_.push_back(traverser_ptr_->GetDepth());
      }

      int32_t intermediate_score = distance_metric_->Put(label, prefix_length_ + traverser_ptr_->GetDepth() - 1);

      TRACE("Candidate: [%s] %ld intermediate score: %d(%d)", distance_metric_->GetCandidate().c_str(),
            prefix_length_ + traverser_ptr_->GetDepth() - 1, intermediate_score, max_edit_distance_);

      if (intermediate_score > max_edit_distance_) {
        traverser_ptr_->Prune();
        continue;
      }

      if (traverser_ptr_->IsFinalState()) {
        std::string match_str = multiword_boundary_ > 0
                                    ? distance_metric_->GetCandidate(prefix_length_ + multiword_boundary_)
                                    : distance_metric_->GetCandidate();

        TRACE("found final state at depth %d %s", prefix_length_ + traverser_ptr_->GetDepth(), match_str.c_str());
        match_t m = std::make_shared<Match>(0, prefix_length_ + traverser_ptr_->GetDepth(), match_str,
                                            distance_metric_->GetScore(), traverser_ptr_->GetFsa(),
                                            traverser_ptr_->GetStateValue());

        (*traverser_ptr_)++;
        return m;
      }
    }

    return match_t();
  }

  void SetMinWeight(uint32_t min_weight) { traverser_ptr_->SetMinWeight(min_weight); }

 private:
  FuzzyMultiwordCompletionMatching(std::unique_ptr<innerTraverserType>&& traverser, match_t&& first_match,
                                   std::unique_ptr<stringdistance::LevenshteinCompletion>&& distance_metric,
                                   const int32_t max_edit_distance, const size_t prefix_length, size_t number_of_tokens,
                                   const unsigned char multiword_separator)
      : traverser_ptr_(std::move(traverser)),
        first_match_(std::move(first_match)),
        distance_metric_(std::move(distance_metric)),
        max_edit_distance_(max_edit_distance),
        prefix_length_(prefix_length),
        number_of_tokens_(number_of_tokens),
        multiword_separator_(static_cast<uint64_t>(multiword_separator)) {}

  FuzzyMultiwordCompletionMatching() {}

 private:
  std::unique_ptr<innerTraverserType> traverser_ptr_;
  match_t first_match_;
  std::unique_ptr<stringdistance::LevenshteinCompletion> distance_metric_;
  const int32_t max_edit_distance_ = 0;
  const size_t prefix_length_ = 0;
  const size_t number_of_tokens_ = 0;
  const uint64_t multiword_separator_ = 0;
  std::vector<size_t> token_start_positions_;
  size_t multiword_boundary_ = 0;

  // reset method for the index in the special case the match is deleted
  template <class MatcherT, class DeletedT>
  friend match_t index::internal::NextFilteredMatchSingle(const MatcherT&, const DeletedT&);
  template <class MatcherT, class DeletedT>
  friend match_t index::internal::NextFilteredMatch(const MatcherT&, const DeletedT&);

  void ResetLastMatch() {}
};

} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
#endif  // KEYVI_DICTIONARY_MATCHING_FUZZY_MULTIWORD_COMPLETION_MATCHING_H_
