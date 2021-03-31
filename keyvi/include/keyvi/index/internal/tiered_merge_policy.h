//
// keyvi - A key value store.
//
// Copyright 2019 Hendrik Muhs<hendrik.muhs@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/*
 * tiered_merge_policy.h
 *
 *  Created on: Feb 11, 2019
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INTERNAL_TIERED_MERGE_POLICY_H_
#define KEYVI_INDEX_INTERNAL_TIERED_MERGE_POLICY_H_

#include <algorithm>
#include <cmath>
#include <vector>

#include "keyvi/index/internal/merge_policy.h"
#include "keyvi/index/internal/segment.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

static const size_t TIERED_MERGE_MAX_SEGMENT_PER_MERGE = 20;
static const size_t TIERED_MERGE_FLOOR_SEGMENT_KEY_SIZE = 10000;

/*
 * Tiered merge policy, inspired by
 * https://github.com/apache/lucene-solr/blob/master/lucene/core/src/java/org/apache/lucene/index/TieredMergePolicy.java
 *
 * Some fundamental differences:
 *
 * - merge must pick adjacent segments (lucene can pick any segments)
 * - we use the number of keys not the size
 */
class TieredMergePolicy final : public MergePolicy {
 public:
  TieredMergePolicy() {}

  inline void MergeFinished(const size_t id) {}

  inline bool SelectMergeSegments(const segments_t& segments, std::vector<segment_t>* elected_segments, size_t* id) {
    std::vector<segment_t> candidate;

    double best_score = -1;
    std::vector<segment_t> best_candidate;

    for (size_t start_index = 0; start_index < segments->size(); ++start_index) {
      size_t totalSizeToMerge = 0;
      candidate.clear();
      for (size_t index = start_index; index < segments->size(); ++index) {
        const segment_t current = (*segments)[index];
        if (current->MarkedForMerge()) {
          break;
        }

        candidate.push_back(current);
        totalSizeToMerge += current->GetDictionaryProperties()->GetNumberOfKeys();

        if (candidate.size() == TIERED_MERGE_MAX_SEGMENT_PER_MERGE) {
          break;
        }
      }

      // skip empty candidate lists
      if (candidate.size() == 0) {
        continue;
      }

      // skip single candidates with no deletes
      if (candidate.size() == 1 && candidate[0]->HasDeletedKeys() == false) {
        continue;
      }

      const double score = ScoreCandidate(candidate);

      if (best_score == -1 || score < best_score) {
        best_score = score;
        best_candidate.swap(candidate);
      }
    }

    if (best_candidate.size() > 0) {
      elected_segments->swap(best_candidate);
      return true;
    }

    return false;
  }

 private:
  /*
   * Score a candidate vector, smaller means better
   */
  inline double ScoreCandidate(const std::vector<segment_t>& candidate) {
    size_t total_size = 0;
    size_t total_size_floored = 0;

    size_t total_deletes = 0;
    size_t biggest_segment_key_size = 0;

    for (const segment_t& segment : candidate) {
      const size_t segment_key_size = segment->GetDictionaryProperties()->GetNumberOfKeys();
      total_size += segment_key_size;
      // floored sizes ensures a minimum size per segment to take fix costs of merging into account
      const size_t segment_key_size_floored = std::max(TIERED_MERGE_FLOOR_SEGMENT_KEY_SIZE, segment_key_size);
      total_size_floored += segment_key_size_floored;
      total_deletes += segment->DeletedKeysSize();
      biggest_segment_key_size = std::max(biggest_segment_key_size, segment_key_size);
    }

    // calculate the skew
    double score = static_cast<double>(biggest_segment_key_size) / total_size_floored;

    TRACE("skew: %.18g", total_size, candidate.size(), score);

    // gently favor smaller merges over bigger ones
    score *= std::pow(total_size, 0.05);

    // boost merges with deletes
    if (total_deletes > 0) {
      const double delete_ratio = static_cast<double>(total_size - total_deletes) / total_size;
      score *= std::pow(delete_ratio, 2);
    }

    TRACE("Candidate 1st element size %ld total size: %ld, number of segments: %ld score: %.18g",
          candidate[0]->GetDictionaryProperties()->GetNumberOfKeys(), total_size, candidate.size(), score);

    return score;
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_TIERED_MERGE_POLICY_H_
