//
// keyvi - A key value store.
//
// Copyright 2026 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * value_store_append_merge_test.cpp
 */

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/value_store_append_merge.h"
#include "keyvi/dictionary/fsa/internal/value_store_properties.h"

namespace keyvi::dictionary::fsa::internal {

namespace {

std::string WriteTempFile(const std::string& content) {
  const boost::filesystem::path path =
      boost::filesystem::temp_directory_path() /
      boost::filesystem::unique_path("keyvi-unit-test-value-store-append-merge-%%%%-%%%%-%%%%-%%%%");
  std::ofstream(path.string(), std::ios::binary) << content;
  return path.string();
}

}  // namespace

BOOST_AUTO_TEST_SUITE(ValueStoreAppendMergeTests)

BOOST_AUTO_TEST_CASE(CopyPayload) {
  // all byte values (incl. \0, \r, \n), longer than the copy buffer
  std::string payload;
  for (size_t i = 0; i < (3 * kCopyValueStorePayloadBufferSize) + 42; ++i) {
    payload.push_back(static_cast<char>(i % 256));
  }
  const std::string filename = WriteTempFile("header" + payload + "trailer");

  std::ostringstream out;
  CopyValueStorePayload(filename, ValueStoreProperties(6, payload.size(), 0, 0, {}), &out);
  BOOST_CHECK(out.str() == payload);

  // truncated: value store larger than the file
  BOOST_CHECK_THROW(CopyValueStorePayload(filename, ValueStoreProperties(6, payload.size() + 1000, 0, 0, {}), &out),
                    std::invalid_argument);

  boost::filesystem::remove(filename);
}

BOOST_AUTO_TEST_CASE(MissingFile) {
  std::ostringstream out;
  BOOST_CHECK_THROW(CopyValueStorePayload("does-not-exist.kv", ValueStoreProperties(0, 1, 0, 0, {}), &out),
                    std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace keyvi::dictionary::fsa::internal
