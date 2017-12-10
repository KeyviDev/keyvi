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
 * merge_process_test.cpp
 *
 *  Created on: Feb 19, 2017
 *      Author: hendrik
 */

#include <chrono>  // NOLINT
#include <thread>  // NOLINT

#include <boost/test/unit_test.hpp>

#include "dictionary/testing/temp_dictionary.h"
#include "index/internal/merge_job.h"
#include "index/internal/segment.h"

namespace keyvi {
namespace index {
namespace internal {

BOOST_AUTO_TEST_SUITE(MergeJobTests)

BOOST_AUTO_TEST_CASE(basic_merge) {
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"abbcd", "{c:3}"}, {"abcde", "{a:1}"}, {"abdd", "{b:2}"}, {"bba", "{c:3}"},
  };
  dictionary::testing::TempDictionary dictionary =
      dictionary::testing::TempDictionary::makeTempDictionaryFromJson(test_data);

  std::vector<std::pair<std::string, std::string>> test_data2 = {
      {"abbe", "{d:4}"}, {"abbc", "{b:3}"}, {"abcd", "{a:1}"}, {"bbacd", "{f:5}"},
  };
  dictionary::testing::TempDictionary dictionary2 =
      dictionary::testing::TempDictionary::makeTempDictionaryFromJson(test_data2);

  segment_t w1(new Segment(dictionary.GetFileName()));
  segment_t w2(new Segment(dictionary2.GetFileName()));

  boost::filesystem::path p("merged.kv");

  MergeJob m({w1, w2}, 0, p);
  m.Run();

  int retry = 100;
  while (retry > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if (m.TryFinalize()) {
      break;
    }
    --retry;
  }

  BOOST_CHECK(retry > 0);

  BOOST_CHECK(m.TryFinalize());
  BOOST_CHECK(m.Successful());
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */
