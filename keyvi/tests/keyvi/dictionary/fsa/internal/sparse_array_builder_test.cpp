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

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/fsa/internal/sparse_array_builder.h"
#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

BOOST_AUTO_TEST_SUITE(SparseArrayBuilderTests)

BOOST_AUTO_TEST_CASE(writeFinalStateCompact) {
  SparseArrayPersistence<uint16_t> p(16000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  b.WriteFinalTransition(25, 55);
  BOOST_CHECK_EQUAL(55, p.ReadFinalValue(25));

  b.WriteFinalTransition(42, 0);
  BOOST_CHECK_EQUAL(0, p.ReadFinalValue(42));

  b.WriteFinalTransition(2048, 23);
  BOOST_CHECK_EQUAL(23, p.ReadFinalValue(2048));
}

BOOST_AUTO_TEST_CASE(writeTransitionAbsoluteMaxValue) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
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

BOOST_AUTO_TEST_CASE(writeTransitionRelativeOverflow) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
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

BOOST_AUTO_TEST_CASE(writeTransitionRelativeOverflowZerobyteGhostState) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // write 1 state, starting at position 0
  UnpackedState<SparseArrayPersistence<uint16_t>> u1(&p);
  u1.Add(65, 100);
  u1.Add(66, 101);
  u1.Add(233, 102);

  b.WriteState(0, u1);

  // it should be allowed to put something at position 255
  BOOST_CHECK(!b.state_start_positions_.IsSet(0xff));
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(0xff), 0);
  // 2nd state, at position 255
  UnpackedState<SparseArrayPersistence<uint16_t>> u2(&p);
  u2.Add(65, 100);
  u2.Add(66, 101);
  u2.Add(233, 102);
  for (size_t i = 1; i < 255 + 65; ++i) {
    // mark transitions
    if (i == 255) {
      continue;
    }
    b.taken_positions_in_sparsearray_.Set(i);
  }

  BOOST_CHECK_EQUAL(255, b.FindFreeBucket(&u2));
  b.WriteState(0xff, u2);

  // 0 + 255 -> 255 should not exist as it would mean u1 has a transition 255
  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(255), 0xfe);
}

BOOST_AUTO_TEST_CASE(writeTransitionRelativeOverflowZerobyte) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
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

BOOST_AUTO_TEST_CASE(writeTransitionRelativeOverflowZerobyte2) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
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

BOOST_AUTO_TEST_CASE(writeTransitionRelativeOverflowZerobyte3) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
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

BOOST_AUTO_TEST_CASE(writeTransitionRelativeOverflowZerobyteEdgecase) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(1000000);

  for (size_t i = 0xff; i > 1; i--) {
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

BOOST_AUTO_TEST_CASE(writeTransitionRelativeOverflowZerobyteEdgecaseStartPositions) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
  int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  p.BeginNewState(1000000);

  for (size_t i = 0; i < 1000; ++i) {
    // mark some state beginnings that could lead to zombie states
    b.state_start_positions_.Set(1000000 + i);

    // fill the labels, just for the purpose of checking it later
    b.WriteTransition(1000000 + i, 70, 21);
  }

  // write a state with a large offset and a large pointer that does not fit in a short and requires overflow
  p.BeginNewState(1001000 - 65);
  b.WriteTransition(1001000, 65, 333336);
  b.taken_positions_in_sparsearray_.Set(1001000);

  BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1001000), 65);
  BOOST_CHECK_EQUAL(p.ResolveTransitionValue(1001000, p.ReadTransitionValue(1001000)), 333336);

  for (size_t i = 0; i < 1000; ++i) {
    BOOST_CHECK_EQUAL(p.ReadTransitionLabel(1000000 + i), 70);
  }
}

BOOST_AUTO_TEST_CASE(writeTransitionZerobyteWeightCase) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
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

BOOST_AUTO_TEST_CASE(writeTransitionFinalStateTransition) {
  SparseArrayPersistence<uint16_t> p(64000, boost::filesystem::temp_directory_path());
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

BOOST_AUTO_TEST_CASE(writeTransitionExternalMemory) {
  const size_t memory_limit_persistence = 64000;
  SparseArrayPersistence<uint16_t> p(memory_limit_persistence, boost::filesystem::temp_directory_path());
  const int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  const size_t chunk_size = p.GetChunkSizeExternalTransitions();
  const size_t factor = (1024 * 1024) / memory_limit_persistence;

  const size_t offset = (factor * chunk_size) - 2;

  p.BeginNewState(offset - 100);

  // write a transition on the chunk border with a overflowing transition
  b.WriteTransition(offset - 20, 20, offset - 80000);
  b.taken_positions_in_sparsearray_.Set(offset - 20);

  // force flushing buffers
  p.BeginNewState(chunk_size * (factor + 2));

  const uint16_t val = p.ReadTransitionValue(offset - 20);

  BOOST_CHECK_EQUAL(offset - 80000, p.ResolveTransitionValue(offset - 20, val));
}

BOOST_AUTO_TEST_CASE(writeTransitionChunkborder) {
  const size_t memory_limit_persistence = 64000;
  SparseArrayPersistence<uint16_t> p(memory_limit_persistence, boost::filesystem::temp_directory_path());
  const int64_t limit = 1024 * 1024;
  SparseArrayBuilder<SparseArrayPersistence<uint16_t>> b(limit, &p, false);

  // simulate that sparse array builder got tons of states
  b.highest_persisted_state_ = 1024 * 1024;

  // find some setting to setup write on a chunk border
  const size_t chunk_size = p.GetChunkSizeExternalTransitions();
  const size_t factor = (1024 * 1024) / memory_limit_persistence;

  const size_t offset = (factor * chunk_size) - 2;

  // mark slots taken in sparse array to force writing on chunk border
  for (size_t i = offset - COMPACT_SIZE_WINDOW - 10; i <= offset - 1; ++i) {
    b.taken_positions_in_sparsearray_.Set(i);
  }

  p.BeginNewState(offset - 5);

  // write a transition on the chunk border with a overflowing transition
  b.WriteTransition(offset - 3, 5, offset - 80000);
  b.taken_positions_in_sparsearray_.Set(offset - 3);

  // force flushing buffers
  p.BeginNewState(chunk_size * (factor + 2));

  const uint16_t val = p.ReadTransitionValue(offset - 3);

  BOOST_CHECK_EQUAL(offset - 80000, p.ResolveTransitionValue(offset - 3, val));
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
