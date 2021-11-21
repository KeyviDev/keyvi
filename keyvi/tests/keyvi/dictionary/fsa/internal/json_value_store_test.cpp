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
 * json_value_store_test.cpp
 *
 *  Created on: Nov 5, 2014
 *      Author: hendrik
 */

#include "keyvi/dictionary/fsa/internal/json_value_store.h"

#include <boost/filesystem.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/dictionary/fsa/internal/value_store_properties.h"
#include "keyvi/util/configuration.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(JsonValueTest)

BOOST_AUTO_TEST_CASE(minimization) {
  JsonValueStore strings(keyvi::util::parameters_t{{TEMPORARY_PATH_KEY, "/tmp"}, {"memory_limit_mb", "10"}});
  bool no_minimization = false;
  uint64_t v = strings.AddValue("{\"mytestvalue\":25, \"mytestvalue2\":23}", &no_minimization);
  BOOST_CHECK_EQUAL(v, 0);
  uint64_t w = strings.AddValue("othervalue", &no_minimization);
  uint64_t x = strings.AddValue("{\"mytestvalue3\":55, \"mytestvalue4\":773}", &no_minimization);

  BOOST_CHECK(w > 0);
  BOOST_CHECK_EQUAL(v, strings.AddValue("{\"mytestvalue\": 25, \"mytestvalue2\": 23}", &no_minimization));
  BOOST_CHECK_EQUAL(x, strings.AddValue("{\"mytestvalue3\":55, \"mytestvalue4\":773}", &no_minimization));
  BOOST_CHECK_EQUAL(w, strings.AddValue("othervalue", &no_minimization));
}

BOOST_AUTO_TEST_CASE(minimization_largevalues) {
  JsonValueStore values(keyvi::util::parameters_t{{TEMPORARY_PATH_KEY, "/tmp"}, {"memory_limit_mb", "10"}});
  bool no_minimization = false;
  // create a value that is longer than a ushort can store
  std::string value = "{\"";
  value += std::string(70000, 'a');
  value += "\":42}";

  uint64_t v = values.AddValue(value, &no_minimization);
  BOOST_CHECK_EQUAL(v, 0);
  uint64_t w = values.AddValue("othervalue", &no_minimization);
  BOOST_CHECK(v != w);

  BOOST_CHECK(w > 0);
  BOOST_CHECK_EQUAL(v, values.AddValue(value, &no_minimization));
  BOOST_CHECK_EQUAL(w, values.AddValue("othervalue", &no_minimization));
}

BOOST_AUTO_TEST_CASE(minimization_largevalue_small_memory) {
  // combination of small value store and large values, spawning more than 1 chunk
  JsonValueStore values(keyvi::util::parameters_t{{TEMPORARY_PATH_KEY, "/tmp"}, {"memory_limit", "500000"}});
  bool no_minimization = false;
  // create a value that almost fills the 1st chunk
  std::string padding_value = "{\"";
  padding_value += std::string(49990, 'a');
  padding_value += "\":33}";
  values.AddValue(padding_value, &no_minimization);

  std::string value = "{\"";
  value += std::string(60000, 'a');
  value += "\":42, ";
  value += std::string(30000, 'b');
  value += "\":99,";
  value += std::string(4000, 'c');
  value += "\":12}";

  uint64_t v = values.AddValue(value, &no_minimization);
  BOOST_CHECK_EQUAL(v, 49999);
  uint64_t w = values.AddValue("othervalue", &no_minimization);
  BOOST_CHECK(v != w);

  BOOST_CHECK(w > 0);
  BOOST_CHECK_EQUAL(v, values.AddValue(value, &no_minimization));
  BOOST_CHECK_EQUAL(w, values.AddValue("othervalue", &no_minimization));
}

BOOST_AUTO_TEST_CASE(minimization2) {
  JsonValueStore strings(keyvi::util::parameters_t{{TEMPORARY_PATH_KEY, "/tmp"}, {"memory_limit_mb", "10"}});
  bool no_minimization = false;

  uint64_t v = strings.AddValue(
      "{\"f\": 5571575, \"df\": 1362790, \"uqf\": 2129086, \"tf1df\": 99838, \"tf2df\": 274586, \"tf3df\": 481278, "
      "\"tf5df\": 811157}",
      &no_minimization);
  uint64_t w = strings.AddValue(
      "{\"f\": 3, \"df\": 1, \"uqf\": 1, \"tf1df\": 0, \"tf2df\": 0, \"tf3df\": 0, \"tf5df\": 0}", &no_minimization);
  BOOST_CHECK(v != w);
}

BOOST_AUTO_TEST_CASE(persistence) {
  JsonValueStore json_value_store(keyvi::util::parameters_t{{TEMPORARY_PATH_KEY, "/tmp"}, {"memory_limit_mb", "10"}});
  bool no_minimization = false;
  std::string value = "{\"";
  value += std::string(60000, 'a');
  value += "\":42}";

  uint64_t v = json_value_store.AddValue(value, &no_minimization);
  uint64_t w = json_value_store.AddValue("{\"mytestvalue2\":23}", &no_minimization);

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

  temp_path /= boost::filesystem::unique_path("dictionary-unit-test-temp-dictionary-%%%%-%%%%-%%%%-%%%%");

  std::string filename = temp_path.string();

  std::ofstream out_stream(filename, std::ios::binary);
  json_value_store.Write(out_stream);
  out_stream.close();

  std::ifstream in_stream(filename, std::ios::binary);
  auto file_mapping = new boost::interprocess::file_mapping(filename.c_str(), boost::interprocess::read_only);

  fsa::internal::ValueStoreProperties properties = fsa::internal::ValueStoreProperties::FromJson(in_stream);

  JsonValueStoreReader reader(file_mapping, properties, loading_strategy_types::lazy);

  BOOST_CHECK_EQUAL(value, reader.GetValueAsString(v));
  BOOST_CHECK_EQUAL("{\"mytestvalue2\":23}", reader.GetValueAsString(w));
  BOOST_CHECK(reader.GetValueStoreType() == value_store_t::JSON);

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
