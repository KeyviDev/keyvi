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
 * bounded_priority_queue.h
 *
 *  Created on: Jul 10, 2014
 *      Author: hendrik
 */

#ifndef BOUNDED_PRIORITY_QUEUE_H_
#define BOUNDED_PRIORITY_QUEUE_H_

#include <queue>
#include <stdexcept>
#include <algorithm>

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace util {

/**
 * Bounded priority queue: a fixed size priority queue to hold the n best items.
 */

template<typename T>
struct BoundedPriorityQueue {
  explicit BoundedPriorityQueue(size_t size)
      : elements_(new T[size]),
        size_(size) {
    std::fill(elements_, elements_ + size_, 0);

    std::make_heap(elements_, elements_ + size, std::greater<T>());
  }

  BoundedPriorityQueue() = delete;
  BoundedPriorityQueue& operator=(BoundedPriorityQueue<T> const&) = delete;
  BoundedPriorityQueue(const BoundedPriorityQueue<T>& that) = delete;

  BoundedPriorityQueue(BoundedPriorityQueue&& other)
      : elements_(other.elements_),
        size_(other.size_) {
    other.elements_ = 0;
    other.size_ = 0;
  }

  ~BoundedPriorityQueue() {
    if (elements_) {
      delete[] elements_;
    }
  }

  T Back() const {
    return elements_[0];
  }

  void ReduceSize() {
    std::pop_heap(elements_, elements_ + size_, std::greater<T>());
    size_--;
  }

  void Put(T value) {
    std::pop_heap(elements_, elements_ + size_, std::greater<T>());
    TRACE("Replace %d %d", elements_[size_ - 1], value);

    elements_[size_ - 1] = value;
    std::push_heap(elements_, elements_ + size_, std::greater<T>());
  }
 private:
  T * elements_;
  size_t size_;
};

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* BOUNDED_PRIORITY_QUEUE_H_ */
