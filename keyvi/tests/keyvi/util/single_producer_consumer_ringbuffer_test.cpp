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

#include <string>

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
  SingeProducerSingleConsumerRingBuffer<Item*, 10> s;

  Item* a = new Item("test");
  Item* b = new Item("test2");
  Item* c = new Item("test3");

  BOOST_CHECK_EQUAL("test", a->key);
  BOOST_CHECK_EQUAL(0, s.Size());
  s.Push(a);
  s.Push(b);
  s.Push(c);
  BOOST_CHECK_EQUAL(3, s.Size());
  Item* d;
  s.Pop(&d);
  BOOST_CHECK_EQUAL(a, d);
  BOOST_CHECK_EQUAL(2, s.Size());
  BOOST_CHECK_EQUAL("test", d->key);
  delete (d);

  s.Pop(&d);
  BOOST_CHECK_EQUAL(1, s.Size());
  BOOST_CHECK_EQUAL(b, d);
  BOOST_CHECK_EQUAL("test2", d->key);
  delete (d);

  s.Pop(&d);
  BOOST_CHECK_EQUAL(0, s.Size());
  BOOST_CHECK_EQUAL(c, d);
  BOOST_CHECK_EQUAL("test3", d->key);
  delete (d);
}

BOOST_AUTO_TEST_CASE(SizeTest) {
  SingeProducerSingleConsumerRingBuffer<Item, 11> s;

  for (size_t i = 0; i < 10; ++i) {
    Item v(std::to_string(i));
    s.Push(v);
  }
  BOOST_CHECK_EQUAL(10, s.Size());
  Item v;
  s.Pop(&v);
  s.Pop(&v);
  BOOST_CHECK_EQUAL(8, s.Size());
  Item w(std::to_string(10));
  s.Push(w);
  BOOST_CHECK_EQUAL(9, s.Size());

  Item x(std::to_string(11));
  s.Push(x);
  BOOST_CHECK_EQUAL(10, s.Size());

  s.Pop(&v);
  s.Pop(&v);
  BOOST_CHECK_EQUAL(8, s.Size());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
