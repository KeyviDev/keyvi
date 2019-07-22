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
 * levenshtein_test.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: hendrik
 */
#include <cstdint>

#include <boost/test/unit_test.hpp>

#include "utf8.h"

#include "keyvi/stringdistance/levenshtein.h"

namespace keyvi {
namespace stringdistance {

BOOST_AUTO_TEST_SUITE(LevenshteinTests)

void TestDistance(Levenshtein* metric, std::string candidate, std::vector<int> intermediateScores, int finalScore) {
  std::vector<uint32_t> codepoints;
  utf8::unchecked::utf8to32(candidate.begin(), candidate.end(), back_inserter(codepoints));

  for (size_t i = 0; i < codepoints.size(); ++i) {
    BOOST_CHECK_EQUAL(intermediateScores[i], metric->Put(codepoints[i], i));
  }

  BOOST_CHECK_EQUAL(candidate, metric->GetCandidate());
  BOOST_CHECK_EQUAL(finalScore, metric->GetScore());
}

BOOST_AUTO_TEST_CASE(exact) {
  std::string input_string = "text";
  std::string comparison_string = "text";
  std::vector<uint32_t> codepoints;

  utf8::unchecked::utf8to32(input_string.begin(), input_string.end(), back_inserter(codepoints));

  Levenshtein ls(codepoints, 20, 3);
  std::vector<int32_t> scores = {0, 0, 0, 0};

  TestDistance(&ls, comparison_string, scores, 0);
}

BOOST_AUTO_TEST_CASE(approximate) {
  std::string input_string = "text";
  std::string comparison_string = "teller";
  std::vector<uint32_t> codepoints;

  utf8::unchecked::utf8to32(input_string.begin(), input_string.end(), back_inserter(codepoints));

  Levenshtein ls(codepoints, 20, 3);
  std::vector<int32_t> scores = {0, 0, 1, 2, 3, 4};

  TestDistance(&ls, comparison_string, scores, 4);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace stringdistance */
} /* namespace keyvi */
