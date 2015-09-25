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
 * dictionary_test.cpp
 *
 *  Created on: Jun 7, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "dictionary/fsa/generator.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/dictionary.h"
#include "dictionary/testing/temp_dictionary.h"
namespace keyvi {
namespace dictionary {
BOOST_AUTO_TEST_SUITE( DictionaryTests )

BOOST_AUTO_TEST_CASE( loadDict ) {

  fsa::internal::SparseArrayPersistence<> p(
      2048, boost::filesystem::temp_directory_path());

  fsa::Generator<fsa::internal::SparseArrayPersistence<>> g;
  g.Add("aaaa");
  g.Add("aabb");
  g.Add("aabc");
  g.Add("aacd");
  g.Add("bbcd");

  g.CloseFeeding();

  std::ofstream out_stream("testFile_completion", std::ios::binary);
  g.Write(out_stream);
  out_stream.close();

  fsa::automata_t f(new fsa::Automata("testFile_completion"));
  dictionary_t d(new Dictionary(f));
}

BOOST_AUTO_TEST_CASE( DictGet ) {
  std::vector<std::pair<std::string, uint32_t>> test_data = { { "test", 22 }, {
      "otherkey", 24 }, { "other", 444 }, { "bar", 200 }, };

  testing::TempDictionary dictionary(test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  bool matched = false;
  for (auto m : d->Get("test")){
      BOOST_CHECK_EQUAL("test", m.GetMatchedString());
      BOOST_CHECK_EQUAL(std::string("22"), boost::get<std::string>(m.GetAttribute("weight")));
      matched=true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Get("test2")){
      matched=true;
  }

  BOOST_CHECK(!matched);

  auto m = (*d)["test"];
  BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
}

BOOST_AUTO_TEST_CASE( DictLookup ) {
  std::vector<std::pair<std::string, uint32_t>> test_data = { { "nude", 22 }, {
      "nude-party", 24 }, };

  testing::TempDictionary dictionary(test_data);
  dictionary_t d(new Dictionary(dictionary.GetFsa()));

  bool matched = false;
  for (auto m : d->Lookup("nude")){
      BOOST_CHECK_EQUAL("nude", m.GetMatchedString());
      BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
      matched=true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Lookup("nude ")){
      BOOST_CHECK_EQUAL("nude", m.GetMatchedString());
      BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
      matched=true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Lookup("nude at work")){
      BOOST_CHECK_EQUAL("nude", m.GetMatchedString());
      BOOST_CHECK_EQUAL("22", boost::get<std::string>(m.GetAttribute("weight")));
      matched=true;
  }
  BOOST_CHECK(matched);

  matched = false;
  for (auto m : d->Lookup("nudelsalat")){
      matched=true;
  }
  BOOST_CHECK(!matched);
}
#define haszero(v) (((v) - 0x0101010101010101ULL) & ~(v) & 0x8080808080808080ULL)
BOOST_AUTO_TEST_CASE( DictTraverse ) {

  unsigned long int magic_bits = -1;
  magic_bits = magic_bits / 0xff * 0xfe << 1 >> 1 | 1;

  unsigned long int charmask=4702111234474983745;

  std::cout << "foo:" << magic_bits << std::endl;
  unsigned long int t= charmask + magic_bits;
  std::cout << "foo2:" << t << std::endl;

  std::string a = "abcdefgh";
  std::string b = "axydtfgr";

  unsigned long int* ptr1 =  (unsigned long int *) a.c_str();
  unsigned long int* ptr2 =  (unsigned long int *) b.c_str();

  unsigned long int xoring = *ptr1^*ptr2;
  std::cout << "foo3:" << xoring << std::endl;
  std::cout << "foo4:" << haszero(xoring) << std::endl;


}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace dictionary */
} /* namespace keyvi */
