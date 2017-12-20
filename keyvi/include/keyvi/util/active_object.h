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
 * active_object.h
 *
 *  Created on: Dec 14, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_UTIL_ACTIVE_OBJECT_H_
#define KEYVI_UTIL_ACTIVE_OBJECT_H_

#include <chrono>  // NOLINT
#include <functional>
#include <thread>  // NOLINT

#include "util/single_producer_consumer_ringbuffer.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace util {

template <typename T, size_t Tsize = 100>
class ActiveObject final {
 public:
  explicit ActiveObject(T* resource, const std::function<void()>& scheduled_task,
                        const std::chrono::duration<double>& flush_interval = std::chrono::milliseconds(1000))
      : resource_(resource), scheduled_task_(scheduled_task), queue_(flush_interval), done_(false) {
    worker_ = std::thread([this] {
      std::function<void()> item;
      while (!done_) {
        if (queue_.Pop(&item)) {
          item();
        }

        // todo: run only if flush interval has passed
        scheduled_task_();
      }
    });
  }

  ~ActiveObject() {
    queue_.Push([this] {
      done_ = true;
    });

    worker_.join();
  }

  template <typename F>
  void operator()(F f) {
    queue_.Push([=] { f(*resource_); });
  }

 private:
  mutable SingeProducerSingleConsumerRingBuffer<std::function<void()>, Tsize> queue_;

  T* resource_;

  std::function<void()> scheduled_task_;

  std::thread worker_;

  bool done_;
};

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_ACTIVE_OBJECT_H_
