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
 * sparse_array_builder_test.cpp
 *
 *  Created on: May 7, 2014
 *      Author: hendrik
 */

#define SPARSE_ARRAY_BUILDER_UNIT_TEST

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "dictionary/fsa/internal/sparse_array_builder.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

BOOST_AUTO_TEST_SUITE( SparseArrayBuilderTests )

BOOST_AUTO_TEST_CASE( writeState ) {
  SparseArrayPersistence<uint32_t> p(2048, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint32_t>> b(limit, &p, false);
  UnpackedState<SparseArrayPersistence<uint32_t>> u1(&p);
  u1.Add(65, 100);
  u1.Add(66, 101);
  u1.Add(233, 102);

  // final state
  u1.Add(256, 1000);
  b.WriteState(10,u1);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(75), 65);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(75), 100);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(76), 66);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(76), 101);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(243), 233);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(243), 102);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(266), 1);
  BOOST_CHECK_EQUAL(p.ReadTransitionValue(266), 1000);
}

BOOST_AUTO_TEST_CASE( writeFinalStateCompact ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  b.WriteFinalTransition(25,55);
  BOOST_CHECK_EQUAL(55, p.ReadFinalValue(25));

  b.WriteFinalTransition(42,0);
  BOOST_CHECK_EQUAL(0, p.ReadFinalValue(42));

  b.WriteFinalTransition(2048,23);
  BOOST_CHECK_EQUAL(23, p.ReadFinalValue(2048));

}

BOOST_AUTO_TEST_CASE( writeTransitionAbsoluteMaxValue ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  // write a state with a large offset but a low pointer
  p.BeginNewState(1000000 - 65);
  b.WriteTransition(1000000, 65, 20);
  b.taken_positions_in_sparsearray_.Set(1000000);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000), 65);
  BOOST_CHECK_EQUAL(p.ResolveTransitionValue(1000000, p.ReadTransitionValue(1000000)), 20);
}

BOOST_AUTO_TEST_CASE( writeTransitionRelativeOverflow ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  // write a state with a large offset and a low pointer > short
  p.BeginNewState(1000001 - 65);
  b.WriteTransition(1000001, 65, 34000);
  b.taken_positions_in_sparsearray_.Set(1000001);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000001), 65);
  BOOST_CHECK_EQUAL(p.ResolveTransitionValue(1000001, p.ReadTransitionValue(1000001)), 34000);
}

BOOST_AUTO_TEST_CASE( writeTransitionRelativeOverflowZerobyteGhostState ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // write 1 state, starting at position 0
  UnpackedState<SparseArrayPersistence<uint16_t>> u1(&p);
  u1.Add(65, 100);
  u1.Add(66, 101);
  u1.Add(233, 102);

  b.WriteState(0,u1);

  // it should be allowed to put something at position 255
  BOOST_CHECK(!b.state_start_positions_.IsSet(0xff));
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(0xff), 0);
  // 2nd state, at position 255
  UnpackedState<SparseArrayPersistence<uint16_t>> u2(&p);
  u2.Add(65, 100);
  u2.Add(66, 101);
  u2.Add(233, 102);
  for (int i = 1; i < 255 + 65; ++i) {
    // mark transitions
    if (i == 255) {
      continue;
    }
    b.taken_positions_in_sparsearray_.Set(i);
  }

  b.FindFreeBucket(u2);
  b.WriteState(0xff,u2);

  // 0 + 255 -> 255 should not exist as it would mean u1 has a transition 255
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(255), 0xfe);
}

BOOST_AUTO_TEST_CASE( writeTransitionRelativeOverflowZerobyte ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(999999 - 67);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000), 0);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000001), 0);

  b.WriteTransition(999999, 67, 21);
  b.taken_positions_in_sparsearray_.Set(999999);
  b.WriteTransition(1000002, 70, 22);
  b.taken_positions_in_sparsearray_.Set(1000002);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000), 0);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000001), 0);

  // write a state with a large offset and a low pointer > short
  p.BeginNewState(1000512 - 65);
  b.WriteTransition(1000512, 65, 333333);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000512), 65);
  BOOST_CHECK_EQUAL(p.ResolveTransitionValue(1000512, p.ReadTransitionValue(1000512)), 333333);

  BOOST_CHECK(p.ReadTransitionLabel(1000000) != 0);
  BOOST_CHECK(p.ReadTransitionLabel(1000001) != 0);
}

BOOST_AUTO_TEST_CASE( writeTransitionRelativeOverflowZerobyte2 ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(1000000);

  // write a valid zero byte state
  b.WriteTransition(1000000, 0, 21);
  b.taken_positions_in_sparsearray_.Set(1000000);
  b.WriteTransition(1000003, 70, 22);
  b.taken_positions_in_sparsearray_.Set(1000003);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000), 0);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000001), 0);

  // write a state with a large offset and a low pointer > short
  p.BeginNewState(1000512 - 65);
  b.WriteTransition(1000512, 65, 333334);
  b.taken_positions_in_sparsearray_.Set(1000512);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000512), 65);
  BOOST_CHECK_EQUAL(p.ResolveTransitionValue(1000512, p.ReadTransitionValue(1000512)), 333334);

  // the zero byte state should not be overwritten
  BOOST_CHECK(p.ReadTransitionLabel(1000000) == 0);
  BOOST_CHECK(p.ReadTransitionLabel(1000001) != 0);
  BOOST_CHECK(p.ReadTransitionLabel(1000002) != 0);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000003), 70);
}

