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
 * bounded_priority_queue_test.cpp
 *
 *  Created on: Jul 11, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/util/bounded_priority_queue.h"

namespace keyvi {
namespace dictionary {
namespace util {

BOOST_AUTO_TEST_SUITE(BoundedPriorityQueueTests)

BOOST_AUTO_TEST_CASE(simpleQueueTest) {
  BoundedPriorityQueue<int> p(10);
  BOOST_CHECK_EQUAL(0, p.Back());
  p.Put(1);
  p.Put(2);
  p.Put(3);
  p.Put(4);
  BOOST_CHECK_EQUAL(0, p.Back());
  p.Put(5);
  p.Put(6);
  p.Put(7);
  p.Put(8);
  p.Put(9);
  BOOST_CHECK_EQUAL(0, p.Back());
  p.Put(10);
  p.Put(11);
  // BOOST_CHECK_EQUAL(11, p.Front());
  BOOST_CHECK_EQUAL(2, p.Back());
  p.Put(12);
  BOOST_CHECK_EQUAL(3, p.Back());
  p.Put(13);
  BOOST_CHECK_EQUAL(4, p.Back());
  p.Put(6);
  BOOST_CHECK_EQUAL(5, p.Back());
  p.Put(7);
  BOOST_CHECK_EQUAL(6, p.Back());
  p.Put(12);
  BOOST_CHECK_EQUAL(6, p.Back());
  p.ReduceSize();
  BOOST_CHECK_EQUAL(7, p.Back());
  p.ReduceSize();
  BOOST_CHECK_EQUAL(7, p.Back());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */
