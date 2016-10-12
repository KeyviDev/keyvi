/* * keyvi - A key value store.
 *
 * Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * in_memory_sorter.h
 *
 *  Created on: Oct 11, 2016
 *      Author: hendrik
 */

#ifndef IN_MEMORY_SORTER_H_
#define IN_MEMORY_SORTER_H_

#include "dictionary/sort/sorter_common.h"
#include "dictionary/fsa/internal/constants.h"
//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace sort {

template<typename KeyValueT>
class InMemorySorter final {

 public:
  typedef typename std::vector<KeyValueT>::iterator iterator;

  /**
   * Instantiate a in memory sorter. This is a simple heap based sorter, everything must fit in memory.
   *
   * @param memory_limit ignored, everything is in memory, client has to ensure it fits in memory.
   */
  InMemorySorter(size_t memory_limit = 1073741824, const sorter_param_t& params = sorter_param_t())
      : params_(params) {
  }

  void push_back(const KeyValueT& kv) {
    key_values_.push_back(kv);
  }

  void sort() {
    std::sort(key_values_.begin(), key_values_.end());
  }

  size_t size() {
    return key_values_.size();
  }

  iterator begin() {
    return key_values_.begin();
  }

  iterator end() {
    return key_values_.end();
  }

  void clear() {
    key_values_.clear();
  }

 private:
  std::vector<KeyValueT> key_values_;
  sorter_param_t params_;
};

} /* namespace sort */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* IN_MEMORY_SORTER_H_ */
