//
// keyvi - A key value store.
//
// Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#include <memory>
#include <string>
#include <vector>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/match_iterator.h"
#include "keyvi/dictionary/matching/fuzzy_matching.h"
#include "keyvi/index/internal/read_only_segment.h"

namespace keyvi {
namespace index {
namespace unit_test {
class IndexFriend;
}
namespace internal {

template <class PayloadT, class SegmentT = ReadOnlySegment>
class BaseIndexReader {
 public:
  using const_segments_t = const std::shared_ptr<std::vector<std::shared_ptr<SegmentT>>>;

  template <typename... Args>
  explicit BaseIndexReader(Args... args) : payload_(args...) {}

  /**
   * Get a match for the given key
   *
   * @param key the key
   */
  dictionary::Match operator[](const std::string& key) {
    dictionary::Match match;
    const_segments_t segments = payload_.Segments();

    for (auto it = segments->crbegin(); it != segments->crend(); ++it) {
      match = (*it)->GetDictionary()->operator[](key);
      if (!match.IsEmpty()) {
        if ((*it)->IsDeleted(key)) {
          return dictionary::Match();
        }
        return match;
      }
    }

    return match;
  }

  /**
   * Check if an entry for a given key exists
   *
   * @param key the key
   */
  bool Contains(const std::string& key) {
    const_segments_t segments = payload_.Segments();
    for (auto it = segments->crbegin(); it != segments->crend(); it++) {
      if ((*it)->GetDictionary()->Contains(key)) {
        return !(*it)->IsDeleted(key);
      }
    }

    return false;
  }

  /**
   * Match approximate given the query and edit distance
   *
   * @param query a query to match against
   * @param max_edit_distance the max edit distance allowed for a single match
   */
  dictionary::MatchIterator::MatchIteratorPair GetFuzzy(const std::string& query, size_t max_edit_distance) const {
    const_segments_t segments = payload_.Segments();
    std::vector<dictionary::fsa::automata_t> fsas;
    for (auto it = segments->cbegin(); it != segments->cend(); it++) {
      fsas.push_back((*it)->GetDictionary()->GetFsa());
    }
    auto data = std::make_shared<dictionary::matching::FuzzyMatching<>>(
        dictionary::matching::FuzzyMatching<>::FromMulipleFsas(fsas, query, max_edit_distance));

    auto func = [data]() { return data->NextMatch(); };
    return dictionary::MatchIterator::MakeIteratorPair(func, data->FirstMatch());
  }

 protected:
  PayloadT& Payload() { return payload_; }

 private:
  PayloadT payload_;

  // friend for unit testing only
  friend class keyvi::index::unit_test::IndexFriend;
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_BASE_INDEX_READER_H_
