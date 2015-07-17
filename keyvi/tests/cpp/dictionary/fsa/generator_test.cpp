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
 * generator_test.cpp
 *
 *  Created on: May 8, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "dictionary/fsa/generator.h"
#include "dictionary/fsa/automata.h"
#include "dictionary/fsa/entry_iterator.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/fsa/internal/int_value_store.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE( GeneratorTests )

BOOST_AUTO_TEST_CASE( simple ) {
  internal::SparseArrayPersistence<> p(2048,
                                     boost::filesystem::temp_directory_path());

  Generator<internal::SparseArrayPersistence<>> g;
  g.Add("aaaa");
  g.Add("aabb");
  g.Add("aabc");
  g.Add("aacd");
  g.Add("bbcd");

  g.CloseFeeding();

  std::ofstream out_stream("testFile", std::ios::binary);
  g.Write(out_stream);
  out_stream.close();

  automata_t f(new Automata("testFile"));

  EntryIterator it(f);
  EntryIterator end_it;

  BOOST_CHECK_EQUAL("aaaa", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("aabb", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("aabc", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("aacd", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("bbcd", it.GetKey());
  ++it;
  BOOST_CHECK(it == end_it);
}

BOOST_AUTO_TEST_CASE( simple2 ) {
  internal::SparseArrayPersistence<> p(2048,
                                     boost::filesystem::temp_directory_path());

  Generator<internal::SparseArrayPersistence<>> g;
  g.Add("aaa");
  g.Add("abcde");
  g.Add("bar");
  g.Add("foo");
  g.Add("zar");

  g.CloseFeeding();

  std::ofstream out_stream("testFile2", std::ios::binary);
  g.Write(out_stream);
  out_stream.close();

  automata_t f(new Automata("testFile2"));
  EntryIterator it(f);
  EntryIterator end_it;

  BOOST_CHECK_EQUAL("aaa", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("abcde", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("bar", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("foo", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("zar", it.GetKey());
  ++it;
  BOOST_CHECK(it == end_it);
}

BOOST_AUTO_TEST_CASE( stringtest ) {
  Generator<internal::SparseArrayPersistence<>> g;
  g.Add(std::string("aaa"));
  g.Add(std::string("abcde"));
  g.Add("bar");
  g.Add("foo");
  g.Add("zar");

  g.CloseFeeding();

  std::ofstream out_stream("testFile3", std::ios::binary);
  g.Write(out_stream);
  out_stream.close();

  automata_t f(new Automata("testFile3"));
  EntryIterator it(f);
  EntryIterator end_it;

  BOOST_CHECK_EQUAL("aaa", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("abcde", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("bar", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("foo", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("zar", it.GetKey());
  ++it;
  BOOST_CHECK(it == end_it);
}

BOOST_AUTO_TEST_CASE( intvaluetest ) {
  internal::SparseArrayPersistence<> p(2048,
                                     boost::filesystem::temp_directory_path());

  Generator<internal::SparseArrayPersistence<>, internal::IntValueStoreWithInnerWeights> g;
  g.Add("eads", 576);

  g.Add("facebook", 4368451);
  g.Add("youtube", 2622207);

  g.CloseFeeding();

  std::ofstream out_stream("testFile4", std::ios::binary);
  g.Write(out_stream);
  out_stream.close();

  automata_t f(new Automata("testFile4"));
  EntryIterator it(f);
  EntryIterator end_it;

  BOOST_CHECK_EQUAL("eads", it.GetKey());
  BOOST_CHECK_EQUAL(576, it.GetValueId());

  ++it;
  BOOST_CHECK_EQUAL("facebook", it.GetKey());
  BOOST_CHECK_EQUAL(4368451, it.GetValueId());

  ++it;
  BOOST_CHECK_EQUAL("youtube", it.GetKey());
  BOOST_CHECK_EQUAL(2622207, it.GetValueId());
  ++it;

  BOOST_CHECK(it == end_it);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
