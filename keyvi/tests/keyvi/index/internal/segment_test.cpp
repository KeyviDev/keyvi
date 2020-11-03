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

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <msgpack.hpp>

#include "keyvi/index/internal/segment.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace index {
namespace internal {

BOOST_AUTO_TEST_SUITE(SegmentTests)

void LoadDeletedKeys(const std::string& filename, std::vector<std::string>* deleted_keys) {
  std::ifstream deleted_keys_stream(filename, std::ios::binary);

  BOOST_CHECK(deleted_keys_stream.good());

  std::stringstream buffer;
  buffer << deleted_keys_stream.rdbuf();

  msgpack::object_handle unpacked_object;
  msgpack::unpack(unpacked_object, buffer.str().data(), buffer.str().size());

  unpacked_object.get().convert(*deleted_keys);
  std::sort(deleted_keys->begin(), deleted_keys->end());
}

BOOST_AUTO_TEST_CASE(deletekey) {
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"cde", "{c:2}"}, {"fgh", "{g:6}"}, {"tyc", "{o:2}"}};
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  // do with lazy load
  segment_t segment(new Segment(dictionary.GetFileName()));

  // delete a key
  segment->DeleteKey("abc");
  segment->Persist();

  std::vector<std::string> deleted_keys;
  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(1, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);

  // delete 2nd key
  segment->DeleteKey("tyc");
  segment->Persist();

  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(2, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);
  BOOST_CHECK_EQUAL("tyc", deleted_keys[1]);

  // delete key again
  segment->DeleteKey("abc");
  segment->Persist();
  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(2, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);
  BOOST_CHECK_EQUAL("tyc", deleted_keys[1]);

  // delete unknown key
  segment->DeleteKey("kkkk");
  segment->Persist();
  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(2, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);
  BOOST_CHECK_EQUAL("tyc", deleted_keys[1]);

  // delete unknown key
  segment->DeleteKey("fgh");
  segment->Persist();
  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(3, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);
  BOOST_CHECK_EQUAL("fgh", deleted_keys[1]);
  BOOST_CHECK_EQUAL("tyc", deleted_keys[2]);

  segment->RemoveFiles();
  BOOST_CHECK(!boost::filesystem::exists(dictionary.GetFileName() + ".dk"));
}

BOOST_AUTO_TEST_CASE(deletekeyDuringMerge) {
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"cde", "{c:2}"}, {"fgh", "{g:6}"}, {"tyc", "{o:2}"}};
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  segment_t segment(new Segment(dictionary.GetFileName()));

  // delete a key
  segment->DeleteKey("abc");
  segment->Persist();

  std::vector<std::string> deleted_keys;
  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(1, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);

  // mark segment for merge
  segment->ElectedForMerge();

  // delete 2nd key
  segment->DeleteKey("tyc");
  segment->Persist();

  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(1, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);

  // check 2nd in-merge list of deleted keys
  LoadDeletedKeys(dictionary.GetFileName() + ".dkm", &deleted_keys);

  BOOST_CHECK_EQUAL(1, deleted_keys.size());
  BOOST_CHECK_EQUAL("tyc", deleted_keys[0]);

  // simulate a merge failure
  segment->MergeFailed();

  // the "dkm" file should be gone
  BOOST_CHECK(!boost::filesystem::exists(dictionary.GetFileName() + ".dkm"));
  LoadDeletedKeys(dictionary.GetFileName() + ".dk", &deleted_keys);

  BOOST_CHECK_EQUAL(2, deleted_keys.size());
  BOOST_CHECK_EQUAL("abc", deleted_keys[0]);
  BOOST_CHECK_EQUAL("tyc", deleted_keys[1]);

  segment->RemoveFiles();
  BOOST_CHECK(!boost::filesystem::exists(dictionary.GetFileName() + ".dk"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary.GetFileName() + ".dkm"));
}

