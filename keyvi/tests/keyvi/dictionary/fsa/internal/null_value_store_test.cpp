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

#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/null_value_store.h"
#include "keyvi/dictionary/fsa/internal/value_store_properties.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(NullValueTest)

BOOST_AUTO_TEST_CASE(basic) {
  NullValueStore nvs;

  bool no_minimization = false;

  BOOST_CHECK_EQUAL(nvs.AddValue(42, &no_minimization), 0);
  BOOST_CHECK(no_minimization == false);
  BOOST_CHECK_EQUAL(nvs.AddValue(3535, &no_minimization), 0);
  BOOST_CHECK(no_minimization == false);
  BOOST_CHECK_EQUAL(nvs.AddValue(0, &no_minimization), 0);
  BOOST_CHECK(no_minimization == false);
}

BOOST_AUTO_TEST_CASE(reader) {
  boost::interprocess::file_mapping file_mapping;
  ValueStoreProperties properties;

  NullValueStoreReader nvsr(&file_mapping, properties);

  BOOST_CHECK(nvsr.GetValueStoreType() == value_store_t::KEY_ONLY);
  BOOST_CHECK_EQUAL(nvsr.GetValueAsAttributeVector(42), IValueStoreReader::attributes_t());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
