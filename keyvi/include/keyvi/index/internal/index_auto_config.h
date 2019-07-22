/* * keyvi - A key value store.
 *
 * Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * index_auto_config.h
 *
 *  Created on: Apr 11, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INTERNAL_INDEX_AUTO_CONFIG_H_
#define KEYVI_INDEX_INTERNAL_INDEX_AUTO_CONFIG_H_

#include <algorithm>
#include <string>
#include <thread>  // NOLINT
#include <unordered_map>

#include "keyvi/index/constants.h"

#include "keyvi/util/configuration.h"
#include "keyvi/util/os_utils.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class IndexAutoConfig final {
 public:
  static size_t MaxSegments() {
    // increase file descriptors and leave some extra room, 1 segment == 1 file, but we want
    // to leave some room for the client.
    size_t file_descriptors = keyvi::util::OsUtils::TryIncreaseFileDescriptors();
    return file_descriptors - 100;
  }

  static size_t MaxConcurrentMerges() {
    unsigned int cores = std::thread::hardware_concurrency();

    // for now use half the cores for mergers
    size_t max_concurrent_merges = cores / 2;
    return std::min(MAX_CONCURRENT_MERGES_DEFAULT, std::max(size_t(1), max_concurrent_merges));
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_AUTO_CONFIG_H_
