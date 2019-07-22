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

#ifndef KEYVI_INDEX_INTERNAL_MERGE_POLICY_H_
#define KEYVI_INDEX_INTERNAL_MERGE_POLICY_H_

#include <cstddef>
#include <vector>

#include "keyvi/index/internal/segment.h"

namespace keyvi {
namespace index {
namespace internal {

class MergePolicy {
 public:
  MergePolicy() {}

  virtual ~MergePolicy() = default;

  virtual void MergeFinished(const size_t id) = 0;

  virtual bool SelectMergeSegments(const segments_t& segments, std::vector<segment_t>* elected_segments,
                                   size_t* id) = 0;
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_MERGE_POLICY_H_
