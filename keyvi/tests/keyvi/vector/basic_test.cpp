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

#include <boost/test/unit_test.hpp>

#include "vector/vector_types.h"

namespace keyvi {
namespace vector {

BOOST_AUTO_TEST_SUITE(VectorTests)

BOOST_AUTO_TEST_CASE(string_test) {
  StringVectorGenerator generator;

  const std::string filename = "vector_string.kvv";
  const size_t size = 100;

  for (size_t i = 0; i < size; ++i) {
    generator.PushBack(std::to_string(i));
  }

  generator.WriteToFile(filename);

  StringVector vector(filename);
  BOOST_CHECK_EQUAL(size, vector.Size());

  for (size_t i = 0; i < vector.Size(); ++i) {
    BOOST_CHECK_EQUAL(std::to_string(i), vector.Get(i));
  }
}

BOOST_AUTO_TEST_CASE(string_out_of_range) {
  StringVectorGenerator generator;

  const std::string filename = "vector_string.kvv";
  const size_t size = 100;

  for (size_t i = 0; i < size; ++i) {
    generator.PushBack(std::to_string(i));
  }

  generator.WriteToFile(filename);

  StringVector vector(filename);
  BOOST_CHECK_THROW(vector.Get(size + 1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(json_test) {
  JsonVectorGenerator generator;

  const std::string filename = "vector.kvv";
  const size_t size = 100;

  for (size_t i = 0; i < size; ++i) {
    generator.PushBack(std::to_string(i));
  }

  generator.WriteToFile(filename);

  JsonVector vector(filename);
  BOOST_CHECK_EQUAL(size, vector.Size());

  for (size_t i = 0; i < vector.Size(); ++i) {
    BOOST_CHECK_EQUAL(std::to_string(i), vector.Get(i));
  }
}

BOOST_AUTO_TEST_CASE(manifest) {
  const std::string filename = "vector.kvv";

  JsonVectorGenerator generator;
  generator.SetManifest("Some manifest");
  generator.WriteToFile(filename);

  JsonVector vector(filename);

  BOOST_CHECK_EQUAL(vector.Manifest(), "Some manifest");
}

BOOST_AUTO_TEST_CASE(wrong_value_store) {
  JsonVectorGenerator generator;

  const std::string filename = "vector.kvv";
  generator.WriteToFile(filename);

  BOOST_CHECK_THROW(std::move(StringVector(filename)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(truncation) {
  JsonVectorGenerator generator;

  const std::string filename = "vector.kvv";
  generator.WriteToFile(filename);

  // make sure file is okay
  JsonVector vector_normal(filename);

  std::ifstream in_stream(filename);
  in_stream.seekg(0, std::ios_base::end);
  const int file_size = in_stream.tellg();
  in_stream.seekg(0, std::ios_base::beg);

  std::vector<char> file_content(file_size);

  in_stream.read(file_content.data(), file_size);

  const std::string truncated_filename = "truncated_vector.kvv";

  std::ofstream truncated_file(truncated_filename);
  truncated_file.write(file_content.data(), file_size - 1);
  truncated_file.close();

  BOOST_CHECK_THROW(std::move(JsonVector(truncated_filename)), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace vector
}  // namespace keyvi
