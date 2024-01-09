/** keyvi - A key value store.
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

#include <boost/test/unit_test.hpp>

#include "keyvi/util/float_vector_value.h"

namespace keyvi {
namespace util {

BOOST_AUTO_TEST_SUITE(FloatVectorValueTests)

BOOST_AUTO_TEST_CASE(EncodeDecodeTest) {
  std::vector<float> v({1.2, 1.3, 1.4, 1.5, 1.6});
  std::string encoded = EncodeFloatVector(v, 5);
  std::vector<float> decoded = DecodeFloatVector(encoded);
  BOOST_CHECK_EQUAL(5, decoded.size());

  for (size_t i = 0; i < 5; ++i) {
    BOOST_CHECK_EQUAL(v[i], decoded[i]);
  }
}

BOOST_AUTO_TEST_CASE(AsStringTest) {
  std::vector<float> v({1.2, 1.3, 1.4, 1.5, 1.6});
  BOOST_CHECK_EQUAL("1.2, 1.3, 1.4, 1.5, 1.6", FloatVectorAsString(v, ", "));

  std::vector<float> w({1.8});
  BOOST_CHECK_EQUAL("1.8", FloatVectorAsString(w, ", "));

  std::vector<float> x;
  BOOST_CHECK_EQUAL("", FloatVectorAsString(x, ", "));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
