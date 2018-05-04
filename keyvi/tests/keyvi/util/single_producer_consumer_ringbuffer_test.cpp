//
// keyvi - A key value store.
//
// Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/*
 * single_producer_consumer_ringbuffer_test.cpp
 *
 *  Created on: Mar 6, 2015
 *      Author: hendrik
 */

#include <chrono>  //NOLINT
#include <string>
#include <thread>  //NOLINT

#include <boost/test/unit_test.hpp>

#include "util/single_producer_consumer_ringbuffer.h"

namespace keyvi {
namespace util {

BOOST_AUTO_TEST_SUITE(SingleProducerConsumerRingBufferTests)

struct Item {
  Item() {}
  explicit Item(const std::string& key) : key(key) {}
  std::string key;
};

BOOST_AUTO_TEST_CASE(SimpleTest) {
  SingeProducerSingleConsumerRingBuffer<Item, 10> s;

  Item a("test");
  Item b("test2");
  Item c("test3");
  auto now = std::chrono::system_clock::now();

  BOOST_CHECK_EQUAL("test", a.key);
  BOOST_CHECK_EQUAL(0, s.Size());
  s.Push(std::move(a));
  s.Push(std::move(b));
  s.Push(std::move(c));
  BOOST_CHECK_EQUAL(3, s.Size());
  Item d;
  s.Pop(&d);
  BOOST_CHECK_EQUAL(2, s.Size());
  BOOST_CHECK_EQUAL("test", d.key);

  s.Pop(&d, now);
  BOOST_CHECK_EQUAL(1, s.Size());
  BOOST_CHECK_EQUAL("test2", d.key);

  s.Pop(&d);
  BOOST_CHECK_EQUAL(0, s.Size());
  BOOST_CHECK_EQUAL("test3", d.key);
}

BOOST_AUTO_TEST_CASE(SizeTest) {
  SingeProducerSingleConsumerRingBuffer<Item, 11> s;

  for (size_t i = 0; i < 10; ++i) {
    Item v(std::to_string(i));
    s.Push(std::move(v));
  }
  BOOST_CHECK_EQUAL(10, s.Size());
  Item v;
  s.Pop(&v);
  s.Pop(&v);
  BOOST_CHECK_EQUAL(8, s.Size());
  Item w(std::to_string(10));
  s.Push(std::move(w));
  BOOST_CHECK_EQUAL(9, s.Size());

  Item x(std::to_string(11));
  s.Push(std::move(x));
  BOOST_CHECK_EQUAL(10, s.Size());

  s.Pop(&v);
  s.Pop(&v);
  BOOST_CHECK_EQUAL(8, s.Size());
}

using TestQueue = SingeProducerSingleConsumerRingBuffer<Item, 100>;

void consumer(TestQueue* queue) {
  std::this_thread::sleep_for(std::chrono::microseconds(5));
  Item v;
  for (size_t i = 0; i < 1000; ++i) {
    queue->Pop(&v);
    BOOST_CHECK_EQUAL(v.key, std::to_string(i));
  }

  auto now = std::chrono::system_clock::now();
  for (size_t i = 0; i < 10; ++i) {
    BOOST_CHECK(!queue->Pop(&v, now + std::chrono::milliseconds(1)));
  }
  BOOST_CHECK(queue->Pop(&v));
  BOOST_CHECK_EQUAL(v.key, "x");
  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  now = std::chrono::system_clock::now();
  for (size_t i = 0; i < 1000; ++i) {
    BOOST_CHECK(queue->Pop(&v, now + std::chrono::milliseconds(4)));
    BOOST_CHECK_EQUAL(v.key, std::to_string(i));
  }
}

BOOST_AUTO_TEST_CASE(ConsumerProducerTest) {
  TestQueue queue;

  std::thread consumer_thread(consumer, &queue);

  for (size_t i = 0; i < 300; ++i) {
    Item w(std::to_string(i));
    queue.Push(std::move(w));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(12));
  for (size_t i = 300; i < 1000; ++i) {
    Item w(std::to_string(i));
    queue.Push(std::move(w));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  Item x("x");
  queue.Push(std::move(x));

  for (size_t i = 0; i < 300; ++i) {
    Item w(std::to_string(i));
    queue.Push(std::move(w));
  }

  std::this_thread::sleep_for(std::chrono::microseconds(12));
  for (size_t i = 300; i < 1000; ++i) {
    Item w(std::to_string(i));
    queue.Push(std::move(w));
  }

  consumer_thread.join();
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
