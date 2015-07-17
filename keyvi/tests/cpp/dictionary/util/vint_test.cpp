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

#include "dictionary/util/vint.h"

namespace keyvi {
namespace dictionary {
namespace util {

BOOST_AUTO_TEST_SUITE( VariableLengthIntegerCodingTests )

BOOST_AUTO_TEST_CASE( VShortSimple ) {

  uint16_t buffer[8];
  uint8_t size;

  encodeVarshort(77777, buffer, &size);
  BOOST_CHECK_EQUAL(2, size);
  BOOST_CHECK_EQUAL(77777, decodeVarshort(buffer));

  encodeVarshort(32767, buffer, &size);
  BOOST_CHECK_EQUAL(1, size);
  BOOST_CHECK_EQUAL(32767, decodeVarshort(buffer));

  encodeVarshort(55, buffer, &size);
  BOOST_CHECK_EQUAL(1, size);
  BOOST_CHECK_EQUAL(55, decodeVarshort(buffer));
}

BOOST_AUTO_TEST_CASE( VShortLength) {

  uint16_t buffer[16];
  uint8_t size;

  encodeVarshort(77777, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarshortLength(77777), size);
  BOOST_CHECK_EQUAL(77777, decodeVarshort(buffer));

  encodeVarshort(32767, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarshortLength(1), size);
  BOOST_CHECK_EQUAL(32767, decodeVarshort(buffer));

  encodeVarshort(0x40000000, buffer, &size);
  BOOST_CHECK_EQUAL(util::getVarshortLength(0x40000000), size);
  BOOST_CHECK_EQUAL(0x40000000, decodeVarshort(buffer));

  encodeVarshort(0x200000000000, buffer, &size);

  BOOST_CHECK_EQUAL(util::getVarshortLength(0x200000000000), size);
  BOOST_CHECK_EQUAL(0x200000000000, decodeVarshort<uint64_t>(buffer));

  uint64_t x = 11687;
  BOOST_CHECK_EQUAL(1, util::getVarshortLength(x));
  encodeVarshort(x, buffer, &size);
  BOOST_CHECK_EQUAL(1, size);
}

BOOST_AUTO_TEST_CASE( VShortSimple2) {

  uint16_t buffer[16];
  uint8_t size;

  uint64_t pointer = 380108;

  auto pointer_low = pointer & 0x7; // get the lower part
  auto pointer_high = pointer >> 3; // the higher part
  BOOST_CHECK_EQUAL(47513, pointer_high);
  encodeVarshort(pointer_high, buffer, &size);
  BOOST_CHECK_EQUAL(2, size);

  BOOST_CHECK_EQUAL(47513, buffer[0]);
  BOOST_CHECK_EQUAL(1, buffer[1]);

  BOOST_CHECK_EQUAL(util::getVarshortLength(47513), size);

  BOOST_CHECK_EQUAL(47513, decodeVarshort(buffer));

  uint32_t resolved_ptr = (47513 << 3) + (8404 & 0x7);

  BOOST_CHECK_EQUAL(pointer, resolved_ptr);
}


BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */





