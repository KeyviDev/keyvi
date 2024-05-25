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

/*
 * secondary_key_dictionary_test.cpp
 *
 *  Created on: May 25, 2024
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/secondary_key_dictionary.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
BOOST_AUTO_TEST_SUITE(SecondaryKeyDictionaryTests)

BOOST_AUTO_TEST_CASE(OneSecondaryKey) {
  std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"acme:siegfried", 22},
      {"acma:walpurga", 10},
  };

  testing::TempDictionary dictionary(&test_data);
  secondary_key_dictionary_t d(new SecondaryKeyDictionary(dictionary.GetFsa()));

  auto m = d->GetFirst("siegfried", {{"skey", "acme"}});
  BOOST_CHECK_EQUAL(22, m.GetWeight());

  auto completer = d->GetMultiwordCompletion("sie", {{"skey", "acme"}});
  auto completer_it = completer.begin();
  size_t i = 0;

  while (completer_it != completer.end()) {
    BOOST_CHECK_EQUAL("siegfried", completer_it->GetMatchedString());
    BOOST_CHECK_EQUAL(22, completer_it->GetWeight());
  }
  BOOST_CHECK_EQUAL(1, i);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */
