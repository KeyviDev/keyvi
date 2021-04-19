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

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary_properties.h"
#include "keyvi/dictionary/dictionary_types.h"

namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE(DictionaryPropertiesTest)

BOOST_AUTO_TEST_CASE(property_tree_bwc) {
  // file header created with 0.3.2, copied out using hex editor
  const char header_v_3_2[176] = {
      75,  69,  89,  86,  73,  70,  83,  65,  0,   0,   0,   -126, 123, 34,  118, 101, 114, 115, 105, 111, 110, 34,
      58,  34,  50,  34,  44,  34,  115, 116, 97,  114, 116, 95,   115, 116, 97,  116, 101, 34,  58,  34,  50,  34,
      44,  34,  110, 117, 109, 98,  101, 114, 95,  111, 102, 95,   107, 101, 121, 115, 34,  58,  34,  50,  34,  44,
      34,  118, 97,  108, 117, 101, 95,  115, 116, 111, 114, 101,  95,  116, 121, 112, 101, 34,  58,  34,  49,  34,
      44,  34,  110, 117, 109, 98,  101, 114, 95,  111, 102, 95,   115, 116, 97,  116, 101, 115, 34,  58,  34,  50,
      34,  44,  34,  109, 97,  110, 105, 102, 101, 115, 116, 34,   58,  34,  123, 39,  118, 101, 114, 115, 105, 111,
      110, 39,  58,  32,  52,  50,  125, 34,  125, 10,  0,   0,    0,   29,  123, 34,  118, 101, 114, 115, 105, 111,
      110, 34,  58,  34,  50,  34,  44,  34,  115, 105, 122, 101,  34,  58,  34,  50,  54,  51,  34,  125, 10,  0};

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-properties-test-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.string();

  {
    std::ofstream out_stream(file_name, std::ios::binary);
    out_stream.write(header_v_3_2, sizeof(header_v_3_2));
    // simulate the size
    out_stream.seekp(963);
    out_stream.write("", 1);
  }

  DictionaryProperties properties = DictionaryProperties::FromFile(file_name);

  BOOST_CHECK_EQUAL(2, properties.GetNumberOfKeys());
  BOOST_CHECK(properties.GetValueStoreType() == dictionary_type_t::KEY_ONLY);
  BOOST_CHECK_EQUAL("{'version': 42}", properties.GetManifest());
  BOOST_CHECK_EQUAL(2, properties.GetStartState());
  BOOST_CHECK_EQUAL(263, properties.GetSparseArraySize());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */
