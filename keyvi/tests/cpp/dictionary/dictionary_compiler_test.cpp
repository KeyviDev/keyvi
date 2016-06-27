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

#include "dictionary/dictionary_compiler.h"
#include "dictionary/dictionary.h"
#include "dictionary/fsa/automata.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/fsa/internal/int_value_store.h"
#include "dictionary/fsa/internal/json_value_store.h"
#include "dictionary/fsa/entry_iterator.h"

namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE( DictionaryCompilerTests )

class DCTTestHelper final{
 public:
  static uint32_t GetStateIdForPrefix(const fsa::automata_t& fsa, std::string& query)
{

  uint32_t state = fsa->GetStartState();
  size_t depth = 0;
  size_t query_length = query.size();

  while (state != 0 && depth != query_length) {
          state = fsa->TryWalkTransition(state, query[depth]);
          ++depth;
        }

  return state;
}
};

BOOST_AUTO_TEST_CASE( minimizationIntInnerWeights ) {

  // simulating  permutation
  std::vector<std::pair<std::string, uint32_t>> test_data = {
    { "fb#fb msg downl de", 22 },
    { "msg#fb msg downl de", 22 },
    { "downl#fb msg downl de", 22 },
    { "de#fb msg downl de", 22 },

    { "fb msg#fb msg downl de", 22 },
    { "fb downl#fb msg downl de", 22 },
    { "fb de#fb msg downl de", 22 },

    { "msg fb#fb msg downl de", 22 },
    { "msg downl#fb msg downl de", 22 },
    { "msg de#fb msg downl de", 22 },

    { "downl fb#fb msg downl de", 22 },
    { "downl msg#fb msg downl de", 22 },
    { "downl de#fb msg downl de", 22 },

    { "de fb#fb msg downl de", 22 },
    { "de msg#fb msg downl de", 22 },
    { "de downl#fb msg downl de", 22 },
  };

  keyvi::dictionary::DictionaryCompiler<
      fsa::internal::SparseArrayPersistence<>,
      fsa::internal::IntValueStoreWithInnerWeights> compiler;

  for (auto p: test_data){
    compiler.Add(p.first, p.second);
  }
  compiler.Compile();

  boost::filesystem::path temp_path =
      boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path(
      "dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.native();

  compiler.WriteToFile(file_name);

  Dictionary d(file_name.c_str());
  fsa::automata_t a = d.GetFsa();

  std::string reference_value ("de#");
  uint32_t reference_weight = DCTTestHelper::GetStateIdForPrefix(a,reference_value);

  std::vector<std::string> query_data = {
      "fb#", "msg#", "downl#",
      "fb msg#", "fb downl#",
      "downl fb#", "downl de#"
  };
  std::cout << "test "<< std::endl;

  int tested_values = 0;
  for (auto q: query_data){
    std::cout << "test "<<q<<std::endl;
    BOOST_CHECK(reference_weight == DCTTestHelper::GetStateIdForPrefix(a, q));
    ++tested_values;
  }

  BOOST_CHECK(tested_values == query_data.size());
  std::remove(file_name.c_str());
}

BOOST_AUTO_TEST_CASE( sortOrder ) {

  // simulating  permutation
  std::vector<std::pair<std::string, uint32_t>> test_data = {
    { "uboot", 22 },
    { "überfall", 33 },
    { "vielleicht", 43 },
    { "arbeit", 3 },
    { "zoo", 5 },
    { "ändern", 6 },
  };

  keyvi::dictionary::DictionaryCompiler<
      fsa::internal::SparseArrayPersistence<>,
      fsa::internal::IntValueStoreWithInnerWeights> compiler;

  for (auto p: test_data){
    compiler.Add(p.first, p.second);
  }
  compiler.Compile();

  boost::filesystem::path temp_path =
      boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path(
      "dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.native();

  compiler.WriteToFile(file_name);

  Dictionary d(file_name.c_str());

  fsa::automata_t f(d.GetFsa());

  fsa::EntryIterator it(f);
  fsa::EntryIterator end_it;

  std::stringstream ss;

  BOOST_CHECK_EQUAL("arbeit", it.GetKey());
  it.WriteKey(ss);
  BOOST_CHECK_EQUAL("arbeit", ss.str());
  ss.str("");
  ss.clear();
  ++it;
  BOOST_CHECK_EQUAL("uboot", it.GetKey());
  it.WriteKey(ss);
  BOOST_CHECK_EQUAL("uboot", ss.str());
  ss.str("");
  ss.clear();

  ++it;
  BOOST_CHECK_EQUAL("vielleicht", it.GetKey());
  it.WriteKey(ss);
  BOOST_CHECK_EQUAL("vielleicht", ss.str());
  ss.str("");
  ss.clear();

  ++it;
  BOOST_CHECK_EQUAL("zoo", it.GetKey());
  it.WriteKey(ss);
  BOOST_CHECK_EQUAL("zoo", ss.str());
  ss.str("");
  ss.clear();

  ++it;
  BOOST_CHECK_EQUAL("ändern", it.GetKey());
  it.WriteKey(ss);
  BOOST_CHECK_EQUAL("ändern", ss.str());
  ss.str("");
  ss.clear();

  ++it;
  BOOST_CHECK_EQUAL("überfall", it.GetKey());
  it.WriteKey(ss);
  BOOST_CHECK_EQUAL("überfall", ss.str());
  ss.str("");
  ss.clear();

  ++it;
  BOOST_CHECK(it == end_it);

  std::remove(file_name.c_str());
}

BOOST_AUTO_TEST_CASE( compactSize ) {

  // simulating  permutation
  std::vector<std::pair<std::string, uint32_t>> test_data = {
    { "uboot", 22 },
    { "überfall", 33 },
    { "vielleicht", 43 },
    { "arbeit", 3 },
    { "zoo", 5 },
    { "ändern", 6 },
  };

  keyvi::dictionary::DictionaryCompiler<
      fsa::internal::SparseArrayPersistence<uint16_t>,
      fsa::internal::IntValueStoreWithInnerWeights> compiler;

  for (auto p: test_data){
    compiler.Add(p.first, p.second);
  }
  compiler.Compile();

  boost::filesystem::path temp_path =
      boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path(
      "dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.native();

  compiler.WriteToFile(file_name);

  Dictionary d(file_name.c_str());

  fsa::automata_t f(d.GetFsa());

  fsa::EntryIterator it(f);
  fsa::EntryIterator end_it;

  BOOST_CHECK_EQUAL("arbeit", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("uboot", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("vielleicht", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("zoo", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("ändern", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("überfall", it.GetKey());
  ++it;
  BOOST_CHECK(it == end_it);
}

BOOST_AUTO_TEST_CASE( stableInsert ) {

  // simulating  permutation
  std::vector<std::pair<std::string, std::string>> test_data = {
    { "aa", "\"{1:2}\"" },
    { "bb", "\"{33:23}\"" },
    { "cc", "\"{3:24}\"" },
    { "aa", "\"{2:27}\"" },
    { "zz", "\"{3:21}\"" },
    { "zz", "\"{5:22}\"" },
    { "aa", "\"{3:24}\"" },
  };

  keyvi::dictionary::compiler_param_t params = {{STABLE_INSERTS, "true"}};

  keyvi::dictionary::DictionaryCompiler<
      fsa::internal::SparseArrayPersistence<uint16_t>,
      fsa::internal::JsonValueStore> compiler(10485760, params);

  for (auto p: test_data){
    compiler.Add(p.first, p.second);
  }
  compiler.Compile();

  boost::filesystem::path temp_path =
      boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path(
      "dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  std::string file_name = temp_path.native();

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
}


BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */
