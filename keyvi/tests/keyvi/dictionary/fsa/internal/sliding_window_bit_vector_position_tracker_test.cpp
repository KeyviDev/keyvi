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
 * sliding_window_bit_vector_position_tracker_test.cpp
 *
 *  Created on: May 14, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/bit_vector.h"
#include "keyvi/dictionary/fsa/internal/sliding_window_bit_vector_position_tracker.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(SlidingWindowBitVectorPositionTrackerTests)

BOOST_AUTO_TEST_CASE(nextfreeslot) {
  SlidingWindowBitArrayPositionTracker positions;

  positions.Set(8);
  positions.Set(9);
  positions.Set(10);
  BOOST_CHECK_EQUAL(11, positions.NextFreeSlot(8));
  BOOST_CHECK_EQUAL(12, positions.NextFreeSlot(12));
}

BOOST_AUTO_TEST_CASE(sliding) {
  SlidingWindowBitArrayPositionTracker positions;

  positions.Set(8);
  positions.Set(9);
  positions.Set(10);

  // trigger switch of the bit vectors
  positions.Set(1050);

  BOOST_CHECK(positions.IsSet(8));
  BOOST_CHECK(positions.IsSet(9));
  BOOST_CHECK(positions.IsSet(10));
  BOOST_CHECK(!positions.IsSet(1024 + 8));
  BOOST_CHECK(!positions.IsSet(1024 + 9));
  BOOST_CHECK(!positions.IsSet(1024 + 10));

  // trigger switch of the bit vectors and cause slide
  positions.Set(2100);
  BOOST_CHECK(!positions.IsSet(1024 + 8));
  BOOST_CHECK(!positions.IsSet(1024 + 9));
  BOOST_CHECK(!positions.IsSet(1024 + 10));

  BOOST_CHECK(!positions.IsSet(2048 + 8));
  BOOST_CHECK(!positions.IsSet(2048 + 9));
  BOOST_CHECK(!positions.IsSet(2048 + 10));

  positions.Set(4095);
}

BOOST_AUTO_TEST_CASE(no_integer_overflow) {
  SlidingWindowBitArrayPositionTracker positions;

  positions.Set(8);
  positions.Set(9);
  positions.Set(10);

  // trigger switch of the bit vectors
  uint32_t t = uint32_t(INT_MAX) + 10;
  positions.Set(t);

  BOOST_CHECK(positions.IsSet(t));
}

BOOST_AUTO_TEST_CASE(set_vector) {
  SlidingWindowBitArrayPositionTracker positions;

  positions.Set(8);
  positions.Set(9);
  positions.Set(10);

  BitVector<32> a;
  a.Set(1);
  a.Set(3);
  a.Set(25);
  a.Set(29);
  a.Set(31);

  positions.SetVector(a, 2);
  BOOST_CHECK(positions.IsSet(3));
  BOOST_CHECK(positions.IsSet(5));
  BOOST_CHECK(positions.IsSet(27));
}

BOOST_AUTO_TEST_CASE(set_vector_overlap) {
  SlidingWindowBitArrayPositionTracker positions;

  positions.Set(8);
  positions.Set(9);
  positions.Set(10);

  BitVector<32> a;
  a.Set(1);
  a.Set(3);
  a.Set(25);
  a.Set(29);
  a.Set(31);

  positions.SetVector(a, 1020);
  BOOST_CHECK(positions.IsSet(1021));
  BOOST_CHECK(positions.IsSet(1023));
  BOOST_CHECK(positions.IsSet(1045));
  BOOST_CHECK(positions.IsSet(1049));
  BOOST_CHECK(positions.IsSet(1051));
}

BOOST_AUTO_TEST_CASE(set_vector_overlap2) {
  SlidingWindowBitArrayPositionTracker positions;

  positions.Set(8);
  positions.Set(9);
  positions.Set(10);

  BitVector<260> a;
  a.Set(119);
  a.Set(151);
  positions.SetVector(a, 929);
  BOOST_CHECK(positions.IsSet(929 + 119));

  BOOST_CHECK(positions.IsSet(929 + 151));

  positions.SetVector(a, 940);
  BOOST_CHECK(positions.IsSet(940 + 119));

  BitVector<260> b;
  b.Set(100);
  positions.SetVector(b, 924);
  BOOST_CHECK(positions.IsSet(924 + 100));
}

BOOST_AUTO_TEST_CASE(set_vector_overlap3) {
  SlidingWindowBitArrayPositionTracker positions;

  positions.Set(8);
  positions.Set(9);
  positions.Set(10);

  BitVector<260> a;
  a.Set(256);
  a.Set(257);
  positions.SetVector(a, 5311);
  BOOST_CHECK(positions.IsSet(5311 + 256));

  BOOST_CHECK(positions.IsSet(5311 + 257));

  positions.Set(5311 + 257);

  BOOST_CHECK(positions.IsSet(5311 + 257));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
