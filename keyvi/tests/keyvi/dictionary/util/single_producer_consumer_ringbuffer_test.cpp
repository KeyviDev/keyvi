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


#include <boost/test/unit_test.hpp>

#include "dictionary/util/single_producer_consumer_ringbuffer.h"

namespace keyvi {
namespace dictionary {
namespace util {

BOOST_AUTO_TEST_SUITE( SingleProducerConsumerRingBufferTests )

struct Item {
  Item(){}
  Item(const std::string& key): key(key) {}
  std::string key;
};


BOOST_AUTO_TEST_CASE( SimpleTest ) {
  SingeProducerSingleConsumerRingBuffer<Item*, 10> s;

  Item* a = new Item("test");
  Item* b = new Item("test2");
  Item* c = new Item("test3");

  BOOST_CHECK_EQUAL("test", a->key);

  BOOST_CHECK(s.Push(a));
  BOOST_CHECK(s.Push(b));
  BOOST_CHECK(s.Push(c));

  Item *d;
  BOOST_CHECK(s.Pop(d));
  BOOST_CHECK_EQUAL(a, d);
  BOOST_CHECK_EQUAL("test", d->key);
  delete(d);

  BOOST_CHECK(s.Pop(d));
  BOOST_CHECK_EQUAL(b, d);
  BOOST_CHECK_EQUAL("test2", d->key);
  delete(d);

  BOOST_CHECK(s.Pop(d));
  BOOST_CHECK_EQUAL(c, d);
  BOOST_CHECK_EQUAL("test3", d->key);
  delete(d);

  BOOST_CHECK(!s.Pop(d));
}


BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */

