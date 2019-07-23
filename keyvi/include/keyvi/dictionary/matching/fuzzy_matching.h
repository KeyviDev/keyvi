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
#include <vector>

#include "utf8.h"

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/codepoint_state_traverser.h"
#include "keyvi/dictionary/fsa/traverser_types.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/util/utf8_utils.h"
#include "keyvi/stringdistance/levenshtein.h"

namespace keyvi {
namespace dictionary {
namespace matching {

class FuzzyMatching final {
 public:
  FuzzyMatching(const fsa::automata_t& fsa, const std::string& query, int32_t max_edit_distance)
      : fsa_(fsa), query_(query), max_edit_distance_(max_edit_distance), first_match_() {
    std::vector<uint32_t> codepoints;
    utf8::unchecked::utf8to32(query.begin(), query.end(), back_inserter(codepoints));

    query_char_length_ = codepoints.size();
    if (query_char_length_ < MINIMUM_EXACT_PREFIX_IN_CHARS) {
      return;
    }
    metric_ptr_.reset(new stringdistance::Levenshtein(codepoints, 20, max_edit_distance_));

    // match exact prefix
    uint64_t state = fsa_->GetStartState();
    size_t depth = 0;
    size_t utf8_depth = 0;
    while (state != 0 && depth < MINIMUM_EXACT_PREFIX_IN_CHARS) {
      const size_t code_point_length = util::Utf8Utils::GetCharLength(query[utf8_depth]);
      for (size_t i = 0; i < code_point_length; ++i, ++utf8_depth) {
        state = fsa_->TryWalkTransition(state, query[utf8_depth]);
        if (0 == state) {
          break;
        }
      }
      metric_ptr_->Put(codepoints[depth], depth);
      ++depth;
    }

    if (state) {
      if (depth == query_char_length_ && fsa_->IsFinalState(state)) {
        first_match_ = Match(0, query_char_length_, query, 0, fsa_, fsa_->GetStateValue(state));
      }
      traverser_ptr_.reset(new fsa::CodePointStateTraverser<fsa::WeightedStateTraverser>(fsa_, state));
    }
  }

  Match FirstMatch() const { return first_match_; }

  Match NextMatch() {
    for (; traverser_ptr_ && *traverser_ptr_; (*traverser_ptr_)++) {
      const int32_t intermediate_score = metric_ptr_->Put(traverser_ptr_->GetStateLabel(), candidate_length() - 1);
      // don't consider subtrees which can not be matched anyways
      if (query_char_length_ > candidate_length() && intermediate_score > max_edit_distance_) {
        traverser_ptr_->Prune();
        continue;
      }

      if (query_char_length_ + max_edit_distance_ < candidate_length()) {
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
    return MINIMUM_EXACT_PREFIX_IN_CHARS + traverser_ptr_->GetDepth();
  }

 private:
  fsa::automata_t fsa_;
  std::string query_;
  const int32_t max_edit_distance_;

  size_t query_char_length_ = 0;

  Match first_match_;

  std::unique_ptr<stringdistance::Levenshtein> metric_ptr_;
  std::unique_ptr<fsa::CodePointStateTraverser<fsa::WeightedStateTraverser>> traverser_ptr_;

  const size_t MINIMUM_EXACT_PREFIX_IN_CHARS = 2;
};

} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
#endif  // KEYVI_DICTIONARY_MATCHING_FUZZY_MATCHING_H_
