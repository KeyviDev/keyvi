/* * keyvi - A key value store.
 *
 * Copyright 2018   Narek Gharibyan<narekgharibyan@gmail.com>
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
 *  basic_test.h
 *
 *  Created on: March 17, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#include <exception>
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/vector/vector_types.h"

namespace keyvi {
namespace vector {

namespace {
template <typename VectorType>
struct TempVectorGenerator {
  TempVectorGenerator() {
    boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
    temp_path /= boost::filesystem::unique_path("vector-unit-test-%%%%-%%%%-%%%%-%%%%.kvv");
    filename = temp_path.string();
  }

  ~TempVectorGenerator() {
    if (boost::filesystem::exists(filename)) {
      boost::filesystem::remove(filename);
    }
  }

  void WriteToFile() { vector.WriteToFile(filename); }

  VectorType vector;
  std::string filename;
};

}  // namespace

BOOST_AUTO_TEST_SUITE(VectorTests)

BOOST_AUTO_TEST_CASE(string_test) {
  TempVectorGenerator<StringVectorGenerator> temp_vector;
  const size_t size = 100;

  for (size_t i = 0; i < size; ++i) {
    temp_vector.vector.PushBack(std::to_string(i));
  }

  temp_vector.WriteToFile();

  StringVector vector(temp_vector.filename);
  BOOST_CHECK_EQUAL(size, vector.Size());

  for (size_t i = 0; i < vector.Size(); ++i) {
    BOOST_CHECK_EQUAL(std::to_string(i), vector.Get(i));
  }
}

BOOST_AUTO_TEST_CASE(string_out_of_range) {
  TempVectorGenerator<StringVectorGenerator> temp_vector;
  const size_t size = 100;

  for (size_t i = 0; i < size; ++i) {
    temp_vector.vector.PushBack(std::to_string(i));
  }

  temp_vector.WriteToFile();

  StringVector vector(temp_vector.filename);
  BOOST_CHECK_THROW(vector.Get(size + 1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(json_test) {
  TempVectorGenerator<JsonVectorGenerator> temp_vector;
  const size_t size = 100;

  for (size_t i = 0; i < size; ++i) {
    temp_vector.vector.PushBack(std::to_string(i));
  }

  temp_vector.WriteToFile();

  JsonVector vector(temp_vector.filename);
  BOOST_CHECK_EQUAL(size, vector.Size());

  for (size_t i = 0; i < vector.Size(); ++i) {
    BOOST_CHECK_EQUAL(std::to_string(i), vector.Get(i));
  }
}

BOOST_AUTO_TEST_CASE(manifest) {
  TempVectorGenerator<JsonVectorGenerator> temp_vector;
  temp_vector.vector.SetManifest("Some manifest");
  temp_vector.WriteToFile();

  JsonVector vector(temp_vector.filename);

  BOOST_CHECK_EQUAL(vector.Manifest(), "Some manifest");
}

BOOST_AUTO_TEST_CASE(wrong_value_store) {
  TempVectorGenerator<JsonVectorGenerator> temp_vector;
  temp_vector.WriteToFile();

  BOOST_CHECK_THROW(std::move(StringVector(temp_vector.filename)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(truncation) {
  TempVectorGenerator<JsonVectorGenerator> temp_vector;
  temp_vector.WriteToFile();

  // make sure file is okay
  JsonVector vector_normal(temp_vector.filename);

  std::ifstream in_stream(temp_vector.filename);
  in_stream.seekg(0, std::ios_base::end);
  const int file_size = in_stream.tellg();
  in_stream.seekg(0, std::ios_base::beg);

  std::vector<char> file_content(file_size);

  in_stream.read(file_content.data(), file_size);

  const std::string truncated_filename = temp_vector.filename + "-truncated";

  std::ofstream truncated_file(truncated_filename);
  truncated_file.write(file_content.data(), file_size - 1);
  truncated_file.close();

  BOOST_CHECK_THROW(std::move(JsonVector(truncated_filename)), std::invalid_argument);

  boost::filesystem::remove(truncated_filename);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace vector
}  // namespace keyvi
