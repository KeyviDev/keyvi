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
 * md5_test.cpp
 *
 *  Created on: Feb 27, 2015
 *      Author: hendrik
 */


#include <boost/test/unit_test.hpp>

#include "md5.h"

namespace keyvi {
namespace dictionary {
namespace util {

BOOST_AUTO_TEST_SUITE( MD5Tests )

BOOST_AUTO_TEST_CASE( MD5HashTest ) {
  misc::MD5 m = misc::MD5();

  BOOST_CHECK_EQUAL(m.digestString("test"), "098f6bcd4621d373cade4e832627b4f6");

  BOOST_CHECK_EQUAL(14618207765679027446U, m.Hash("test"));
  BOOST_CHECK_EQUAL(13339385412431753948U, m.Hash("MYKEY"));
  BOOST_CHECK_EQUAL(16825458760271544958U, m.Hash(""));
  BOOST_CHECK_EQUAL(17513079290081602220U, m.Hash("LLLLLLLLLLLOOOOOOOOOOOONNNNNNNNNNNGGGGGGGGGG"));
  BOOST_CHECK_EQUAL(13356017483753479679U, m.Hash("-------------------------------------"));
}


BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */


