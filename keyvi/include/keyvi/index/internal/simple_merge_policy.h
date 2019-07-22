//
// keyvi - A key value store.
//
// Copyright 2017 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#ifndef KEYVI_INDEX_INTERNAL_SIMPLE_MERGE_POLICY_H_
#define KEYVI_INDEX_INTERNAL_SIMPLE_MERGE_POLICY_H_

#include <vector>

#include "keyvi/index/internal/merge_policy.h"
#include "keyvi/index/internal/segment.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class SimpleMergePolicy final : public MergePolicy {
 public:
  SimpleMergePolicy() {}

  inline void MergeFinished(const size_t id) {}

  inline bool SelectMergeSegments(const segments_t& segments, std::vector<segment_t>* elected_segments, size_t* id) {
    std::vector<segment_t> to_merge;
    for (segment_t& s : *segments) {
      if (!s->MarkedForMerge()) {
        TRACE("Add to merge list %s", s->GetDictionaryFilename().c_str());
        to_merge.push_back(s);
      }

      if (to_merge.size() > MAX_SEGMENT_PER_MERGE) {
        break;
      }
    }

    *id = 0;
    if ((to_merge.size() > 1) || (to_merge.size() == 1 && to_merge[0]->HasDeletedKeys())) {
      elected_segments->swap(to_merge);
      return true;
    }

    return false;
  }

 private:
  static const size_t MAX_SEGMENT_PER_MERGE = 500;
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_SIMPLE_MERGE_POLICY_H_
