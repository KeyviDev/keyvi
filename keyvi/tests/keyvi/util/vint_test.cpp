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
 * vint_test.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/util/vint.h"

namespace keyvi {
namespace util {

BOOST_AUTO_TEST_SUITE(VariableLengthIntegerCodingTests)

BOOST_AUTO_TEST_CASE(VShortSimple) {
  uint16_t buffer[8];
  size_t size;

  encodeVarShort(77777, buffer, &size);
  BOOST_CHECK_EQUAL(2, size);
  BOOST_CHECK_EQUAL(77777, decodeVarShort(buffer));

  encodeVarShort(32767, buffer, &size);
  BOOST_CHECK_EQUAL(1, size);
  BOOST_CHECK_EQUAL(32767, decodeVarShort(buffer));

  encodeVarShort(55, buffer, &size);
  BOOST_CHECK_EQUAL(1, size);
  BOOST_CHECK_EQUAL(55, decodeVarShort(buffer));
}

BOOST_AUTO_TEST_CASE(VShortLength) {
  uint16_t buffer[16];
  size_t size;

  encodeVarShort(77777, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarShortLength(77777), size);
  BOOST_CHECK_EQUAL(77777, decodeVarShort(buffer));

  encodeVarShort(32767, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarShortLength(32767), size);
  BOOST_CHECK_EQUAL(1, size);
  BOOST_CHECK_EQUAL(32767, decodeVarShort(buffer));

  encodeVarShort(32768, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarShortLength(32768), size);
  BOOST_CHECK_EQUAL(2, size);
  BOOST_CHECK_EQUAL(32768, decodeVarShort(buffer));

  encodeVarShort(0x3fffffff, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarShortLength(0x3fffffff), size);
  BOOST_CHECK_EQUAL(2, size);
  BOOST_CHECK_EQUAL(0x3fffffff, decodeVarShort(buffer));

  encodeVarShort(0x40000000, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarShortLength(0x40000000), size);
  BOOST_CHECK_EQUAL(3, size);
  BOOST_CHECK_EQUAL(0x40000000, decodeVarShort(buffer));

  encodeVarShort(0x1fffffffffff, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarShortLength(0x1fffffffffff), size);
  BOOST_CHECK_EQUAL(3, size);
  BOOST_CHECK_EQUAL(0x1fffffffffff, decodeVarShort<uint64_t>(buffer));

  encodeVarShort(0x200000000000, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarShortLength(0x200000000000), size);
  BOOST_CHECK_EQUAL(4, size);
  BOOST_CHECK_EQUAL(0x200000000000, decodeVarShort<uint64_t>(buffer));

  uint64_t x = 11687;
  BOOST_CHECK_EQUAL(1, util::getVarShortLength(x));
  encodeVarShort(x, buffer, &size);
  BOOST_CHECK_EQUAL(1, size);
}

BOOST_AUTO_TEST_CASE(VShortSimple2) {
  uint16_t buffer[16];
  size_t size;

  uint64_t pointer = 380108;

  auto pointer_high = pointer >> 3;  // the higher part
  BOOST_CHECK_EQUAL(47513, pointer_high);
  encodeVarShort(pointer_high, buffer, &size);
  BOOST_CHECK_EQUAL(2, size);

  BOOST_CHECK_EQUAL(47513, buffer[0]);
  BOOST_CHECK_EQUAL(1, buffer[1]);

  BOOST_CHECK_EQUAL(util::getVarShortLength(47513), size);

  BOOST_CHECK_EQUAL(47513, decodeVarShort(buffer));

  uint32_t resolved_ptr = (47513 << 3) + (8404 & 0x7);

  BOOST_CHECK_EQUAL(pointer, resolved_ptr);
}

BOOST_AUTO_TEST_CASE(VIntLength) {
  uint8_t buffer[32];
  size_t size;

  encodeVarInt(77777, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarIntLength(77777), size);
  BOOST_CHECK_EQUAL(77777, decodeVarInt(buffer));

  encodeVarInt(127, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarIntLength(1), size);
  BOOST_CHECK_EQUAL(127, decodeVarInt(buffer));

  encodeVarInt(0x40000000, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarIntLength(0x40000000), size);
  BOOST_CHECK_EQUAL(0x40000000, decodeVarInt(buffer));

  encodeVarInt(0x200000000000, buffer, &size);

  BOOST_CHECK_EQUAL(util::getVarIntLength(0x200000000000), size);
  BOOST_CHECK_EQUAL(0x200000000000, decodeVarInt<uint64_t>(buffer));

  uint64_t x = 11687;
  BOOST_CHECK_EQUAL(2, util::getVarIntLength(x));
  encodeVarInt(x, buffer, &size);
  BOOST_CHECK_EQUAL(2, size);
}

BOOST_AUTO_TEST_CASE(VIntDecode) {
  uint8_t buffer[32];
  size_t size;
  encodeVarInt(15, buffer, &size);
  BOOST_CHECK_EQUAL(1, size);

  size = 99;
  const char* buf_ptr = decodeVarIntString((const char*)buffer, &size);

  BOOST_CHECK_EQUAL((const char*)buffer + 1, buf_ptr);
  BOOST_CHECK_EQUAL(15, size);

  encodeVarInt(800, buffer, &size);
  BOOST_CHECK_EQUAL(2, size);

  size = 99;
  buf_ptr = decodeVarIntString((const char*)buffer, &size);

  BOOST_CHECK_EQUAL((const char*)buffer + 2, buf_ptr);
  BOOST_CHECK_EQUAL(800, size);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
