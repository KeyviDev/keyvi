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
 * index_writer_test.cpp
 *
 *  Created on: Jan 13, 2017
 *      Author: hendrik
 */

// this test is unix specific
#if defined(_WIN32)
#else
#include <sys/resource.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/index/constants.h"
#include "keyvi/index/index.h"

inline std::string get_keyvimerger_bin() {
  boost::filesystem::path path{std::getenv("KEYVI_UNITTEST_BASEPATH")};
  path /= DEFAULT_KEYVIMERGER_BIN;

  BOOST_CHECK(boost::filesystem::is_regular_file(path));

  return path.string();
}

inline size_t limit_filedescriptors(size_t file_descriptor_limit) {
  struct rlimit limit;

  getrlimit(RLIMIT_NOFILE, &limit);
  size_t old_limit = limit.rlim_cur;
  limit.rlim_cur = file_descriptor_limit;
  BOOST_CHECK(setrlimit(RLIMIT_NOFILE, &limit) == 0);
  getrlimit(RLIMIT_NOFILE, &limit);
  BOOST_CHECK_EQUAL(file_descriptor_limit, limit.rlim_cur);
  return old_limit;
}

namespace keyvi {
namespace index {
BOOST_AUTO_TEST_SUITE(IndexLimitsTests)

BOOST_AUTO_TEST_CASE(filedescriptor_limit) {
  using boost::filesystem::temp_directory_path;
  using boost::filesystem::unique_path;

  size_t old_limit = limit_filedescriptors(20);

  auto tmp_path = temp_directory_path();
  tmp_path /= unique_path("index-limits-test-temp-index-%%%%-%%%%-%%%%-%%%%");
  {
    Index writer(tmp_path.string(), {{"refresh_interval", "100"},
                                     {KEYVIMERGER_BIN, get_keyvimerger_bin()},
                                     {"segment_compile_key_threshold", "10"},
                                     {"max_segments", "10"}});

    for (int i = 0; i < 5000; ++i) {
      writer.Set("a", "{\"id\":" + std::to_string(i) + "}");
    }
    writer.Flush();
    BOOST_CHECK(writer.Contains("a"));
    dictionary::Match m = writer["a"];

    BOOST_CHECK_EQUAL("{\"id\":4999}", m.GetValueAsString());
  }
  boost::filesystem::remove_all(tmp_path);

  size_t increased_file_descriptors = keyvi::util::OsUtils::TryIncreaseFileDescriptors();
  BOOST_CHECK(increased_file_descriptors > 20);

  limit_filedescriptors(old_limit);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace index
}  // namespace keyvi
#endif
