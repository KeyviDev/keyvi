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

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/minimization_hash.h"
#include "keyvi/dictionary/fsa/internal/packed_state.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(MinimizationHashTests)

BOOST_AUTO_TEST_CASE(insert) {
  MinimizationHash<PackedState<>> *hash = new MinimizationHash<PackedState<>>();
  PackedState<> p1 = {10, 25, 2};
  hash->Add(p1);
  PackedState<> p2 = {12, 25, 3};
  hash->Add(p2);
  PackedState<> p3 = {13, 25, 5};
  hash->Add(p3);
  PackedState<> p4 = {15, 25, 6};
  hash->Add(p4);

  BOOST_CHECK(hash->Get(p1) == p1);
  BOOST_CHECK(hash->Get(p2) == p2);
  BOOST_CHECK(hash->Get(p3) == p3);
  BOOST_CHECK(hash->Get(p4) == p4);

  delete hash;
}

BOOST_AUTO_TEST_CASE(reset) {
  MinimizationHash<PackedState<>> *hash = new MinimizationHash<PackedState<>>();
  PackedState<> p1 = {10, 25, 2};
  hash->Add(p1);
  PackedState<> p2 = {12, 25, 3};
  hash->Add(p2);

  BOOST_CHECK(hash->Get(p1) == p1);
  BOOST_CHECK(hash->Get(p2) == p2);

  hash->Reset();

  BOOST_CHECK(hash->Get(p1) == PackedState<>());
  BOOST_CHECK(hash->Get(p2) == PackedState<>());

  delete hash;
}

BOOST_AUTO_TEST_CASE(memory_usage) {
  // default
  MinimizationHash<PackedState<>> hash;
  BOOST_CHECK_EQUAL(hash.GetMemoryUsage(), (8171 + 2042) * sizeof(PackedState<>));

  // some non-default
  MinimizationHash<PackedState<>> hash2(5);
  BOOST_CHECK_EQUAL(hash2.GetMemoryUsage(), (32749 + 8187) * sizeof(PackedState<>));
}

BOOST_AUTO_TEST_CASE(memory_calculation) {
  MinimizationHash<PackedState<>> hash;

  auto memoryConfiguration = hash.FindMemoryLimitConfigurationForLRUCache(1000000, 3, 6);
  BOOST_CHECK(memoryConfiguration.best_fit_memory_limit < 1000000);
  BOOST_CHECK(memoryConfiguration.best_fit_generations <= 6);
  BOOST_CHECK(memoryConfiguration.best_fit_generations >= 3);

  // max case
  memoryConfiguration = hash.FindMemoryLimitConfigurationForLRUCache(std::numeric_limits<size_t>::max(), 3, 6);

  BOOST_CHECK(memoryConfiguration.best_fit_memory_limit > 0);
  BOOST_CHECK_EQUAL(6, memoryConfiguration.best_fit_generations);

  // min case
  BOOST_CHECK_THROW(hash.FindMemoryLimitConfigurationForLRUCache(0, 1, 1), std::invalid_argument);

  // another min case
  BOOST_CHECK_THROW(hash.FindMemoryLimitConfigurationForLRUCache(5, 10, 20), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
