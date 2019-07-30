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
 * configuration_test.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: hendrik
 */

#include <map>

#include <boost/test/unit_test.hpp>

#include "keyvi/util/configuration.h"

namespace keyvi {
namespace util {

BOOST_AUTO_TEST_SUITE(ConfigurationTests)

BOOST_AUTO_TEST_CASE(memory) {
  std::map<std::string, std::string> memory_default({});
  BOOST_CHECK_EQUAL(DEFAULT_MEMORY_LIMIT_GENERATOR,
                    util::mapGetMemory(memory_default, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_GENERATOR));

  std::map<std::string, std::string> memory_bytes({{"memory_limit", "52428800"}});

  BOOST_CHECK_EQUAL(std::size_t(52428800),
                    util::mapGetMemory(memory_bytes, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_GENERATOR));

  std::map<std::string, std::string> memory_kb({{"memory_limit_kb", "61440"}});
  BOOST_CHECK_EQUAL(std::size_t(62914560),
                    util::mapGetMemory(memory_kb, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_GENERATOR));

  std::map<std::string, std::string> memory_mb({{"memory_limit_mb", "70"}});
  BOOST_CHECK_EQUAL(std::size_t(73400320),
                    util::mapGetMemory(memory_mb, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_GENERATOR));

  std::map<std::string, std::string> memory_gb({{"memory_limit_gb", "2"}});
  BOOST_CHECK_EQUAL(std::size_t(2147483648),
                    util::mapGetMemory(memory_gb, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_GENERATOR));
}

BOOST_AUTO_TEST_CASE(boolean) {
  std::map<std::string, std::string> bool_default({});
  BOOST_CHECK_EQUAL(false, util::mapGetBool(bool_default, "some_bool", false));
  BOOST_CHECK_EQUAL(true, util::mapGetBool(bool_default, "some_bool", true));

  std::map<std::string, std::string> bool_false({{"some_bool", "fALse"}});
  BOOST_CHECK_EQUAL(false, util::mapGetBool(bool_false, "some_bool", true));

  std::map<std::string, std::string> bool_true({{"some_bool", "truE"}});
  BOOST_CHECK_EQUAL(true, util::mapGetBool(bool_true, "some_bool", false));

  std::map<std::string, std::string> bool_on({{"some_bool", "on"}});
  BOOST_CHECK_EQUAL(true, util::mapGetBool(bool_on, "some_bool", false));

  std::map<std::string, std::string> bool_off({{"some_bool", "off"}});
  BOOST_CHECK_EQUAL(false, util::mapGetBool(bool_off, "some_bool", true));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
