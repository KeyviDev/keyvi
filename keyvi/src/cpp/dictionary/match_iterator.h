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

#ifndef MATCH_ITERATOR_H_
#define MATCH_ITERATOR_H_

#include <boost/iterator/iterator_facade.hpp>

#include "dictionary/match.h"
#include "dictionary/util/iterator_utils.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

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
class MatchIterator : public boost::iterator_facade<MatchIterator  // CRTP, just use the Iterator name
    , Match const  // Value type of what is iterated over (contained element type)
    , boost::single_pass_traversal_tag  // type of traversal allowed
    >// Reference and Difference can be omitted
{
 public:
  typedef util::iterator_pair<MatchIterator> MatchIteratorPair;

  MatchIterator(std::function<Match()> match_functor, const Match& first_match = Match())
      : match_functor_(match_functor) {
    current_match_ = first_match;
    if (first_match.IsEmpty()){
      increment();
    }
  }

  static MatchIteratorPair MakeIteratorPair(std::function<Match()> f, const Match& first_match = Match()) {
    return MatchIteratorPair(MatchIterator(f, first_match), MatchIterator());
  }

  static MatchIteratorPair EmptyIteratorPair() {
      return MatchIteratorPair(MatchIterator(), MatchIterator());
  }

  MatchIterator()
      : match_functor_(0) {
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
      if (current_match_.IsEmpty()) {
        TRACE("Match Iterator: no more match found reset functor");
        match_functor_ = 0;
      }
    }
  }

  bool equal(MatchIterator const& other) const {

    // usual case for comparing with end()
    if (this->current_match_.IsEmpty() && other.current_match_.IsEmpty()) {

      TRACE("Match Iterator: equal true");
      return true;
    }

    TRACE("Match Iterator: equal false");
    return false;  //return this->current_match_ == other.current_match_;
  }

  Match const & dereference() const {
    return current_match_;
  }

 private:
  std::function<Match()> match_functor_;
  Match current_match_;
};


} /* namespace dictionary */
} /* namespace keyvi */

#endif /* MATCH_ITERATOR_H_ */
