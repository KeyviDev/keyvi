/* * keyvi - A key value store.
 *
 * Copyright 2020 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * segment_iterator.h
 *
 *  Created on: Nov 19, 2020
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_SEGMENT_ITERATOR_H_
#define KEYVI_DICTIONARY_FSA_SEGMENT_ITERATOR_H_

#include <memory>

#include "keyvi/dictionary/fsa/entry_iterator.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

class SegmentIterator final {
  using EntryIteratorPtr = std::shared_ptr<fsa::EntryIterator>;

 public:
  /**
   * SegmentIterator
   *
   * wrapper around an entry iterator that stores a priority acting as tie breaker when merging
   */
  SegmentIterator(const fsa::EntryIterator& e, size_t segment_index)
      : entry_iterator_ptr_(std::make_shared<fsa::EntryIterator>(e)), segment_index_(segment_index) {}

  bool operator<(const SegmentIterator& rhs) const {
    // very important difference in semantics: we have to ensure that in case
    // of equal key,
    // the iterator with the higher index (priority) is taken

    if (segment_index_ < rhs.segment_index_) {
      return entryIterator() > rhs.entryIterator();
    }

    return rhs.entryIterator() < entryIterator();
  }

  operator bool() const { return entryIterator() != endIterator(); }

  SegmentIterator& operator++() {
    ++(*entry_iterator_ptr_);
    return *this;
  }

  const fsa::EntryIterator& entryIterator() const { return *entry_iterator_ptr_; }

  const size_t segmentIndex() const { return segment_index_; }

 private:
  static const fsa::EntryIterator& endIterator() {
    static fsa::EntryIterator end_it;
    return end_it;
  }

 private:
  EntryIteratorPtr entry_iterator_ptr_;
  size_t segment_index_;
};

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_SEGMENT_ITERATOR_H_
