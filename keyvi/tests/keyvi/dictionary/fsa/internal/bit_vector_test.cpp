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
 * bit_vector_test.cpp
 *
 *  Created on: May 2, 2014
 *      Author: hendrik
 */

#define BITVECTOR_UNIT_TEST

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/bit_vector.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(BitVectorTests)

BOOST_AUTO_TEST_CASE(size) {
  BitVector<1> a;

  BOOST_CHECK(a.bits_.size() == 1);
  auto number_of_bits = sizeof(a.bits_[0]);
  if (number_of_bits == 32) {
    BitVector<32> b;
    BOOST_CHECK(b.bits_.size() == 2);
  } else {
    BitVector<64> b;
    BOOST_CHECK(b.bits_.size() == 2);
  }
}

BOOST_AUTO_TEST_CASE(reset) {
  BitVector<100> a;

  a.Set(42);
  BOOST_CHECK(!a.Get(41));
  BOOST_CHECK(a.Get(42));
  BOOST_CHECK(!a.Get(43));

  a.Clear();
  BOOST_CHECK(!a.Get(42));
  BOOST_CHECK(a.bits_[1] == 0);
}

BOOST_AUTO_TEST_CASE(getByte) {
  BitVector<16> origin;

  // 0000 0010 1010 0100
  origin.Set(6);
  origin.Set(8);
  origin.Set(13);
  origin.Set(10);

  // 0101 0100
  BitVector<32> a;
  a.Set(6);
  a.Set(8);
  a.Set(13);
  a.Set(10);

  // 0101 0100 1000 0000
  BitVector<32> b;
  b.Set(1);
  b.Set(3);
  b.Set(5);
  b.Set(8);

  // 1010 1000
  BitVector<32> c;
  c.Set(0);
  c.Set(2);
  c.Set(4);
  c.Set(7);

  // 1001 0000
  BitVector<32> d;
  d.Set(0);
  d.Set(3);

  // 0000 0000
  BitVector<15> e;

  BOOST_CHECK_EQUAL(a.bits_[0], origin.GetUnderlyingIntegerAtPosition(0, 0));
  BOOST_CHECK_EQUAL(b.bits_[0], origin.GetUnderlyingIntegerAtPosition(0, 5));
  BOOST_CHECK_EQUAL(c.bits_[0], origin.GetUnderlyingIntegerAtPosition(0, 6));
  BOOST_CHECK_EQUAL(d.bits_[0], origin.GetUnderlyingIntegerAtPosition(0, 10));
  BOOST_CHECK_EQUAL(e.bits_[0], origin.GetUnderlyingIntegerAtPosition(0, 15));
}

BOOST_AUTO_TEST_CASE(disjoint) {
  BitVector<16> origin;

  // 0000 0010 1010 0100
  origin.Set(6);
  origin.Set(8);
  origin.Set(13);
  origin.Set(10);

  // 0000 0100
  BitVector<32> a;
  a.Set(5);

  // 0000 0010
  BitVector<16> b;
  b.Set(6);

  // 0010 0000
  BitVector<16> c;
  c.Set(2);

  // 1001 0000
  BitVector<16> d;
  d.Set(0);
  d.Set(3);

  // 0000 0010
  // 0000 0100
  BOOST_CHECK(origin.Disjoint(a, 0));

  // 0000 0010 1010
  //  000 0010 0
  BOOST_CHECK(!origin.Disjoint(a, 1));

  // 0000 0010 1010
  //   00 0001 00
  BOOST_CHECK(origin.Disjoint(a, 2));

  // 0000 0010 1010 0100
  //           0000 0100
  BOOST_CHECK(!origin.Disjoint(a, 8));

  // 0000 0010 1010 0100
  //             00 0001 00
  BOOST_CHECK(origin.Disjoint(a, 10));

  // 0000 0010 1010 0100
  //                  00 0001 00
  BOOST_CHECK(origin.Disjoint(a, 14));

  // 0000 0010 1010
  // 0000 0010
  BOOST_CHECK(!origin.Disjoint(b, 0));

  // 0000 0010 1010
  //      0010 0000
  BOOST_CHECK(!origin.Disjoint(c, 4));
}

BOOST_AUTO_TEST_CASE(unsignedBitShiftConsistency) {
  // tests that bit shifting works for bit usually used for signed/unsigned (fails if underlying buffer uses a signed
  // type)

  // 0000 0000 0000 0000 0000 0000 0000 0001
  BitVector<32> f;
  f.Set(31);
  BOOST_CHECK(f.Get(31));
  BOOST_CHECK(!f.Disjoint(f, 0));
  f.bits_[0] = f.GetUnderlyingIntegerAtPosition(0, 1);
  BOOST_CHECK(!f.Get(31));
  BOOST_CHECK(f.Get(30));

  BitVector<8> g;
  g.Set(7);
  BOOST_CHECK(g.Get(7));
  BOOST_CHECK(!g.Disjoint(g, 0));
  g.bits_[0] = g.GetUnderlyingIntegerAtPosition(0, 1);
  BOOST_CHECK(!g.Get(7));
  BOOST_CHECK(g.Get(6));
}

