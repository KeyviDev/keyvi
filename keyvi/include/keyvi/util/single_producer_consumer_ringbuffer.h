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
 * single_producer_consumer_ringbuffer.h
 *
 *  Created on: Feb 4, 2015
 *      Author: hendrik
 */

#ifndef KEYVI_UTIL_SINGLE_PRODUCER_CONSUMER_RINGBUFFER_H_
#define KEYVI_UTIL_SINGLE_PRODUCER_CONSUMER_RINGBUFFER_H_

#include <chrono>  //NOLINT
#include <thread>  //NOLINT

#include <boost/atomic.hpp>

namespace keyvi {
namespace util {

#define SPINLOCK_WAIT_IN_MS 1

/**
 * Simple lock-free single producer, single consumer queue
 */
template <typename T, size_t Tsize>
class SingeProducerSingleConsumerRingBuffer {
 public:
  explicit SingeProducerSingleConsumerRingBuffer(
      const std::chrono::milliseconds& return_interval = std::chrono::milliseconds(1000))
      : head_(0), tail_(0), return_interval_(return_interval) {
    next_pop_ = std::chrono::system_clock::now() + return_interval_;
  }

  void Push(const T& value) {
    size_t head = head_.load(boost::memory_order_relaxed);
    size_t next_head = next(head);
    if (next_head == tail_.load(boost::memory_order_acquire)) {
      // spin-lock
      while (next_head == tail_.load(boost::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SPINLOCK_WAIT_IN_MS));
      }
    }
    ring_[head] = std::move(value);
    head_.store(next_head, boost::memory_order_release);
    return;
  }

  bool Pop(T* value) {
    size_t tail = tail_.load(boost::memory_order_relaxed);
    while (tail == head_.load(boost::memory_order_acquire)) {
      auto tp = std::chrono::system_clock::now();
      if (tp > next_pop_) {
        next_pop_ = tp + return_interval_;
        return false;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(SPINLOCK_WAIT_IN_MS));
    }
    *value = std::move(ring_[tail]);
    tail_.store(next(tail), boost::memory_order_release);
    next_pop_ = std::chrono::system_clock::now() + return_interval_;
    return true;
  }

  size_t Size() const {
    if (head_ >= tail_) {
      return head_ - tail_;
    }

    return Tsize - tail_ + head_;
  }

 private:
  size_t next(size_t current) { return (current + 1) % Tsize; }
  T ring_[Tsize];
  boost::atomic<size_t> head_, tail_;
  std::chrono::time_point<std::chrono::system_clock> next_pop_;
  std::chrono::milliseconds return_interval_;
};

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_SINGLE_PRODUCER_CONSUMER_RINGBUFFER_H_
