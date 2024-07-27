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
 * match_iterator.h
 *
 *  Created on: Jun 3, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_MATCH_ITERATOR_H_
#define KEYVI_DICTIONARY_MATCH_ITERATOR_H_

#include <utility>

#include <boost/iterator/iterator_facade.hpp>

#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/util/iterator_utils.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

/**
 * An iterator class for matches, which makes the usage more convinient.
 * The Lookup code simply has to implement a functor/delegate/lambda function which the iterator calls for every step
 *
 * Remarks:
 *
 *  Most clients use C++11 lambda functions
 *
 *  There might be faster solutions, using 'fast delegates':
 *
 *  https://gist.github.com/SuperV1234/6462221
 *  http://www.codeproject.com/Articles/384572/Implementation-of-Delegates-in-Cplusplus11
 *  http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
 *  http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11
 */
class MatchIterator : public boost::iterator_facade<MatchIterator, match_t const, boost::single_pass_traversal_tag> {
 public:
  using MatchIteratorPair = util::iterator_pair<MatchIterator>;

  explicit MatchIterator(std::function<match_t()> match_functor, match_t&& first_match = match_t(),
                         std::function<void(uint32_t)> set_min_weight = {})
      : match_functor_(std::move(match_functor)),
        current_match_(std::move(first_match)),
        set_min_weight_(std::move(set_min_weight)) {
    if (!current_match_) {
      TRACE("first match empty");
      increment();
    }
  }

  MatchIterator() : match_functor_(0), set_min_weight_({}) {}

  static MatchIteratorPair MakeIteratorPair(std::function<match_t()> f, match_t&& first_match = match_t(),
                                            std::function<void(uint32_t)> set_min_weight = {}) {
    return MatchIteratorPair(MatchIterator(std::move(f), std::move(first_match), std::move(set_min_weight)),
                             MatchIterator());
  }

  static MatchIteratorPair EmptyIteratorPair() { return MatchIteratorPair(MatchIterator(), MatchIterator()); }

  void SetMinWeight(uint32_t min_weight) {
    // ignore if a min weight setter was not provided
    if (set_min_weight_) {
      set_min_weight_(min_weight);
    }
  }

  // What we implement is determined by the boost::forward_traversal_tag
  // template parameter
 private:
  friend class boost::iterator_core_access;

  void increment() {
    if (match_functor_) {
      TRACE("Match Iterator: call increment()");

      current_match_ = match_functor_();

      // if we get an empty match, release the functor
      if (!current_match_) {
        TRACE("Match Iterator: no more match found reset functor");
        match_functor_ = 0;
        set_min_weight_ = {};
      }
    }
  }

  bool equal(MatchIterator const& other) const {
    // usual case for comparing with end()
    if (!this->current_match_ && !other.current_match_) {
      TRACE("Match Iterator: equal true");
      return true;
    }

    TRACE("Match Iterator: equal false");
    return false;  // return this->current_match_ == other.current_match_;
  }

  match_t const& dereference() const { return current_match_; }

 private:
  std::function<match_t()> match_functor_;
  match_t current_match_;
  std::function<void(uint32_t)> set_min_weight_;
};

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_MATCH_ITERATOR_H_