BOOST_AUTO_TEST_CASE( writeTransitionRelativeOverflowZerobyte3 ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(1000000);

  // mark some state beginnings that could lead to zombie states
  b.state_start_positions_.Set(1000001 - 0xff);
  b.state_start_positions_.Set(1000001 - 0xfe);
  b.state_start_positions_.Set(1000001 - 0xfd);
  b.state_start_positions_.Set(1000001 - 0xfc);
  b.state_start_positions_.Set(1000001 - 0xfb);
  b.state_start_positions_.Set(1000001 - 0xfa);

  // write a valid zero byte state
  b.WriteTransition(1000000, 0, 21);
  b.taken_positions_in_sparsearray_.Set(1000000);
  b.WriteTransition(1000003, 70, 22);
  b.taken_positions_in_sparsearray_.Set(1000003);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000), 0);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000001), 0);

  // write a state with a large offset and a low pointer > short
  p.BeginNewState(1000512 - 65);
  b.WriteTransition(1000512, 65, 333335);
  b.taken_positions_in_sparsearray_.Set(1000512);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000512), 65);
  BOOST_CHECK_EQUAL(p.ResolveTransitionValue(1000512, p.ReadTransitionValue(1000512)), 333335);

  // the zero byte state should not be overwritten
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000), 0);
  BOOST_CHECK(p.ReadTransitionLabel(1000001) != 0);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000001), 0xf9);
  BOOST_CHECK(p.ReadTransitionLabel(1000002) != 0);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000003), 70);
}

BOOST_AUTO_TEST_CASE( writeTransitionRelativeOverflowZerobyteEdgecase ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(1000000);

  for (int i = 0xff; i>1; i--) {
    // mark some state beginnings that could lead to zombie states
    b.state_start_positions_.Set(1000001 - i);
  }

  // write a valid zero byte state
  b.WriteTransition(1000000, 0, 21);
  b.taken_positions_in_sparsearray_.Set(1000000);
  b.WriteTransition(1000003, 70, 22);
  b.taken_positions_in_sparsearray_.Set(1000003);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000), 0);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000001), 0);

  // write a state with a large offset and a low pointer > short
  p.BeginNewState(1000512 - 65);
  b.WriteTransition(1000512, 65, 333336);
  b.taken_positions_in_sparsearray_.Set(1000512);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000512), 65);
  BOOST_CHECK_EQUAL(p.ResolveTransitionValue(1000512, p.ReadTransitionValue(1000512)), 333336);

  // the zero byte state should not be overwritten
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000), 0);

  BOOST_CHECK(b.state_start_positions_.IsSet(1000001 - 0xff));

  // if 1000002 has label 1 we would have a wrong final state
  BOOST_CHECK(p.ReadTransitionLabel(1000002) != 1);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000003), 70);
  BOOST_CHECK(p.ReadTransitionLabel(1000004) != 0);
  BOOST_CHECK(p.ReadTransitionLabel(1000005) != 0);
}

BOOST_AUTO_TEST_CASE( writeTransitionRelativeOverflowZerobyteEdgecaseStartPositions ) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(1000000);

  for (int i = 0; i < 1000; ++i) {
    // mark some state beginnings that could lead to zombie states
    b.state_start_positions_.Set(1000000 + i);

    // fill the labels, just for the purpose of checking it later
    b.WriteTransition(1000000 + i, 70, 21);
  }

  // write a state with a large offset and a low pointer > short
  p.BeginNewState(1001000 - 65);
  b.WriteTransition(1001000, 65, 333336);
  b.taken_positions_in_sparsearray_.Set(1001000);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1001000), 65);
  BOOST_CHECK_EQUAL(p.ResolveTransitionValue(1001000, p.ReadTransitionValue(1001000)), 333336);

  for (int i = 0; i < 1000; ++i) {
      // mark some state beginnings that could lead to zombie states
      b.state_start_positions_.Set(1000000 + i);
      BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000 + i), 70);
  }
}

BOOST_AUTO_TEST_CASE( writeTransitionZerobyteWeightCase) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(1000000);

  // write a weight state
  b.UpdateWeightIfNeeded(1000000, 42);
  b.taken_positions_in_sparsearray_.Set(1000000 + INNER_WEIGHT_TRANSITION_COMPACT);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000 + INNER_WEIGHT_TRANSITION_COMPACT), 0);
  BOOST_CHECK(b.state_start_positions_.IsSet(1000000 + INNER_WEIGHT_TRANSITION_COMPACT));
}

BOOST_AUTO_TEST_CASE( writeTransitionFinalStateTransition) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(1000000);

  // write a final state which requires an overflow to the next cell
  b.WriteFinalTransition(1000000, 1000000);
  b.taken_positions_in_sparsearray_.Set(1000000 + FINAL_OFFSET_TRANSITION);
  b.taken_positions_in_sparsearray_.Set(1000000 + FINAL_OFFSET_TRANSITION + 1);

  b.WriteTransition(1000003, 70, 22);
  b.taken_positions_in_sparsearray_.Set(1000003);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000 + FINAL_OFFSET_TRANSITION), 1);
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000 + FINAL_OFFSET_TRANSITION + 1), 2);
}



BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
