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

#include <atomic>
#include <chrono>              //NOLINT
#include <condition_variable>  //NOLINT
#include <mutex>               //NOLINT
#include <thread>              //NOLINT
#include <utility>

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace util {

/**
 * Simple lock-free single producer, single consumer queue
 */
template <typename T, size_t Tsize>
class SingeProducerSingleConsumerRingBuffer {
 public:
  SingeProducerSingleConsumerRingBuffer()
      : head_(0),
        tail_(0),
        mutex_(),
        producer_condition_(),
        consumer_condition_(),
        producer_stopped_(false),
        consumer_stopped_(false) {}

  void Push(T&& value) {
    size_t head = head_.load(std::memory_order_relaxed);
    size_t tail = tail_.load(std::memory_order_acquire);
    size_t next_head = next(head);
    if (next_head == tail) {
      // wait for bucket
      std::unique_lock<std::mutex> lock(mutex_);

      // check again
      tail = tail_.load(std::memory_order_acquire);

      while (next_head == tail) {
        TRACE("queue full, wait.");
        if (consumer_stopped_) {
          consumer_condition_.notify_all();
        }
        producer_stopped_ = true;
        producer_condition_.wait(lock);
        TRACE("woke up, producer");
        tail = tail_.load(std::memory_order_acquire);
      }

      producer_stopped_ = false;
    }
    ring_[head] = std::move(value);
    head_.store(next_head, std::memory_order_release);

    if (consumer_stopped_) {
      consumer_condition_.notify_all();
    }
  }

  bool Pop(T* value) {
    size_t tail = tail_.load(std::memory_order_relaxed);
    size_t head = head_.load(std::memory_order_acquire);

    if (tail == head) {
      // wait for bucket
      std::unique_lock<std::mutex> lock(mutex_);

      head = head_.load(std::memory_order_acquire);

      while (tail == head) {
        TRACE("queue empty");

        if (producer_stopped_) {
          producer_condition_.notify_all();
        }

        consumer_stopped_ = true;
        consumer_condition_.wait(lock);

        head = head_.load(std::memory_order_acquire);
      }
      consumer_stopped_ = false;
    }
    *value = std::move(ring_[tail]);
    tail_.store(next(tail), std::memory_order_release);

    if (producer_stopped_) {
      producer_condition_.notify_all();
    }

    return true;
  }

  bool Pop(T* value, const std::chrono::time_point<std::chrono::system_clock>& timeout_time) {
    size_t tail = tail_.load(std::memory_order_relaxed);
    size_t head = head_.load(std::memory_order_acquire);
    size_t next_tail = next(tail);

    if (tail == head) {
      // wait for bucket
      std::unique_lock<std::mutex> lock(mutex_);

      head = head_.load(std::memory_order_acquire);

      while (tail == head) {
        TRACE("queue empty");
        if (producer_stopped_) {
          producer_condition_.notify_all();
        }

        consumer_stopped_ = true;

        if (consumer_condition_.wait_until(lock, timeout_time) == std::cv_status::timeout) {
          // woken up by timeout
          consumer_stopped_ = false;
          return false;
        }

        head = head_.load(std::memory_order_acquire);
      }

      consumer_stopped_ = false;
    }
    *value = std::move(ring_[tail]);
    tail_.store(next_tail, std::memory_order_release);

    if (producer_stopped_) {
      producer_condition_.notify_all();
    }

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
  std::atomic_size_t head_, tail_;
  std::mutex mutex_;
  std::condition_variable producer_condition_;
  std::condition_variable consumer_condition_;
  std::atomic_bool producer_stopped_;
  std::atomic_bool consumer_stopped_;
};

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_SINGLE_PRODUCER_CONSUMER_RINGBUFFER_H_
