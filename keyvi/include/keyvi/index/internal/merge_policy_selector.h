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
 * merge_policy_selector.h
 *
 *  Created on: Jan 14, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INTERNAL_MERGE_POLICY_SELECTOR_H_
#define KEYVI_INDEX_INTERNAL_MERGE_POLICY_SELECTOR_H_

#include <memory>
#include <string>

#include "keyvi/index/internal/merge_policy.h"
#include "keyvi/index/internal/simple_merge_policy.h"
#include "keyvi/index/internal/tiered_merge_policy.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

inline std::shared_ptr<MergePolicy> merge_policy(const std::string& name = "") {
  auto lower_name = name;

  TRACE("Merge Policy: %s", name.c_str());

  boost::algorithm::to_lower(lower_name);
  if (lower_name == "simple") {
    return std::make_shared<SimpleMergePolicy>();
  } else if (lower_name == "tiered") {
    return std::make_shared<TieredMergePolicy>();
  } else {
    throw std::invalid_argument(name + " is not a valid merge policy");
  }
}  // namespace internal

typedef std::shared_ptr<MergePolicy> merge_policy_t;

}  // namespace internal
}  // namespace index
}  // namespace keyvi

#endif  // KEYVI_INDEX_INTERNAL_MERGE_POLICY_SELECTOR_H_
