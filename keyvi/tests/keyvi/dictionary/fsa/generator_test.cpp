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

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/entry_iterator.h"
#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/internal/int_value_store.h"
#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/util/configuration.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE(GeneratorTests)

BOOST_AUTO_TEST_CASE(simple) {
  internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

  Generator<internal::SparseArrayPersistence<>> g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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

BOOST_AUTO_TEST_CASE(simple2) {
  internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

  Generator<internal::SparseArrayPersistence<>> g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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

BOOST_AUTO_TEST_CASE(stringtest) {
  Generator<internal::SparseArrayPersistence<>> g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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

BOOST_AUTO_TEST_CASE(intvaluetest) {
  internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

  Generator<internal::SparseArrayPersistence<>, internal::IntInnerWeightsValueStore> g(
      keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
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

BOOST_AUTO_TEST_CASE(feedwithoutclose) {
  // test that just triggers the case (if) generato is created but FSA creation is not finalized

  auto g = new Generator<internal::SparseArrayPersistence<>, internal::IntInnerWeightsValueStore>(
      keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
  g->Add("eads", 576);

  g->Add("facebook", 4368451);
  g->Add("youtube", 2622207);
  delete g;
}

BOOST_AUTO_TEST_CASE(manifesttest) {
  Generator<internal::SparseArrayPersistence<>> g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
  g.Add(std::string("aaa"));
  g.Add(std::string("abcde"));
  g.Add("bar");
  g.Add("foo");
  g.Add("zar");

  g.CloseFeeding();

  g.SetManifest("{\"version\":\"42\"}");

  std::ofstream out_stream("testFile3", std::ios::binary);
  g.Write(out_stream);
  out_stream.close();

  automata_t f(new Automata("testFile3"));
  BOOST_CHECK_EQUAL("{\"version\":\"42\"}", f->GetManifest());
}

BOOST_AUTO_TEST_CASE(zeroBytes) {
  internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

  TRACE("test zerobyte");
  Generator<internal::SparseArrayPersistence<>> g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));
  g.Add(std::string("\0bbcd", 5));
  g.Add(std::string("a\0abc", 5));
  g.Add("aaaa");
  g.Add(std::string("aabb\0", 5));
  g.Add("aacd");
  g.Add("bbcd");

  g.CloseFeeding();

  std::ofstream out_stream("testFileZB", std::ios::binary);
  g.Write(out_stream);
  out_stream.close();

  TRACE("test zerobyte: load FSA");
  automata_t f(new Automata("testFileZB"));

  auto zero_state_walk = f->TryWalkTransition(f->GetStartState(), 0);
  BOOST_CHECK(zero_state_walk != 0);
  TRACE("test zerobyte: tested zb FSA");
  EntryIterator it(f);
  EntryIterator end_it;

  BOOST_CHECK_EQUAL(std::string("\0bbcd", 5), it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL(std::string("a\0abc", 5), it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("aaaa", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL(std::string("aabb\0", 5), it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("aacd", it.GetKey());
  ++it;
  BOOST_CHECK_EQUAL("bbcd", it.GetKey());
  ++it;
  BOOST_CHECK(it == end_it);
}

BOOST_AUTO_TEST_CASE(state_exception_handling) {
  internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

  Generator<internal::SparseArrayPersistence<>> g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

  BOOST_CHECK_THROW(g.WriteToFile("somefile"), generator_exception);

  g.Add("abcd");
  BOOST_CHECK_THROW(g.WriteToFile("somefile"), generator_exception);
  g.CloseFeeding();
  BOOST_CHECK_THROW(g.CloseFeeding(), generator_exception);

  BOOST_CHECK_THROW(g.Add("cdef"), generator_exception);

  ValueHandle handle;
  BOOST_CHECK_THROW(g.Add("ghij", handle), generator_exception);
}

BOOST_AUTO_TEST_CASE(value_handle) {
  ValueHandle handle = {0, 0, false, false};
  ValueHandle handle2 = {1, 0, false, false};
  ValueHandle handle3 = {0, 0, false, false};
  ValueHandle handle4 = {0, 1, false, false};
  ValueHandle handle5 = {0, 0, true, false};
  ValueHandle handle6 = {0, 0, false, true};

  BOOST_CHECK(handle == handle);
  BOOST_CHECK(handle != handle2);
  BOOST_CHECK(handle == handle3);
  BOOST_CHECK(handle != handle4);
  BOOST_CHECK(handle != handle5);
  BOOST_CHECK(handle != handle6);
  BOOST_CHECK(handle6 == handle6);
  BOOST_CHECK(handle3 != handle4);
  BOOST_CHECK(handle5 != handle6);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
