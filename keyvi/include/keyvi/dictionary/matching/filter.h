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

#include <utility>

#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/util/bounded_priority_queue.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace matching {

using filter_result_t = std::pair<bool, uint32_t>;
using filter_t = std::function<filter_result_t(const Match&)>;
using filter_wrapper_t = std::function<filter_result_t(const Match&, void*)>;

inline filter_result_t accept_all(const Match& m) {
  return filter_result_t(true, 0);
}
namespace filter {
class TopN final {
 public:
  TopN(size_t n) : priority_queue_(n) {}

  filter_result_t filter(const Match& m) {
    if (m.GetWeight() < priority_queue_.Back()) {
      return filter_result_t(false, priority_queue_.Back());
    }

    priority_queue_.Put(m.GetWeight());
    return filter_result_t(true, priority_queue_.Back());
  }

 private:
  util::BoundedPriorityQueue<uint32_t> priority_queue_;
};

/**
 * A wrapper around a filter that can hold an object internally.
 * Used for interfacing with bindings, e.g. to implement filter code in python.
 */
class FilterWrapper final {
 public:
  FilterWrapper(filter_wrapper_t filter, void* user_data) : inner_filter_(filter), user_data_(user_data) {}

  filter_result_t filter(const Match& m) { return (inner_filter_(m, user_data_)); }

 private:
  filter_wrapper_t inner_filter_;
  void* user_data_;
};

} /* namespace filter */
} /* namespace matching */
} /* namespace dictionary */
} /* namespace keyvi */
#endif  // KEYVI_DICTIONARY_MATCHING_FILTER_H_
