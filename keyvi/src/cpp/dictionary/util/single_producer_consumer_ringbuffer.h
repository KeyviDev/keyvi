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

#ifndef SINGLE_PRODUCER_CONSUMER_RINGBUFFER_H_
#define SINGLE_PRODUCER_CONSUMER_RINGBUFFER_H_

#include <boost/atomic.hpp>
#include <chrono>
#include <thread>

namespace keyvi {
namespace dictionary {
namespace util {

#define SPINLOCK_WAIT_IN_MS 1

/**
 * Simple lock-free single producer, single consumer queue
 */
template<typename T, size_t Size>
class SingeProducerSingleConsumerRingBuffer {
public:
  SingeProducerSingleConsumerRingBuffer() : head_(0), tail_(0), done_(false) {}

  bool Push(T & value, bool block = false)
  {
    size_t head = head_.load(boost::memory_order_relaxed);
    size_t next_head = next(head);
    if (next_head == tail_.load(boost::memory_order_acquire))
    {
      if (!block) {
        return false;
      }
      // spin-lock
      while (next_head == tail_.load(boost::memory_order_acquire))
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(SPINLOCK_WAIT_IN_MS));
      }
    }
    ring_[head] = std::move(value);
    head_.store(next_head, boost::memory_order_release);
    return true;
  }

  bool Pop(T & value, bool block = false)
  {
    size_t tail = tail_.load(boost::memory_order_relaxed);
    if (tail == head_.load(boost::memory_order_acquire))
    {
      if (!block) {
        return false;
      }
      while (tail == head_.load(boost::memory_order_acquire))
      {
        // exit if work is done
        if (done_) {
          return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPINLOCK_WAIT_IN_MS));
      }
    }
    value = std::move(ring_[tail]);
    tail_.store(next(tail), boost::memory_order_release);
    return true;
  }

  void SetDone(){
    done_ = true;
  }

  bool IsDone(){
    return done_;
  }

private:
  size_t next(size_t current)
  {
    return (current + 1) % Size;
  }
  T ring_[Size];
  boost::atomic<size_t> head_, tail_;
  boost::atomic<bool> done_;
};

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* SINGLE_PRODUCER_CONSUMER_RINGBUFFER_H_ */
