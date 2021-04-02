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

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/match_iterator.h"
#include "keyvi/dictionary/matching/fuzzy_matching.h"
#include "keyvi/index/internal/read_only_segment.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

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
   * @param minimum_exact_prefix prefix length to be matched exact
   */
  dictionary::MatchIterator::MatchIteratorPair GetFuzzy(const std::string& query, const int32_t max_edit_distance,
                                                        const size_t minimum_exact_prefix = 2) {
    TRACE("matching fuzzy: %s max edit distance %ld minimum prefix %ld", query.c_str(), max_edit_distance,
          minimum_exact_prefix);
    const_segments_t segments = payload_.Segments();

    if (segments->size() == 0) {
      return dictionary::MatchIterator::EmptyIteratorPair();
    }

    std::vector<dictionary::fsa::automata_t> fsas;
    for (auto it = segments->cbegin(); it != segments->cend(); it++) {
      fsas.push_back((*it)->GetDictionary()->GetFsa());
    }

    std::vector<std::pair<dictionary::fsa::automata_t, uint64_t>> fsa_start_state_pairs =
        dictionary::matching::FuzzyMatching<>::FilterWithExactPrefix(fsas, query, minimum_exact_prefix);

    if (fsa_start_state_pairs.size() == 0) {
      return dictionary::MatchIterator::EmptyIteratorPair();
    }

    if (fsa_start_state_pairs.size() == 1) {
      auto fuzzy_matcher = std::make_shared<dictionary::matching::FuzzyMatching<>>(
          dictionary::matching::FuzzyMatching<>::FromSingleFsa<>(fsa_start_state_pairs[0].first, query,
                                                                 max_edit_distance, minimum_exact_prefix));

      for (auto it = segments->crbegin(); it != segments->crend(); it++) {
        if ((*it)->GetDictionary()->GetFsa() == fsa_start_state_pairs[0].first) {
          typename SegmentT::deleted_ptr_t deleted_keys = (*it)->DeletedKeys();
          if ((*it)->DeletedKeysSize() > 0) {
            auto func = [fuzzy_matcher, deleted_keys]() {
              dictionary::Match m = fuzzy_matcher->NextMatch();

              TRACE("check if key is deleted");
              while (m.IsEmpty() == false) {
                if (deleted_keys->count(m.GetMatchedString()) > 0) {
                  m = fuzzy_matcher->NextMatch();
                  continue;
                }

                break;
              }
              return m;
            };

            // check if first match is a deleted key and reset in case
            dictionary::Match first_match = fuzzy_matcher->FirstMatch();
            if (fuzzy_matcher->FirstMatch().IsEmpty() == false) {
              if (deleted_keys->count(first_match.GetMatchedString()) > 0) {
                first_match = dictionary::Match();
              }
            }
            return dictionary::MatchIterator::MakeIteratorPair(func, first_match);
          }
          break;  // else: found the fsa, but segments has no deletes
        }
      }

      auto func = [fuzzy_matcher]() { return fuzzy_matcher->NextMatch(); };
      return dictionary::MatchIterator::MakeIteratorPair(func, fuzzy_matcher->FirstMatch());
    }

    TRACE("collect deleted keys");
    // segments and filtered fsa's must have the same order
    std::map<dictionary::fsa::automata_t, typename SegmentT::deleted_ptr_t> deleted_keys_map;
    auto segments_it = segments->cbegin();
    for (auto fsa : fsa_start_state_pairs) {
      while (fsa.first != (*segments_it)->GetDictionary()->GetFsa()) {
        ++segments_it;
        // this should never happen
        if (segments_it == segments->end()) {
          throw std::runtime_error("order of segments do not match expected order");
        }
      }
      TRACE("found corresponding sement");
      if ((*segments_it)->DeletedKeysSize() > 0) {
        deleted_keys_map.emplace(fsa.first, (*segments_it)->DeletedKeys());
      }
      ++segments_it;
    }

    TRACE("create the fuzzy matcher");

    auto fuzzy_matcher = std::make_shared<
        dictionary::matching::FuzzyMatching<dictionary::fsa::ZipStateTraverser<dictionary::fsa::StateTraverser<>>>>(
        dictionary::matching::FuzzyMatching<dictionary::fsa::ZipStateTraverser<dictionary::fsa::StateTraverser<>>>::
            FromMulipleFsas<dictionary::fsa::StateTraverser<>>(fsa_start_state_pairs, query, max_edit_distance,
                                                               minimum_exact_prefix));

    if (deleted_keys_map.size() == 0) {
      auto func = [fuzzy_matcher]() { return fuzzy_matcher->NextMatch(); };
      return dictionary::MatchIterator::MakeIteratorPair(func, fuzzy_matcher->FirstMatch());
    }

    auto func = [fuzzy_matcher, deleted_keys_map]() {
      dictionary::Match m = fuzzy_matcher->NextMatch();

      TRACE("check if key is deleted");
      while (m.IsEmpty() == false) {
        auto dk = deleted_keys_map.find(m.GetFsa());
        if (dk != deleted_keys_map.end()) {
          if (dk->second->count(m.GetMatchedString()) > 0) {
            m = fuzzy_matcher->NextMatch();
            continue;
          }
        }
        break;
      }
      return m;
    };

    // check if first match is a deleted key and reset in case
    dictionary::Match first_match = fuzzy_matcher->FirstMatch();
    if (fuzzy_matcher->FirstMatch().IsEmpty() == false) {
      auto dk = deleted_keys_map.find(first_match.GetFsa());
      if (dk != deleted_keys_map.end()) {
        if (dk->second->count(first_match.GetMatchedString()) > 0) {
          // empty the first match
          first_match = dictionary::Match();
        }
      }
    }

    return dictionary::MatchIterator::MakeIteratorPair(func, first_match);
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
