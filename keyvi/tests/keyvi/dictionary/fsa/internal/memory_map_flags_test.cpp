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
 * memory_map_flags_test.cpp
 *
 *  Created on: May 17, 2016
 *      Author: hendrik
 */

#include <boost/interprocess/mapped_region.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/memory_map_flags.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE(MemoryMapFlagsTests)

BOOST_AUTO_TEST_CASE(MemoryMapFlagsTestLazy) {
  loading_strategy_types strategy = loading_strategy_types::lazy;
  auto key_advise_flags = MemoryMapFlags::FSAGetMemoryMapAdvices(strategy);
  auto value_advise_flags = MemoryMapFlags::ValuesGetMemoryMapAdvices(strategy);

#if not defined(OS_MACOSX)
  int key_flags = MemoryMapFlags::FSAGetMemoryMapOptions(strategy);
  int value_flags = MemoryMapFlags::ValuesGetMemoryMapOptions(strategy);
  // no map populate
  BOOST_CHECK((key_flags & MAP_POPULATE) == 0);
  BOOST_CHECK((value_flags & MAP_POPULATE) == 0);
#endif

  BOOST_CHECK(key_advise_flags == boost::interprocess::mapped_region::advice_types::advice_normal);
  BOOST_CHECK(value_advise_flags == boost::interprocess::mapped_region::advice_types::advice_normal);
}

BOOST_AUTO_TEST_CASE(MemoryMapFlagsTestPopulate) {
  loading_strategy_types strategy = loading_strategy_types::populate;
  auto key_advise_flags = MemoryMapFlags::FSAGetMemoryMapAdvices(strategy);
  auto value_advise_flags = MemoryMapFlags::ValuesGetMemoryMapAdvices(strategy);

#if not defined(OS_MACOSX)
  int key_flags = MemoryMapFlags::FSAGetMemoryMapOptions(strategy);
  int value_flags = MemoryMapFlags::ValuesGetMemoryMapOptions(strategy);
  // map populate
  BOOST_CHECK((key_flags & MAP_POPULATE));
  BOOST_CHECK((value_flags & MAP_POPULATE));
#endif

  BOOST_CHECK(key_advise_flags == boost::interprocess::mapped_region::advice_types::advice_normal);
  BOOST_CHECK(value_advise_flags == boost::interprocess::mapped_region::advice_types::advice_normal);
}

BOOST_AUTO_TEST_CASE(MemoryMapFlagsTestPopulate_key_part) {
  loading_strategy_types strategy = loading_strategy_types::populate_key_part;
  auto key_advise_flags = MemoryMapFlags::FSAGetMemoryMapAdvices(strategy);
  auto value_advise_flags = MemoryMapFlags::ValuesGetMemoryMapAdvices(strategy);

#if not defined(OS_MACOSX)
  int key_flags = MemoryMapFlags::FSAGetMemoryMapOptions(strategy);
  int value_flags = MemoryMapFlags::ValuesGetMemoryMapOptions(strategy);
  // no map populate
  BOOST_CHECK((key_flags & MAP_POPULATE));
  BOOST_CHECK((value_flags & MAP_POPULATE) == 0);
#endif

  BOOST_CHECK(key_advise_flags == boost::interprocess::mapped_region::advice_types::advice_normal);
  BOOST_CHECK(value_advise_flags == boost::interprocess::mapped_region::advice_types::advice_normal);
}

BOOST_AUTO_TEST_CASE(MemoryMapFlagsTestPopulate_lazy) {
  loading_strategy_types strategy = loading_strategy_types::populate_lazy;
  auto key_advise_flags = MemoryMapFlags::FSAGetMemoryMapAdvices(strategy);
  auto value_advise_flags = MemoryMapFlags::ValuesGetMemoryMapAdvices(strategy);

#if not defined(OS_MACOSX)
  int key_flags = MemoryMapFlags::FSAGetMemoryMapOptions(strategy);
  int value_flags = MemoryMapFlags::ValuesGetMemoryMapOptions(strategy);
  // no map populate
  BOOST_CHECK((key_flags & MAP_POPULATE) == 0);
  BOOST_CHECK((value_flags & MAP_POPULATE) == 0);
#endif

  BOOST_CHECK(key_advise_flags == boost::interprocess::mapped_region::advice_types::advice_willneed);
  BOOST_CHECK(value_advise_flags == boost::interprocess::mapped_region::advice_types::advice_willneed);
}

BOOST_AUTO_TEST_CASE(MemoryMapFlagsTestlazy_no_readahead) {
  loading_strategy_types strategy = loading_strategy_types::lazy_no_readahead;
  auto key_advise_flags = MemoryMapFlags::FSAGetMemoryMapAdvices(strategy);
  auto value_advise_flags = MemoryMapFlags::ValuesGetMemoryMapAdvices(strategy);

#if not defined(OS_MACOSX)
  int key_flags = MemoryMapFlags::FSAGetMemoryMapOptions(strategy);
  int value_flags = MemoryMapFlags::ValuesGetMemoryMapOptions(strategy);
  // no map populate
  BOOST_CHECK((key_flags & MAP_POPULATE) == 0);
  BOOST_CHECK((value_flags & MAP_POPULATE) == 0);
#endif

  BOOST_CHECK(key_advise_flags == boost::interprocess::mapped_region::advice_types::advice_random);
  BOOST_CHECK(value_advise_flags == boost::interprocess::mapped_region::advice_types::advice_random);
}

BOOST_AUTO_TEST_CASE(MemoryMapFlagsTestlazy_no_readahead_value_part) {
  loading_strategy_types strategy = loading_strategy_types::lazy_no_readahead_value_part;
  auto key_advise_flags = MemoryMapFlags::FSAGetMemoryMapAdvices(strategy);
  auto value_advise_flags = MemoryMapFlags::ValuesGetMemoryMapAdvices(strategy);

#if not defined(OS_MACOSX)
  int key_flags = MemoryMapFlags::FSAGetMemoryMapOptions(strategy);
  int value_flags = MemoryMapFlags::ValuesGetMemoryMapOptions(strategy);
  // no map populate
  BOOST_CHECK((key_flags & MAP_POPULATE) == 0);
  BOOST_CHECK((value_flags & MAP_POPULATE) == 0);
#endif

  BOOST_CHECK(key_advise_flags == boost::interprocess::mapped_region::advice_types::advice_normal);
  BOOST_CHECK(value_advise_flags == boost::interprocess::mapped_region::advice_types::advice_random);
}

BOOST_AUTO_TEST_CASE(MemoryMapFlagsTestpopulate_key_part_no_readahead_value_part) {
  loading_strategy_types strategy = loading_strategy_types::populate_key_part_no_readahead_value_part;
  auto key_advise_flags = MemoryMapFlags::FSAGetMemoryMapAdvices(strategy);
  auto value_advise_flags = MemoryMapFlags::ValuesGetMemoryMapAdvices(strategy);

#if not defined(OS_MACOSX)
  int key_flags = MemoryMapFlags::FSAGetMemoryMapOptions(strategy);
  int value_flags = MemoryMapFlags::ValuesGetMemoryMapOptions(strategy);
  // no map populate
  BOOST_CHECK((key_flags & MAP_POPULATE));
  BOOST_CHECK((value_flags & MAP_POPULATE) == 0);
#endif

  BOOST_CHECK(key_advise_flags == boost::interprocess::mapped_region::advice_types::advice_normal);
  BOOST_CHECK(value_advise_flags == boost::interprocess::mapped_region::advice_types::advice_random);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
