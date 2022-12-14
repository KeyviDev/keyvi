/* * keyvi - A key value store.
 *
 * Copyright 2021 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#include "keyvi/dictionary/fsa/internal/float_vector_value_store.h"

#include <boost/filesystem.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/dictionary/fsa/internal/value_store_properties.h"
#include "keyvi/util/configuration.h"
#include "keyvi/util/float_vector_value.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

BOOST_AUTO_TEST_SUITE(FloatVectorValueStoreTest)

BOOST_AUTO_TEST_CASE(minimization) {
  FloatVectorValueStore float_store(
      keyvi::util::parameters_t{{TEMPORARY_PATH_KEY, "/tmp"}, {"memory_limit_mb", "10"}, {VECTOR_SIZE_KEY, "5"}});
  bool no_minimization = false;
  uint64_t v = float_store.AddValue({1.2, 1.3, 1.4, 1.5, 1.6}, &no_minimization);
  BOOST_CHECK_EQUAL(v, 0);
  uint64_t w = float_store.AddValue({1.2, 1.888, 1.44235, 2.5, 1.36}, &no_minimization);
  uint64_t x = float_store.AddValue({1.21, 1.3, 1.4, 1.5, 1.6}, &no_minimization);
  BOOST_CHECK(w > 0);
  BOOST_CHECK(x > 0);
  BOOST_CHECK_EQUAL(v, float_store.AddValue({1.2, 1.3, 1.4, 1.5, 1.6}, &no_minimization));
  BOOST_CHECK_EQUAL(x, float_store.AddValue({1.21, 1.3, 1.4, 1.5, 1.6}, &no_minimization));
  BOOST_CHECK_EQUAL(w, float_store.AddValue({1.2, 1.888, 1.44235, 2.5, 1.36}, &no_minimization));
}

BOOST_AUTO_TEST_CASE(persistence) {
  FloatVectorValueStore float_store(
      keyvi::util::parameters_t{{TEMPORARY_PATH_KEY, "/tmp"}, {"memory_limit_mb", "10"}, {VECTOR_SIZE_KEY, "5"}});
  bool no_minimization = false;

  std::vector<float> v({1.2, 1.3, 1.4, 1.5, 1.6});
  std::vector<float> w({1.2, 1.888, 1.44235, 2.5, 1.36});

  uint64_t v_idx = float_store.AddValue(v, &no_minimization);
  uint64_t w_idx = float_store.AddValue(w, &no_minimization);

  boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
  temp_path /= boost::filesystem::unique_path("float-vector-vs-unit-test-temp-dictionary-%%%%-%%%%-%%%%-%%%%");
  std::string filename = temp_path.string();

  std::ofstream out_stream(filename, std::ios::binary);
  float_store.Write(out_stream);
  out_stream.close();

  std::ifstream in_stream(filename, std::ios::binary);
  auto file_mapping = new boost::interprocess::file_mapping(filename.c_str(), boost::interprocess::read_only);

  fsa::internal::ValueStoreProperties properties = fsa::internal::ValueStoreProperties::FromJson(in_stream);

  FloatVectorValueStoreReader reader(file_mapping, properties, loading_strategy_types::lazy);
  auto actual_v = keyvi::util::DecodeFloatVector(reader.GetRawValueAsString(v_idx));
  BOOST_CHECK_EQUAL(5, actual_v.size());
  for (size_t i = 0; i < 5; ++i) {
    BOOST_CHECK_EQUAL(v[i], actual_v[i]);
  }

  auto actual_w = keyvi::util::DecodeFloatVector(reader.GetRawValueAsString(w_idx));
  BOOST_CHECK_EQUAL(5, actual_w.size());
  for (size_t i = 0; i < 5; ++i) {
    BOOST_CHECK_EQUAL(w[i], actual_w[i]);
  }
  BOOST_CHECK(reader.GetValueStoreType() == value_store_t::FLOAT_VECTOR);

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
