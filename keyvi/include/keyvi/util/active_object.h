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

#include <algorithm>
#include <chrono>  // NOLINT
#include <functional>
#include <thread>  // NOLINT

#include "blockingconcurrentqueue.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace util {

template <typename T, size_t Tsize = 100>
class ActiveObject final {
 public:
  explicit ActiveObject(T* resource, const std::function<void()>& scheduled_task,
                        const std::chrono::milliseconds& flush_interval = std::chrono::milliseconds(1000))
      : queue_(Tsize),
        resource_(resource),
        flush_interval_(flush_interval),
        scheduled_task_(scheduled_task),
        scheduled_task_next_run_(std::chrono::system_clock::now() + flush_interval_),
        done_(false) {
    worker_ = std::thread([this] {
      std::function<void()> item;
      while (!done_) {
        if (queue_.wait_dequeue_timed(item, flush_interval_)) {
          item();
        }

        // run only if flush interval has passed
        auto tp = std::chrono::system_clock::now();
        if (tp > scheduled_task_next_run_) {
          scheduled_task_next_run_ = tp + flush_interval_;
          TRACE("run scheduled task");
          scheduled_task_();
        }
      }

      // run scheduled task a last time before finishing
      scheduled_task_();
    });
  }

  ~ActiveObject() {
    queue_.enqueue([this] { done_ = true; });
    worker_.join();
  }

  template <typename F>
  void operator()(F f) {
    queue_.enqueue([=] { f(*resource_); });
  }

  size_t Size() const { return queue_.size_approx(); }

 private:
  moodycamel::BlockingConcurrentQueue<std::function<void()>> queue_;

  T* resource_;

  std::chrono::milliseconds flush_interval_;

  std::function<void()> scheduled_task_;

  std::chrono::time_point<std::chrono::system_clock> scheduled_task_next_run_;

  std::thread worker_;

  bool done_;
};

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_ACTIVE_OBJECT_H_
