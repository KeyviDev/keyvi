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
 * transform_test.cpp
 *
 *  Created on: Jul 22, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/util/transform.h"

namespace keyvi {
namespace dictionary {
namespace util {

BOOST_AUTO_TEST_SUITE(TransformTests)

BOOST_AUTO_TEST_CASE(BagOfWordsPartialTest) {
  size_t number_of_tokens;

  BOOST_CHECK_EQUAL("angela dr merkel leb", Transform::BagOfWordsPartial("dr angela merkel leb", number_of_tokens));
  BOOST_CHECK_EQUAL(4, number_of_tokens);

  BOOST_CHECK_EQUAL("angela lebenslauf merkel d",
                    Transform::BagOfWordsPartial("angela merkel lebenslauf d", number_of_tokens));
  BOOST_CHECK_EQUAL(4, number_of_tokens);

  BOOST_CHECK_EQUAL("download facebook m", Transform::BagOfWordsPartial("download facebook m", number_of_tokens));
  BOOST_CHECK_EQUAL(3, number_of_tokens);

  BOOST_CHECK_EQUAL("download facebook m", Transform::BagOfWordsPartial("facebook download m", number_of_tokens));
  BOOST_CHECK_EQUAL(3, number_of_tokens);

  BOOST_CHECK_EQUAL("youtube", Transform::BagOfWordsPartial("youtube", number_of_tokens));
  BOOST_CHECK_EQUAL(1, number_of_tokens);

  BOOST_CHECK_EQUAL("youtube ", Transform::BagOfWordsPartial("youtube ", number_of_tokens));
  BOOST_CHECK_EQUAL(2, number_of_tokens);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */
