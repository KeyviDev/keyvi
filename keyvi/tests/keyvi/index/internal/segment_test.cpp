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

#include <algorithm>
#include <boost/test/unit_test.hpp>

#include <msgpack.hpp>

#include "index/internal/segment.h"
#include "testing/temp_dictionary.h"

namespace keyvi {
namespace index {
namespace internal {

BOOST_AUTO_TEST_SUITE(SegmentTests)

void LoadDeletedKeys(const std::string& filename, std::vector<std::string>* deleted_keys) {
  std::ifstream deleted_keys_stream(filename, std::ios::binary);

  BOOST_CHECK(deleted_keys_stream.good());

  std::stringstream buffer;
  buffer << deleted_keys_stream.rdbuf();

  msgpack::unpacked unpacked_object;
  msgpack::unpack(unpacked_object, buffer.str().data(), buffer.str().size());

  unpacked_object.get().convert(deleted_keys);
  std::sort(deleted_keys->begin(), deleted_keys->end());
}

BOOST_AUTO_TEST_CASE(deletekey) {
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"cde", "{c:2}"}, {"fgh", "{g:6}"}, {"tyc", "{o:2}"}};
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  segment_t segment(new Segment(dictionary.GetFileName()));

  segment->DeleteKey("abc");
  segment->Persist();

  std::vector<std::string> deleted_keys;
  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(1, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);

  segment->DeleteKey("tyc");
  segment->Persist();

  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(2, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);
  BOOST_CHECK_EQUAL("tyc", deleted_keys[1]);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace internal
}  // namespace index
}  // namespace keyvi
