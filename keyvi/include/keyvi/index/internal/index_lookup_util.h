/* * keyvi - A key value store.
 *
 * Copyright 2021 Hendrik Muhs<hendrik.muhs@gmail.com>
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

/**
 * Shared index lookup code for special matchers like fuzzy and near, mainly handlinge deletes in the index.
 */

#ifndef KEYVI_INDEX_INTERNAL_INDEX_LOOKUP_UTIL_H_
#define KEYVI_INDEX_INTERNAL_INDEX_LOOKUP_UTIL_H_

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/match.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

/**
 * Filters matches and drops results for deleted keys for a single FSA.
 */
template <class MatcherT, class DeletedT>
inline keyvi::dictionary::Match NextFilteredMatchSingle(const MatcherT& matcher, const DeletedT& deleted_keys) {
  keyvi::dictionary::Match m = matcher->NextMatch();

  TRACE("check if key is deleted");
  while (m.IsEmpty() == false) {
    if (deleted_keys->count(m.GetMatchedString()) > 0) {
      matcher->ResetLastMatch();
      m = matcher->NextMatch();
      continue;
    }

    break;
  }
  return m;
}

/**
 * Checks if a first match is marked as deleted. Single FSA case.
 */
template <class MatcherT, class DeletedT>
inline keyvi::dictionary::Match FirstFilteredMatchSingle(const MatcherT& matcher, const DeletedT& deleted_keys) {
  dictionary::Match first_match = matcher->FirstMatch();
  if (first_match.IsEmpty() == false) {
    if (deleted_keys->count(first_match.GetMatchedString()) > 0) {
      return dictionary::Match();
    }
  }
  return first_match;
}

/**
 * Filters matches and drops results for deleted keys for a multiple FSA's.
 */
template <class MatcherT, class DeletedT>
inline keyvi::dictionary::Match NextFilteredMatch(const MatcherT& matcher, const DeletedT& deleted_keys_map) {
  keyvi::dictionary::Match m = matcher->NextMatch();

  TRACE("check if key is deleted");
  while (m.IsEmpty() == false) {
    auto dk = deleted_keys_map.find(m.GetFsa());
    if (dk != deleted_keys_map.end()) {
      if (dk->second->count(m.GetMatchedString()) > 0) {
        matcher->ResetLastMatch();
        m = matcher->NextMatch();
        continue;
      }
    }
    break;
  }
  return m;
}

/**
 * Checks if a first match is marked as deleted. Multi FSA case.
 */
template <class MatcherT, class DeletedT>
inline keyvi::dictionary::Match FirstFilteredMatch(const MatcherT& matcher, const DeletedT& deleted_keys_map) {
  dictionary::Match first_match = matcher->FirstMatch();
  if (first_match.IsEmpty() == false) {
    auto dk = deleted_keys_map.find(first_match.GetFsa());
    if (dk != deleted_keys_map.end()) {
      if (dk->second->count(first_match.GetMatchedString()) > 0) {
        return dictionary::Match();
      }
    }
  }
  return first_match;
}

template <class SegmentT, class StatePairT>
inline std::map<dictionary::fsa::automata_t, typename SegmentT::deleted_ptr_t> CreatedDeletedKeysMap(
    const std::shared_ptr<std::vector<std::shared_ptr<SegmentT>>>& segments,
    const std::vector<StatePairT>& fsa_start_state_pairs) {
  std::map<dictionary::fsa::automata_t, typename SegmentT::deleted_ptr_t> deleted_keys_map;

  auto segments_it = segments->cbegin();
  for (const auto& fsa : fsa_start_state_pairs) {
    while (std::get<0>(fsa) != (*segments_it)->GetDictionary()->GetFsa()) {
      ++segments_it;
      // this should never happen
      if (segments_it == segments->end()) {
        throw std::runtime_error("order of segments do not match expected order");
      }
    }
    TRACE("found corresponding sement");
    if ((*segments_it)->DeletedKeysSize() > 0) {
      deleted_keys_map.emplace(std::get<0>(fsa), (*segments_it)->DeletedKeys());
    }
    ++segments_it;
  }

  return deleted_keys_map;
}

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_LOOKUP_UTIL_H_