BOOST_AUTO_TEST_CASE(deletekeyMerging) {
  std::vector<std::pair<std::string, std::string>> test_data_segment1 = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"cde", "{c:2}"}, {"fgh", "{g:6}"}, {"tyc", "{o:2}"}};
  testing::TempDictionary dictionary1 = testing::TempDictionary::makeTempDictionaryFromJson(&test_data_segment1);

  segment_t segment1(new Segment(dictionary1.GetFileName()));

  // delete a key
  segment1->DeleteKey("abc");
  segment1->Persist();

  std::vector<std::pair<std::string, std::string>> test_data_segment2 = {
      {"efg", "{g:1}"}, {"hij", "{b:2}"}, {"lmn", "{c:2}"}, {"q", "{w:6}"}};
  testing::TempDictionary dictionary2 = testing::TempDictionary::makeTempDictionaryFromJson(&test_data_segment2);

  // mixin lazy load
  segment_t segment2(new Segment(dictionary2.GetFileName()));

  // simulate a merge operation
  segment1->ElectedForMerge();
  segment2->ElectedForMerge();

  // delete keys during a merge
  segment1->DeleteKey("cde");
  segment2->DeleteKey("lmn");

  std::vector<std::pair<std::string, std::string>> test_data_segment_merged = test_data_segment1;
  test_data_segment_merged.insert(test_data_segment_merged.end(), test_data_segment2.begin(), test_data_segment2.end());

  // erase "abc", because it has been deleted before merge and create the merged dictionary
  test_data_segment_merged.erase(test_data_segment_merged.begin());
  testing::TempDictionary dictionary3 = testing::TempDictionary::makeTempDictionaryFromJson(&test_data_segment_merged);

  segment_vec_t parent_segments{segment1, segment2};

  segment_t segment_merged(new Segment(dictionary3.GetFileName(), parent_segments));

  std::vector<std::string> deleted_keys;
  LoadDeletedKeys(dictionary3.GetFileName() + ".dk", &deleted_keys);
  BOOST_CHECK_EQUAL(2, deleted_keys.size());
  BOOST_CHECK_EQUAL("cde", deleted_keys[0]);
  BOOST_CHECK_EQUAL("lmn", deleted_keys[1]);

  segment1->RemoveFiles();
  BOOST_CHECK(!boost::filesystem::exists(dictionary1.GetFileName() + ".dk"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary1.GetFileName() + ".dkm"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary1.GetFileName()));

  segment2->RemoveFiles();
  BOOST_CHECK(!boost::filesystem::exists(dictionary2.GetFileName() + ".dk"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary2.GetFileName() + ".dkm"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary2.GetFileName()));

  segment_merged->RemoveFiles();
  BOOST_CHECK(!boost::filesystem::exists(dictionary3.GetFileName() + ".dk"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary3.GetFileName() + ".dkm"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary3.GetFileName()));
}

BOOST_AUTO_TEST_CASE(deletekeyendtoend) {
  std::vector<std::pair<std::string, std::string>> test_data{{"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"b", "{ytr:2}"},
                                                             {"cde", "{c:2}"}, {"fgh", "{g:6}"},  {"o", "{ee:2}"},
                                                             {"tyc", "{o:2}"}};
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  segment_t segment(new Segment(dictionary.GetFileName()));

  BOOST_CHECK(!segment->HasDeletedKeys());
  BOOST_CHECK(!segment->DeletedKeys());

  segment->DeleteKey("abc");
  segment->DeleteKey("tyc");
  segment->DeleteKey("not_in_dict");
  segment->Persist();

  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(!segment->DeletedKeys()->count("o"));
  BOOST_CHECK(!segment->DeletedKeys()->count("not_in_dict"));
  BOOST_CHECK(segment->IsDeleted("abc"));

  segment->DeleteKey("o");
  segment->Persist();

  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(!segment->DeletedKeys()->count("not_in_dict"));

  segment->ElectedForMerge();

  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(!segment->DeletedKeys()->count("not_in_dict"));

  segment->DeleteKey("b");
  segment->DeleteKey("not_in_dict");
  segment->Persist();

  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->DeletedKeys()->count("b"));
  BOOST_CHECK(!segment->DeletedKeys()->count("not_in_dict"));

  segment->MergeFailed();

  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->DeletedKeys()->count("b"));
  BOOST_CHECK(!segment->DeletedKeys()->count("not_in_dict"));

  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->DeletedKeys()->count("b"));
  BOOST_CHECK(!segment->DeletedKeys()->count("not_in_dict"));

  segment->ElectedForMerge();

  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->DeletedKeys()->count("b"));
  BOOST_CHECK(!segment->DeletedKeys()->count("not_in_dict"));

  segment->DeleteKey("fgh");
  segment->DeleteKey("not_in_dict");
  segment->Persist();

  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->DeletedKeys()->count("b"));
  BOOST_CHECK(segment->DeletedKeys()->count("fgh"));
  BOOST_CHECK(!segment->DeletedKeys()->count("not_in_dict"));

  segment->RemoveFiles();
  BOOST_CHECK(!boost::filesystem::exists(dictionary.GetFileName() + ".dk"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary.GetFileName() + ".dkm"));
  BOOST_CHECK(!boost::filesystem::exists(dictionary.GetFileName()));
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace internal
}  // namespace index
}  // namespace keyvi
