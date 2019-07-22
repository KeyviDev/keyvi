//
// keyvi - A key value store.
//
// Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#include <cstdio>

#include <boost/test/unit_test.hpp>

#include <msgpack.hpp>

#include "keyvi/index/internal/read_only_segment.h"
#include "keyvi/testing/temp_dictionary.h"

namespace keyvi {
namespace index {
namespace internal {

BOOST_AUTO_TEST_SUITE(ReadOnlySegmentTests)

BOOST_AUTO_TEST_CASE(deletedkey) {
  std::vector<std::pair<std::string, std::string>> test_data{
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"cde", "{c:2}"}, {"fgh", "{g:6}"}, {"tyc", "{o:2}"}};
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  std::vector<std::string> deleted_keys{"abc", "tyc"};
  std::vector<std::string> deleted_keys_dkm{"b", "o"};

  std::string filename{dictionary.GetFileName() + ".dk"};
  std::string filename_dkm{dictionary.GetFileName() + ".dkm"};

  {
    std::ofstream out_stream(filename, std::ios::binary);
    msgpack::pack(out_stream, deleted_keys);

    std::ofstream out_stream_dkm(filename_dkm, std::ios::binary);
    msgpack::pack(out_stream_dkm, deleted_keys_dkm);
  }

  read_only_segment_t segment(new ReadOnlySegment(dictionary.GetFileName()));

  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("b"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->IsDeleted("abc"));

  std::remove(filename_dkm.c_str());
  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE(deletedkeys_reload) {
  std::vector<std::pair<std::string, std::string>> test_data{
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"cde", "{c:2}"}, {"fgh", "{g:6}"}, {"tyc", "{o:2}"}};
  testing::TempDictionary dictionary = testing::TempDictionary::makeTempDictionaryFromJson(&test_data);

  read_only_segment_t segment(new ReadOnlySegment(dictionary.GetFileName()));

  BOOST_CHECK(!segment->HasDeletedKeys());
  BOOST_CHECK(!segment->DeletedKeys());

  std::string filename{dictionary.GetFileName() + ".dk"};
  std::string filename_dkm{dictionary.GetFileName() + ".dkm"};

  std::vector<std::string> deleted_keys{"abc", "tyc"};
  {
    std::ofstream out_stream(filename, std::ios::binary);
    msgpack::pack(out_stream, deleted_keys);
  }

  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(!segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->IsDeleted("abc"));

  std::vector<std::string> deleted_keys_2{"abc", "tyc", "o"};
  {
    std::ofstream out_stream(filename, std::ios::binary);
    msgpack::pack(out_stream, deleted_keys_2);
  }
  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->IsDeleted("abc"));

  std::vector<std::string> deleted_keys_dkm{"cde"};
  {
    std::ofstream out_stream(filename_dkm, std::ios::binary);
    msgpack::pack(out_stream, deleted_keys_dkm);
  }
  segment->ReloadDeletedKeys();
  BOOST_CHECK(segment->DeletedKeys()->count("abc"));
  BOOST_CHECK(segment->DeletedKeys()->count("tyc"));
  BOOST_CHECK(segment->DeletedKeys()->count("o"));
  BOOST_CHECK(segment->DeletedKeys()->count("cde"));
  BOOST_CHECK(segment->HasDeletedKeys());
  BOOST_CHECK(segment->IsDeleted("abc"));

  std::remove(filename_dkm.c_str());
  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace internal
}  // namespace index
}  // namespace keyvi
