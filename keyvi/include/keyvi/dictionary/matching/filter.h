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
 * filter.h
 */

#ifndef KEYVI_DICTIONARY_MATCHING_FILTER_H_
#define KEYVI_DICTIONARY_MATCHING_FILTER_H_

#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/util/bounded_priority_queue.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace matching {

struct FilterResult {
  FilterResult() {}
  FilterResult(bool a, uint32_t m) : accept(a), min_weight(m) {}

  bool accept = true;
  uint32_t min_weight = 0;
};

using filter_t = std::function<FilterResult(const Match&)>;

inline FilterResult accept_all(const Match& m) {
  return FilterResult();
}
namespace filter {
class TopN final {
 public:
  TopN(size_t n) : priority_queue_(n) {}

  FilterResult filter(const Match& m) {
    if (m.GetWeight() < priority_queue_.Back()) {
      return FilterResult(false, priority_queue_.Back());
    }

    priority_queue_.Put(m.GetWeight());
    return FilterResult(true, priority_queue_.Back());
  }

 private:
  util::BoundedPriorityQueue<uint32_t> priority_queue_;
};

} /* namespace filter */
} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
#endif  // KEYVI_DICTIONARY_MATCHING_FILTER_H_
