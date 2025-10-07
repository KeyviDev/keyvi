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
 * main.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: hendrik
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Keyvi Unit Test Suite
#define BOOST_TEST_NO_MAIN

#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <boost/test/unit_test.hpp>

// entry point:
int main(int argc, char* argv[]) {
  std::filesystem::path path_of_executable{std::filesystem::absolute(argv[0])};  // NOLINT
  path_of_executable.remove_filename();

  const std::filesystem::path base_path = std::filesystem::canonical(path_of_executable);

  std::cout << "Running unit tests from path: " << base_path.string() << std::endl;  // NOLINT

  // set an environment variable, to be used in tests
#if defined(_WIN32)
  _putenv_s("KEYVI_UNITTEST_BASEPATH", base_path.string().c_str());
#else
  setenv("KEYVI_UNITTEST_BASEPATH", base_path.string().c_str(), 1);  // NOLINT
#endif

  return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}
