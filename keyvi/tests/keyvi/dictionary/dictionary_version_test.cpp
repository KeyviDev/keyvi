//
// keyvi - A key value store.
//
// Copyright 2025 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * dictionary_version_test.cpp
 *
 *  Created on: Feb 20, 2025
 *      Author: hendrik
 */

#include <string>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/dictionary_compiler.h"
#include "keyvi/dictionary/dictionary_types.h"

namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE(DictionaryVersionTests)

void check_dictionary_version_with_compression(const std::string& compression, uint64_t expected_version) {
  keyvi::dictionary::DictionaryCompiler<dictionary_type_t::JSON> compiler(
      {{"memory_limit_mb", "10"}, {"compression", compression}, {"compression_threshold", "0"}});

  for (size_t i = 0; i < 10; ++i) {
    compiler.Add("key-" + std::to_string(i), "{\"id\":" + std::to_string(i) + "}");
  }
  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  const std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  const Dictionary d(file_name);
  BOOST_CHECK_EQUAL(expected_version, d.GetVersion());
  BOOST_CHECK(std::remove(file_name.c_str()) == 0);
}

BOOST_AUTO_TEST_CASE(CheckVersionsByCompression) {
  check_dictionary_version_with_compression("raw", 2);
  check_dictionary_version_with_compression("zlib", 2);
  check_dictionary_version_with_compression("snappy", 2);
  check_dictionary_version_with_compression("zstd", 3);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */
