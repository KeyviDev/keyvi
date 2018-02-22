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

#include "index/constants.h"

#include "util/configuration.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class IndexSettings final {
 public:
  explicit IndexSettings(const keyvi::util::parameters_t& params) : settings_() {
    // only parse what we know
    if (params.count(KEYVIMERGER_BIN)) {
      settings_[KEYVIMERGER_BIN] = params.at(KEYVIMERGER_BIN);
    }
  }

  const std::string& GetKeyviMergerBin() const {
    if (settings_.count(KEYVIMERGER_BIN)) {
      return settings_.at(KEYVIMERGER_BIN);
    }

    return default_keyvimerger_bin_;
  }

 private:
  std::unordered_map<std::string, std::string> settings_;
  const std::string default_keyvimerger_bin_ = "keyvimerger";
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_SETTINGS_H_
