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
 * fsa_transform_test.cpp
 *
 *  Created on: Apr 8, 2015
 *      Author: hendrik
 */

#include <memory>

#include <boost/test/unit_test.hpp>

#include "keyvi/testing/temp_dictionary.h"
#include "keyvi/transform/fsa_transform.h"

namespace keyvi {
namespace transform {

BOOST_AUTO_TEST_SUITE(FSATransformTests)

BOOST_AUTO_TEST_CASE(Normalize) {
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"aa", "b"},
      {"c", "d"},
      {"caa", "ef"},
      {"a", "g"},
  };

  testing::TempDictionary dictionary(&test_data);

  auto transformer = FsaTransform(dictionary.GetFsa());

  std::string input = "aa ";

  BOOST_CHECK_EQUAL("b ", transformer.Normalize(input));
  input = "aa";
  BOOST_CHECK_EQUAL("b", transformer.Normalize(input));
  input = "caaa";
  BOOST_CHECK_EQUAL("efg", transformer.Normalize(input));
  input = "aaa";
  BOOST_CHECK_EQUAL("bg", transformer.Normalize(input));
  input = "cac";
  BOOST_CHECK_EQUAL("dgd", transformer.Normalize(input));
  input = "dcac";
  BOOST_CHECK_EQUAL("ddgd", transformer.Normalize(input));
}

BOOST_AUTO_TEST_CASE(NormalizePartialAtEnd) {
  std::vector<std::pair<std::string, std::string>> test_data = {{"aa", "x"}, {"aabc", "y"}};

  testing::TempDictionary dictionary(&test_data);

  std::shared_ptr<dictionary::Dictionary> d = std::make_shared<dictionary::Dictionary>(dictionary.GetFsa());
  auto transformer = FsaTransform(d);

  std::string input = "aab";

  BOOST_CHECK_EQUAL("xb", transformer.Normalize(input));
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace transform
}  // namespace keyvi
