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

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/dictionary_compiler.h"
#include "keyvi/dictionary/dictionary_index_compiler.h"
#include "keyvi/dictionary/dictionary_types.h"
#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/entry_iterator.h"
#include "keyvi/dictionary/fsa/internal/int_inner_weights_value_store.h"
#include "keyvi/dictionary/fsa/internal/int_value_store.h"
#include "keyvi/dictionary/fsa/internal/json_value_store.h"
#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/util/configuration.h"
#include "keyvi/util/float_vector_value.h"

namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE(DictionaryCompilerTests)

class DCTTestHelper final {
 public:
  static uint32_t GetStateIdForPrefix(const fsa::automata_t& fsa, const std::string& query) {
    uint32_t state = fsa->GetStartState();
    size_t depth = 0;
    const size_t query_length = query.size();

    while (state != 0 && depth != query_length) {
      state = fsa->TryWalkTransition(state, query[depth]);
      ++depth;
    }

    return state;
  }
};

using int_with_weight_types =
    boost::mpl::list<keyvi::dictionary::DictionaryCompiler<dictionary_type_t::INT_WITH_WEIGHTS>,
                     keyvi::dictionary::DictionaryIndexCompiler<dictionary_type_t::INT_WITH_WEIGHTS>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(minimizationIntInnerWeights, DictT, int_with_weight_types) {
  // simulating  permutation
  const std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"fb#fb msg downl de", 22},       {"msg#fb msg downl de", 22},      {"downl#fb msg downl de", 22},
      {"de#fb msg downl de", 22},       {"fb msg#fb msg downl de", 22},   {"fb downl#fb msg downl de", 22},
      {"fb de#fb msg downl de", 22},    {"msg fb#fb msg downl de", 22},   {"msg downl#fb msg downl de", 22},
      {"msg de#fb msg downl de", 22},   {"downl fb#fb msg downl de", 22}, {"downl msg#fb msg downl de", 22},
      {"downl de#fb msg downl de", 22}, {"de fb#fb msg downl de", 22},    {"de msg#fb msg downl de", 22},
      {"de downl#fb msg downl de", 22},
  };

  DictT compiler(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

  for (const auto& p : test_data) {
    compiler.Add(p.first, p.second);
  }
  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  const std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  const Dictionary d(file_name);
  const fsa::automata_t a = d.GetFsa();

  const std::string reference_value("de#");
  const uint32_t reference_weight = DCTTestHelper::GetStateIdForPrefix(a, reference_value);

  const std::vector<std::string> query_data = {"fb#",       "msg#",      "downl#",   "fb msg#",
                                               "fb downl#", "downl fb#", "downl de#"};

  size_t tested_values = 0;
  for (const auto& q : query_data) {
    BOOST_CHECK(reference_weight == DCTTestHelper::GetStateIdForPrefix(a, q));
    ++tested_values;
  }

  BOOST_CHECK(tested_values == query_data.size());
  BOOST_CHECK(std::remove(file_name.c_str()) == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(unsortedKeys, DictT, int_with_weight_types) {
  // simulating  permutation
  const std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"uboot", 22}, {"überfall", 33}, {"vielleicht", 43}, {"arbeit", 3}, {"zoo", 5}, {"ändern", 6},
  };

  DictT compiler(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

  for (const auto& p : test_data) {
    compiler.Add(p.first, p.second);
  }
  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  const std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  const Dictionary d(file_name);

  const fsa::automata_t f(d.GetFsa());

  fsa::EntryIterator it(f);
  const fsa::EntryIterator end_it;

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

  BOOST_CHECK(std::remove(file_name.c_str()) == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(compactSize, DictT, int_with_weight_types) {
  // simulating  permutation
  const std::vector<std::pair<std::string, uint32_t>> test_data = {
      {"uboot", 22}, {"überfall", 33}, {"vielleicht", 43}, {"arbeit", 3}, {"zoo", 5}, {"ändern", 6},
  };

  DictT compiler(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

  for (const auto& p : test_data) {
    compiler.Add(p.first, p.second);
  }
  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  const std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  const Dictionary d(file_name);

  const fsa::automata_t f(d.GetFsa());

  fsa::EntryIterator it(f);
  const fsa::EntryIterator end_it;

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

  BOOST_CHECK(std::remove(file_name.c_str()) == 0);
}

using json_types = boost::mpl::list<keyvi::dictionary::DictionaryCompiler<dictionary_type_t::JSON>,
                                    keyvi::dictionary::DictionaryIndexCompiler<dictionary_type_t::JSON>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(MultipleKeyUpdateAndCompile, DictT, json_types) {
  DictT compiler(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

  compiler.Add("a", "1");
  compiler.Add("b", "1");
  compiler.Compile();
  BOOST_CHECK_THROW(compiler.Add("a", "1"), compiler_exception);
}

void bigger_compile_test(const keyvi::util::parameters_t& params = keyvi::util::parameters_t(), size_t keys = 5000) {
  keyvi::dictionary::DictionaryCompiler<dictionary_type_t::JSON> compiler(params);

  for (size_t i = 0; i < keys; ++i) {
    compiler.Add("loooooooooooooooonnnnnnnngggggggg_key-" + std::to_string(i), "{\"id\":" + std::to_string(i) + "}");
  }
  compiler.Compile();

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  const std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  const Dictionary d(file_name);

  for (size_t i = 0; i < keys; ++i) {
    const keyvi::dictionary::match_t m = d["loooooooooooooooonnnnnnnngggggggg_key-" + std::to_string(i)];
    BOOST_CHECK_EQUAL("loooooooooooooooonnnnnnnngggggggg_key-" + std::to_string(i), m->GetMatchedString());
    BOOST_CHECK_EQUAL("{\"id\":" + std::to_string(i) + "}", m->GetValueAsString());
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

BOOST_AUTO_TEST_CASE(float_dictionary) {
  DictionaryCompiler<dictionary_type_t::FLOAT_VECTOR> compiler(
      keyvi::util::parameters_t({{"memory_limit_mb", "10"}, {VECTOR_SIZE_KEY, "5"}}));

  compiler.Add("abbe", {3.1, 0.2, 1.3, 0.4, 0.5});

  compiler.Compile();
  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-dictionarycompiler-%%%%-%%%%-%%%%-%%%%");
  const std::string file_name = temp_path.string();

  compiler.WriteToFile(file_name);

  const Dictionary d(file_name);

  bool matched = false;
  for (const auto& m : d.Get("abbe")) {
    BOOST_CHECK_EQUAL("3.1, 0.2, 1.3, 0.4, 0.5", m->GetValueAsString());
    std::vector<float> float_vector = keyvi::util::DecodeFloatVector(m->GetRawValueAsString());
    BOOST_CHECK_EQUAL(5, float_vector.size());
    BOOST_CHECK_EQUAL(3.1F, float_vector[0]);
    BOOST_CHECK_EQUAL(1.3F, float_vector[2]);
    BOOST_CHECK_EQUAL(0.5F, float_vector[4]);
    matched = true;
  }
  BOOST_CHECK(matched);

  BOOST_CHECK(std::remove(file_name.c_str()) == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MultipleCompile, DictT, json_types) {
  DictT compiler(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

  compiler.Add("a", "1");
  compiler.Add("b", "1");
  compiler.Compile();
  compiler.Compile();
  compiler.Compile();
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */
