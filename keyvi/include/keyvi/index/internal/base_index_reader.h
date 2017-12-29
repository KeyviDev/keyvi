//
// keyvi - A key value store.
//
// Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * base_index_reader.h
 *
 *  Created on: Nov 1, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INTERNAL_BASE_INDEX_READER_H_
#define KEYVI_INDEX_INTERNAL_BASE_INDEX_READER_H_

#include <string>

#include "dictionary/match.h"
#include "index/internal/segment.h"

namespace keyvi {
namespace index {
namespace internal {

template <class PayloadT>
class BaseIndexReader {
 public:
  template <typename... Args>
  explicit BaseIndexReader(Args... args) : payload_(args...) {}

  dictionary::Match operator[](const std::string& key) const {
    dictionary::Match m;
    segments_t segments = payload_.Segments();

    for (auto it = segments->crbegin(); it != segments->crend(); ++it) {
      m = (*it)->GetDictionary()->operator[](key);
      if (!m.IsEmpty()) {
        return m;
      }
    }

    return m;
  }

  bool Contains(const std::string& key) const {
    segments_t segments = payload_.Segments();
    for (auto it = segments->crbegin(); it != segments->crend(); it++) {
      if ((*it)->GetDictionary()->Contains(key)) {
        return true;
      }
    }

    return false;
  }

  PayloadT& Payload() { return payload_; }

 private:
  PayloadT payload_;
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_BASE_INDEX_READER_H_
