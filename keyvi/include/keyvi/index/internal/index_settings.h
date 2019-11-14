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
 * index_settings.h
 *
 *  Created on: Feb 5, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INTERNAL_INDEX_SETTINGS_H_
#define KEYVI_INDEX_INTERNAL_INDEX_SETTINGS_H_

#include <string>
#include <unordered_map>

#include <boost/variant.hpp>

#include "keyvi/index/constants.h"
#include "keyvi/index/internal/index_auto_config.h"
#include "keyvi/util/configuration.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class IndexSettings final {
 public:
  explicit IndexSettings(const keyvi::util::parameters_t& params) : settings_() {
    // only parse what we know
    if (params.count(KEYVIMERGER_BIN)) {
      settings_[KEYVIMERGER_BIN] = params.at(KEYVIMERGER_BIN);
    } else {
      settings_[KEYVIMERGER_BIN] = DEFAULT_KEYVIMERGER_BIN;
    }
    if (params.count(INDEX_MAX_SEGMENTS)) {
      settings_[INDEX_MAX_SEGMENTS] = keyvi::util::mapGet<size_t>(params, INDEX_MAX_SEGMENTS);
    } else {
      settings_[INDEX_MAX_SEGMENTS] = IndexAutoConfig::MaxSegments();
    }
    if (params.count(MAX_CONCURRENT_MERGES)) {
      settings_[MAX_CONCURRENT_MERGES] = keyvi::util::mapGet<size_t>(params, MAX_CONCURRENT_MERGES);
    } else {
      settings_[MAX_CONCURRENT_MERGES] = IndexAutoConfig::MaxConcurrentMerges();
    }
    if (params.count(SEGMENT_COMPILE_KEY_THRESHOLD)) {
      settings_[SEGMENT_COMPILE_KEY_THRESHOLD] = keyvi::util::mapGet<size_t>(params, SEGMENT_COMPILE_KEY_THRESHOLD);
    } else {
      settings_[SEGMENT_COMPILE_KEY_THRESHOLD] = DEFAULT_COMPILE_KEY_THRESHOLD;
    }
    if (params.count(INDEX_REFRESH_INTERVAL)) {
      settings_[INDEX_REFRESH_INTERVAL] = keyvi::util::mapGet<size_t>(params, INDEX_REFRESH_INTERVAL);
    } else {
      settings_[INDEX_REFRESH_INTERVAL] = DEFAULT_REFRESH_INTERVAL;
    }
    if (params.count(SEGMENT_EXTERNAL_MERGE_KEY_THRESHOLD)) {
      settings_[SEGMENT_EXTERNAL_MERGE_KEY_THRESHOLD] =
          keyvi::util::mapGet<size_t>(params, SEGMENT_EXTERNAL_MERGE_KEY_THRESHOLD);
    } else {
      settings_[SEGMENT_EXTERNAL_MERGE_KEY_THRESHOLD] = DEFAULT_EXTERNAL_MERGE_KEY_THRESHOLD;
    }
  }

  const std::string& GetKeyviMergerBin() const { return boost::get<std::string>(settings_.at(KEYVIMERGER_BIN)); }

  const size_t GetMaxSegments() const { return boost::get<size_t>(settings_.at(INDEX_MAX_SEGMENTS)); }

  const size_t GetSegmentCompileKeyThreshold() const {
    return boost::get<size_t>(settings_.at(SEGMENT_COMPILE_KEY_THRESHOLD));
  }

  const size_t GetMaxConcurrentMerges() const { return boost::get<size_t>(settings_.at(MAX_CONCURRENT_MERGES)); }

  const size_t GetRefreshInterval() const { return boost::get<size_t>(settings_.at(INDEX_REFRESH_INTERVAL)); }

  const size_t GetSegmentExternalMergeKeyThreshold() const {
    return boost::get<size_t>(settings_.at(SEGMENT_EXTERNAL_MERGE_KEY_THRESHOLD));
  }

 private:
  std::unordered_map<std::string, boost::variant<std::string, size_t>> settings_;
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_SETTINGS_H_