BOOST_AUTO_TEST_CASE(nextZeroBit) {
  // 1010 0000
  BitVector<32> a;
  BOOST_CHECK_EQUAL(0, a.GetNextNonSetBit(0));
  a.Set(0);
  a.Set(2);

  BOOST_CHECK_EQUAL(1, a.GetNextNonSetBit(0));
  BOOST_CHECK_EQUAL(1, a.GetNextNonSetBit(1));

  // 1110 0000
  a.Set(1);
  BOOST_CHECK_EQUAL(3, a.GetNextNonSetBit(0));
  BOOST_CHECK_EQUAL(3, a.GetNextNonSetBit(1));
  BOOST_CHECK_EQUAL(3, a.GetNextNonSetBit(2));
  BOOST_CHECK_EQUAL(3, a.GetNextNonSetBit(3));

  // 1110 0000
  a.Set(4);
  BOOST_CHECK_EQUAL(3, a.GetNextNonSetBit(0));
  BOOST_CHECK_EQUAL(3, a.GetNextNonSetBit(1));
  BOOST_CHECK_EQUAL(3, a.GetNextNonSetBit(2));
  BOOST_CHECK_EQUAL(3, a.GetNextNonSetBit(3));
  BOOST_CHECK_EQUAL(5, a.GetNextNonSetBit(4));

  for (int i = 0; i < 32; ++i) {
    a.Set(i);
  }

  BOOST_CHECK_EQUAL(32, a.GetNextNonSetBit(0));

  BitVector<64> b;

  for (int i = 32; i < 39; ++i) {
    b.Set(i);
  }

  for (int i = 40; i < 52; ++i) {
    b.Set(i);
  }

  BOOST_CHECK_EQUAL(0, b.GetNextNonSetBit(0));
  BOOST_CHECK_EQUAL(39, b.GetNextNonSetBit(32));
}

BOOST_AUTO_TEST_CASE(setVector) {
  BitVector<64> origin;

  // 0000 0010 1010 0000 0000 0000 0000 0000 0000 0000 0000 0000
  origin.Set(6);
  origin.Set(8);
  origin.Set(10);

  // 1100 0000
  BitVector<8> a;
  a.Set(0);
  a.Set(1);

  // should result in 0011 0010 1010 0000
  origin.SetVector(a, 2);

  BOOST_CHECK(origin.Get(2));
  BOOST_CHECK(origin.Get(3));

  // 0000 0000 0000 0000 0100 0011
  BitVector<32> b;
  b.Set(30);
  b.Set(31);
  b.Set(25);

  // should result in
  //         0 0000 0000 0000 0000 0000 0000 1000 011
  // 0011 0010 1010 0000 0000 0000 0000 0000 1000 0110 0000
  origin.SetVector(b, 7);

  BOOST_CHECK(origin.Get(32));
  BOOST_CHECK(origin.Get(37));
  BOOST_CHECK(origin.Get(38));

  origin.Clear();
  // 0000 0010 1010 0000 0000 0000 0000 0000 1000 0000 0000 0000
  origin.Set(6);
  origin.Set(8);
  origin.Set(10);
  origin.Set(32);

  // 0110 0000 0000 0000 0100 0011
  BitVector<32> c;
  c.Set(1);
  c.Set(2);
  c.Set(25);
  c.Set(30);
  c.Set(31);

  // should result in
  //                                         0110 0000 0000 0000 0000 0000 1000 011
  // 0000 0010 1010 0000 0000 0000 0000 0000 1110 0000 0000 0000 0000 0000 1000 011
  origin.SetVector(c, 32);

  BOOST_CHECK(!origin.Get(0));
  BOOST_CHECK(!origin.Get(1));
  BOOST_CHECK(origin.Get(32));
  BOOST_CHECK(origin.Get(33));
  BOOST_CHECK(origin.Get(34));
  BOOST_CHECK(origin.Get(57));
  BOOST_CHECK(origin.Get(62));
  BOOST_CHECK(origin.Get(63));

  origin.SetVector(c, 50);
  BOOST_CHECK(origin.Get(52));
}

BOOST_AUTO_TEST_CASE(setVector_overlap) {
  BitVector<64> origin;
  origin.Set(6);
  origin.Set(8);
  origin.Set(10);
  origin.Set(32);

  BitVector<64> c;
  c.Set(1);
  c.Set(2);
  c.Set(25);
  c.Set(30);
  c.Set(31);
  c.Set(45);
  c.Set(57);

  origin.SetVector(c, 50);
  BOOST_CHECK(origin.Get(6));
  BOOST_CHECK(origin.Get(32));
  BOOST_CHECK(origin.Get(51));
  BOOST_CHECK(origin.Get(52));
}

BOOST_AUTO_TEST_CASE(SetVectorAndShiftOther) {
  BitVector<64> origin;
  origin.Set(6);
  origin.Set(8);
  origin.Set(10);
  origin.Set(32);

  BitVector<64> c;
  c.Set(1);
  c.Set(2);
  c.Set(25);
  c.Set(30);
  c.Set(31);
  c.Set(45);
  c.Set(57);

  origin.SetVectorAndShiftOther(c, 14);
  BOOST_CHECK(origin.Get(6));
  BOOST_CHECK(origin.Get(8));
  BOOST_CHECK(origin.Get(11));
  BOOST_CHECK(origin.Get(16));
  BOOST_CHECK(origin.Get(17));
  BOOST_CHECK(origin.Get(31));
  BOOST_CHECK(origin.Get(32));
  BOOST_CHECK(origin.Get(43));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
