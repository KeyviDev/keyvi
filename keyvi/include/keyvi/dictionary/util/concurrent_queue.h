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
 * concurrent_queue.h
 *
 *  Created on: Dec 14, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_UTIL_CONCURRENT_QUEUE_H_
#define KEYVI_DICTIONARY_UTIL_CONCURRENT_QUEUE_H_

#include <boost/circular_buffer.hpp>

#include <condition_variable>  //NOLINT
#include <mutex>               //NOLINT

namespace keyvi {
namespace dictionary {
namespace util {

/**
 * A concurrent queue
 */
template <typename T, size_t Tsize>
class ConcurrentQueue final {
 public:
  ConcurrentQueue(void) : queue_(Tsize) {}

  ConcurrentQueue() = delete;
  ConcurrentQueue& operator=(ConcurrentQueue<T, Tsize> const&) = delete;
  ConcurrentQueue(const ConcurrentQueue<T, Tsize>& that) = delete;

  /**
   * Pop an item out of the queue, might block until an item is available
   */
  T Pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      consumer_condition_.wait(lock);
    }
    size_t old_size = queue_.size();
    T val = queue_.front();
    queue_.pop_front();

    if (old_size >= Tcapacity) {
      lock.unlock();
      producer_condition_.notify_all();
    }
    return val;
  }

  /**
   * Pop an item out of the queue, might block until an item is available
   * @param item the item
   */
  void Pop(const T* value) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      consumer_condition_.wait(lock);
    }

    size_t old_size = queue_.size();
    *value = queue_.front();
    queue_.pop_front();

    if (old_size >= Tsize) {
      lock.unlock();
      producer_condition_.notify_all();
    }
  }

  /**
   * Pop an item out of the queue, might block if the queue is full
   */
  void Push(const T& value) {
    std::unique_lock<std::mutex> lock(mutex_);
    size_t pending = queue_.size();

    while (pending >= Tsize) {
      producer_condition_.wait(lock);
      pending = queue_.size();
    }

    queue_.push_back(value);

    lock.unlock();
    if (pending == 0) {
      consumer_condition_.notify_all();
    }
  }

  size_t Size() const { return queue_.size(); }

 private:
  boost::circular_buffer<T> queue_;

  std::mutex mutex_;

  std::condition_variable consumer_condition_;

  std::condition_variable producer_condition_;
};

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  //  KEYVI_DICTIONARY_UTIL_CONCURRENT_QUEUE_H_
