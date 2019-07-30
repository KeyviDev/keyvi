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
 * lru_generation_cache_test.cpp
 *
 *  Created on: May 17, 2014
 *      Author: hendrik
 */

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/lru_generation_cache.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(LRUGenerationCacheTests)

BOOST_AUTO_TEST_CASE(generation) {
  LeastRecentlyUsedGenerationsCache<> cache{5, 3};

  PackedState<> p_empty{0, 0, 0};
  PackedState<> p1{1, 1, 1};
  PackedState<> p1_1{1, 1, 1};
  PackedState<> p2{2, 2, 1};
  PackedState<> p2_1{2, 2, 1};
  PackedState<> p3{3, 3, 1};
  PackedState<> p3_1{3, 3, 1};
  PackedState<> p4{4, 4, 1};
  PackedState<> p5{5, 5, 1};
  PackedState<> p6{6, 6, 1};
  PackedState<> p7{7, 7, 1};
  PackedState<> p8{8, 8, 1};
  PackedState<> p9{9, 9, 1};
  PackedState<> p9_1{9, 9, 1};
  PackedState<> p10{10, 10, 1};
  PackedState<> p11{11, 11, 1};
  PackedState<> p11_1{11, 11, 1};
  PackedState<> p12{12, 12, 1};
  PackedState<> p13{13, 13, 1};
  PackedState<> p14{14, 14, 1};
  PackedState<> p14_1{14, 14, 1};
  PackedState<> p15{15, 15, 1};
  PackedState<> p15_1{15, 15, 1};
  PackedState<> p16{16, 16, 1};
  PackedState<> p17{17, 17, 1};
  PackedState<> p18{18, 18, 1};
  PackedState<> p19{19, 19, 1};
  PackedState<> p20{20, 20, 1};
  PackedState<> p21{21, 21, 1};

  cache.Add(p1);
  cache.Add(p2);
  cache.Add(p3);
  cache.Add(p4);
  cache.Add(p5);
  cache.Add(p6);
  cache.Add(p7);
  cache.Add(p8);
  cache.Add(p9);
  cache.Add(p10);
  cache.Add(p11);
  cache.Add(p12);
  cache.Add(p13);
  cache.Add(p14);
  cache.Add(p15);

  BOOST_CHECK(p1 == cache.Get(p1_1));
  cache.Add(p16);
  cache.Add(p17);
  cache.Add(p18);
  cache.Add(p19);
  BOOST_CHECK(p1 == cache.Get(p1_1));
  BOOST_CHECK(p_empty == cache.Get(p2_1));
  BOOST_CHECK(p_empty == cache.Get(p3_1));
  cache.Add(p20);
  cache.Add(p21);
  BOOST_CHECK(p1 == cache.Get(p1_1));
  BOOST_CHECK(p_empty == cache.Get(p9_1));
  BOOST_CHECK(p11 == cache.Get(p11_1));
  BOOST_CHECK(p14 == cache.Get(p14_1));
  BOOST_CHECK(p15 == cache.Get(p15_1));
  BOOST_CHECK(p1 == cache.Get(p1_1));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
