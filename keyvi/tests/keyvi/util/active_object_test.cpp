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
 * active_object_test.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: hendrik
 */

#include <chrono>  //NOLINT
#include <sstream>
#include <thread>  //NOLINT

#include <boost/test/unit_test.hpp>

#include "keyvi/util/active_object.h"

namespace keyvi {
namespace util {

BOOST_AUTO_TEST_SUITE(ActiveObjectTests)

void ScheduledTask(size_t* calls) {
  ++*calls;
}

BOOST_AUTO_TEST_CASE(scheduledtasktimingemptyqueue) {
#if defined(OS_MACOSX)
  // skip this test on osx to avoid failures on travis CI
  return;
#endif
  std::ostringstream string_stream;
  std::chrono::system_clock::time_point last_call;
  size_t calls = 0;

  {
    ActiveObject<std::ostringstream> wrapped_stream(&string_stream, std::bind(ScheduledTask, &calls),
                                                    std::chrono::milliseconds(20));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  BOOST_CHECK_GT(calls, 7);
  BOOST_CHECK_LT(calls, 11 + 1);
}

BOOST_AUTO_TEST_CASE(scheduledtasktimingfullqueue) {
#if defined(OS_MACOSX)
  // skip this test on osx to avoid failures on travis CI
  return;
#endif
  std::ostringstream string_stream;
  std::chrono::system_clock::time_point last_call;
  size_t calls = 0;
  auto start_time = std::chrono::high_resolution_clock::now();
  {
    ActiveObject<std::ostringstream> wrapped_stream(&string_stream, std::bind(ScheduledTask, &calls),
                                                    std::chrono::milliseconds(8));
    for (size_t i = 0; i < 15000; ++i) {
      wrapped_stream([i](std::ostream& o) { o << "Hello world" << i << std::endl; });

      if (i % 50 == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }
  auto end_time = std::chrono::high_resolution_clock::now();

  size_t duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();
  size_t expected_min_calls = (duration / 8) > 5 ? ((duration - duration / 10) / 8) - 5 : 0;

  BOOST_CHECK_GT(expected_min_calls, 0);
  BOOST_CHECK_GT(calls, expected_min_calls);
  BOOST_CHECK_LT(calls, (duration / 8) + 1 + 1);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace util */
} /* namespace keyvi */
