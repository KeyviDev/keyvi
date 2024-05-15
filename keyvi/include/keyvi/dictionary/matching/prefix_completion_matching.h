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
 * prefix_completion_matching.h
 */

#ifndef KEYVI_DICTIONARY_MATCHING_PREFIX_COMPLETION_MATCHING_H_
#define KEYVI_DICTIONARY_MATCHING_PREFIX_COMPLETION_MATCHING_H_

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

template <class innerTraverserType = fsa::WeightedStateTraverser>
class PrefixCompletionMatching final {
 public:
  /**
   * Create a prefix completer from a single Fsa
   *
   * @param fsa the fsa
   * @param query the query
   */
  static PrefixCompletionMatching FromSingleFsa(const fsa::automata_t& fsa, const std::string& query) {
    return FromSingleFsa(fsa, fsa->GetStartState(), query);
  }

  /**
   * Create a prefix completer from a single Fsa
   *
   * @param fsa the fsa
   * @param start_state the state to start from
   * @param query the query
   */
  static PrefixCompletionMatching FromSingleFsa(const fsa::automata_t& fsa, const uint64_t start_state,
                                                const std::string& query) {
    if (start_state == 0) {
      return PrefixCompletionMatching();
    }

    std::unique_ptr<std::vector<unsigned char>> traversal_stack = std::make_unique<std::vector<unsigned char>>();
    traversal_stack->reserve(1024);

    const size_t query_length = query.size();
    size_t depth = 0;
    uint64_t state = start_state;

    match_t first_match;

    TRACE("start state %d", state);

    while (state != 0 && depth != query_length) {
      traversal_stack->push_back(query[depth]);
      state = fsa->TryWalkTransition(state, query[depth++]);
    }

    TRACE("state %d", state);

    if (state == 0) {
      return PrefixCompletionMatching();
    }

    TRACE("matched prefix, length %d", depth);

    std::unique_ptr<innerTraverserType> traverser = std::make_unique<innerTraverserType>(fsa, state);

    if (fsa->IsFinalState(state)) {
      first_match = std::make_shared<Match>(0, query_length, query, 0, fsa, fsa->GetStateValue(state));
    }

    TRACE("create matcher");
    return PrefixCompletionMatching(std::move(traverser), std::move(first_match), std::move(traversal_stack),
                                    query_length);
  }

  /**
   * Create a prefix completer from multiple Fsas
   *
   * @param fsas a vector of fsas
   * @param query the query
   */
  static PrefixCompletionMatching FromMulipleFsas(const std::vector<fsa::automata_t>& fsas, const std::string& query) {
    const size_t query_length = query.size();
    std::vector<std::pair<fsa::automata_t, uint64_t>> fsa_start_state_pairs;

    for (const fsa::automata_t& fsa : fsas) {
      uint64_t state = fsa->GetStartState();

      size_t depth = 0;
      while (state != 0 && depth != query_length) {
        state = fsa->TryWalkTransition(state, query[depth++]);
      }

      if (state != 0) {
        fsa_start_state_pairs.emplace_back(fsa, state);
      }
    }

    if (fsa_start_state_pairs.size() == 0) {
      return PrefixCompletionMatching();
    }

    // create the traversal stack
    std::unique_ptr<std::vector<unsigned char>> traversal_stack = std::make_unique<std::vector<unsigned char>>();
    traversal_stack->reserve(1024);

    for (const char& c : query) {
      traversal_stack->push_back(c);
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

    return PrefixCompletionMatching(std::move(traverser), std::move(first_match), std::move(traversal_stack),
                                    query_length);
  }

  match_t& FirstMatch() { return first_match_; }

  match_t NextMatch() {
    for (; traverser_ptr_ && *traverser_ptr_; (*traverser_ptr_)++) {
      traversal_stack_->resize(prefix_length_ + traverser_ptr_->GetDepth() - 1);
      traversal_stack_->push_back(traverser_ptr_->GetStateLabel());
      TRACE("Current depth %d (%d)", prefix_length_ + traverser_ptr_->GetDepth() - 1, traversal_stack_->size());

      if (traverser_ptr_->IsFinalState()) {
        std::string match_str = std::string(traversal_stack_->begin(), traversal_stack_->end());

        TRACE("found final state at depth %d %s", prefix_length_ + traverser_ptr_->GetDepth(), match_str.c_str());
        match_t m = std::make_shared<Match>(0, prefix_length_ + traverser_ptr_->GetDepth(), match_str, 0,
                                            traverser_ptr_->GetFsa(), traverser_ptr_->GetStateValue());

        (*traverser_ptr_)++;
        return m;
      }
    }

    return match_t();
  }

  void SetMinWeight(uint32_t min_weight) { traverser_ptr_->SetMinWeight(min_weight); }

 private:
  PrefixCompletionMatching(std::unique_ptr<innerTraverserType>&& traverser, match_t&& first_match,
                           std::unique_ptr<std::vector<unsigned char>>&& traversal_stack, const size_t prefix_length)
      : traverser_ptr_(std::move(traverser)),
        first_match_(std::move(first_match)),
        traversal_stack_(std::move(traversal_stack)),
        prefix_length_(prefix_length) {}

  PrefixCompletionMatching() {}

 private:
  std::unique_ptr<innerTraverserType> traverser_ptr_;
  match_t first_match_;
  std::unique_ptr<std::vector<unsigned char>> traversal_stack_;
  const size_t prefix_length_ = 0;

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
#endif  // KEYVI_DICTIONARY_MATCHING_PREFIX_COMPLETION_MATCHING_H_
