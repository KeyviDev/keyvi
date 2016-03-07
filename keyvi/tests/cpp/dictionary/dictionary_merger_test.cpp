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
 * dictionary_merger_test.cpp
 *
 *  Created on: Feb 29, 2016
 *      Author: hendrik
 */


#include <boost/test/unit_test.hpp>

#include "dictionary/dictionary_merger.h"
#include "dictionary/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {

BOOST_AUTO_TEST_SUITE( DictionaryMergerTests )

BOOST_AUTO_TEST_CASE ( MergeKeyOnlyDicts) {
  std::vector<std::string> test_data = {"aaaa", "aabb", "aabc", "aacd", "bbcd", "aaceh", "cdefgh"};
  testing::TempDictionary dictionary (test_data);

  std::vector<std::string> test_data2 = {"aaaaz", "aabbe", "cdddefgh"};
  testing::TempDictionary dictionary2 (test_data2);

  DictionaryMerger<fsa::internal::SparseArrayPersistence<>> merger;
  merger.Add(dictionary.GetFileName());
  merger.Add(dictionary2.GetFileName());

  merger.Merge("merged-dict.kv");
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */




