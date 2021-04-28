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
 * dictionary_compiler_test.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/dictionary_index_compiler.h"
#include "keyvi/dictionary/dictionary_types.h"
#include "keyvi/dictionary/fsa/entry_iterator.h"

namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE(DictionaryIndexCompilerTests)

BOOST_AUTO_TEST_CASE(stableInsert) {
  // simulating  permutation
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"aa", "\"{1:2}\""},  {"ab", "\"{2:44}\""}, {"bb", "\"{33:23}\""}, {"cc", "\"{3:24}\""},
      {"aa", "\"{2:27}\""}, {"zz", "\"{3:21}\""}, {"zz", "\"{5:22}\""},  {"aa", "\"{3:24}\""},
  };

  keyvi::util::parameters_t params = {{"memory_limit_mb", "10"}};

  keyvi::dictionary::DictionaryIndexCompiler<dictionary_type_t::JSON> compiler(params);

  for (auto p : test_data) {
    compiler.Add(p.first, p.second);
  }

  // test delete
  compiler.Delete("ab");

  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  Dictionary d(file_name.c_str());

  fsa::automata_t f(d.GetFsa());

  fsa::EntryIterator it(f);
  fsa::EntryIterator end_it;

  BOOST_CHECK_EQUAL("aa", it.GetKey());
  BOOST_CHECK_EQUAL("\"{3:24}\"", it.GetValueAsString());
  ++it;
  BOOST_CHECK_EQUAL("bb", it.GetKey());
  BOOST_CHECK_EQUAL("\"{33:23}\"", it.GetValueAsString());
  ++it;
  BOOST_CHECK_EQUAL("cc", it.GetKey());
  BOOST_CHECK_EQUAL("\"{3:24}\"", it.GetValueAsString());
  ++it;
  BOOST_CHECK_EQUAL("zz", it.GetKey());
  BOOST_CHECK_EQUAL("\"{5:22}\"", it.GetValueAsString());
  ++it;
  BOOST_CHECK(it == end_it);
  std::remove(file_name.c_str());
}

BOOST_AUTO_TEST_CASE(addAndDeletes) {
  keyvi::util::parameters_t params = {{"memory_limit_mb", "10"}};

  keyvi::dictionary::DictionaryIndexCompiler<dictionary_type_t::JSON> compiler(params);

  // add, delete, add again
  compiler.Add("aa", "1");
  compiler.Delete("aa");
  compiler.Delete("aa");
  compiler.Add("aa", "2");

  // delete, add
  compiler.Delete("bb");
  compiler.Add("bb", "1");

  // add, delete, last item
  compiler.Add("zz", "1");
  compiler.Delete("zz");

  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  Dictionary d(file_name.c_str());

  fsa::automata_t f(d.GetFsa());

  fsa::EntryIterator it(f);
  fsa::EntryIterator end_it;

  BOOST_CHECK_EQUAL("aa", it.GetKey());
  BOOST_CHECK_EQUAL("2", it.GetValueAsString());
  ++it;

  BOOST_CHECK_EQUAL("bb", it.GetKey());
  BOOST_CHECK_EQUAL("1", it.GetValueAsString());
  ++it;

  BOOST_CHECK(it == end_it);

  std::remove(file_name.c_str());
}

void bigger_compile_test(const keyvi::util::parameters_t& params = keyvi::util::parameters_t(), size_t keys = 5000) {
  keyvi::dictionary::DictionaryIndexCompiler<dictionary_type_t::JSON> compiler(params);

  for (size_t i = 0; i < keys; ++i) {
    compiler.Add("loooooooooooooooonnnnnnnngggggggg_key-" + std::to_string(i % 50),
                 "{\"id\":" + std::to_string(i) + "}");
  }
  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  Dictionary d(file_name.c_str());

  for (size_t i = 0; i < 50; ++i) {
    keyvi::dictionary::Match m = d["loooooooooooooooonnnnnnnngggggggg_key-" + std::to_string(i)];
    BOOST_CHECK_EQUAL("loooooooooooooooonnnnnnnngggggggg_key-" + std::to_string(i), m.GetMatchedString());
    BOOST_CHECK_EQUAL("{\"id\":" + std::to_string(keys - 50 + i) + "}", m.GetValueAsString());
  }
}

BOOST_AUTO_TEST_CASE(bigger_compile) {
  bigger_compile_test({{"memory_limit_mb", "100"}});
}

BOOST_AUTO_TEST_CASE(bigger_compile_1MB_50k) {
  bigger_compile_test({{MEMORY_LIMIT_KEY, std::to_string(1024 * 1024)}}, 50000);
}

BOOST_AUTO_TEST_CASE(bigger_compile_parallel_1MB) {
  bigger_compile_test({{MEMORY_LIMIT_KEY, std::to_string(1024 * 1024)}, {PARALLEL_SORT_THRESHOLD_KEY, "1"}});
}

// see gh#215
BOOST_AUTO_TEST_CASE(parallel_sort_bug) {
  keyvi::util::parameters_t params = {{"memory_limit_mb", "10"}};
  keyvi::dictionary::DictionaryIndexCompiler<dictionary_type_t::JSON> compiler(params);

  for (size_t i = 0; i < 100000; ++i) {
    compiler.Add(std::to_string(i), "test");
  }
  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  Dictionary d(file_name.c_str());
  BOOST_CHECK(d.Contains("42"));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */
