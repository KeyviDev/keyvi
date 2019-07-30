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
 * needleman_wunsch_test.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: hendrik
 */

#include <cstdint>

#include <boost/test/unit_test.hpp>

#include "utf8.h"

#include "keyvi/stringdistance/costfunctions/damerau_levenshtein.h"
#include "keyvi/stringdistance/needleman_wunsch.h"

namespace keyvi {
namespace stringdistance {

BOOST_AUTO_TEST_SUITE(NeedlemanWunschTests)

void TestDistance(NeedlemanWunsch<costfunctions::Damerau_Levenshtein>* metric, std::string candidate,
                  std::vector<int32_t> intermediateScores, int32_t finalScore) {
  std::vector<uint32_t> codepoints;
  utf8::unchecked::utf8to32(candidate.begin(), candidate.end(), back_inserter(codepoints));

  for (size_t i = 0; i < codepoints.size(); ++i) {
    BOOST_CHECK_EQUAL(intermediateScores[i], metric->Put(codepoints[i], i));
  }

  // Assert.AreEqual(candidate, metric.Candidate);
  BOOST_CHECK_EQUAL(finalScore, metric->GetScore());
}

BOOST_AUTO_TEST_CASE(exact) {
  std::string input_string = "text";
  std::string comparison_string = "text";
  std::vector<uint32_t> codepoints;

  utf8::unchecked::utf8to32(input_string.begin(), input_string.end(), back_inserter(codepoints));

  NeedlemanWunsch<costfunctions::Damerau_Levenshtein> nw(codepoints, 20, 3);
  std::vector<int32_t> scores = {0, 0, 0, 0};

  TestDistance(&nw, comparison_string, scores, 0);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace stringdistance */
} /* namespace keyvi */
