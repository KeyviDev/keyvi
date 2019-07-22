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
 * tiered_merge_policy_test.cpp
 *
 *  Created on: Feb 14, 2019
 *      Author: hendrik
 */

#include <memory>

#include <boost/test/unit_test.hpp>

#include "keyvi/dictionary/dictionary_properties.h"
#include "keyvi/dictionary/dictionary_types.h"
#include "keyvi/index/internal/segment.h"
#include "keyvi/index/internal/tiered_merge_policy.h"

namespace keyvi {
namespace index {
namespace internal {
namespace unit_test {
class SegmentFriend {
 public:
  static segment_t CreateSegment(dictionary::dictionary_properties_t properties) {
    // due to the friend declaration we can not use make_shared in this place
    return std::shared_ptr<Segment>(new Segment(properties));
  }

  static void SetDeletedKeys(segment_t segment, std::unordered_set<std::string> keys) {
    segment->deleted_keys_for_write_ = keys;
    if (keys.size() > 0) {
      segment->deletes_loaded = true;
    }
  }
};
}  // namespace unit_test

BOOST_AUTO_TEST_SUITE(MergePolicySelectorTests)

static dictionary::dictionary_properties_t createDictionaryProperties(const uint64_t number_of_keys,
                                                                      const uint64_t start_state = 0) {
  return std::make_shared<dictionary::DictionaryProperties>(0, start_state, number_of_keys, 0,
                                                            dictionary::dictionary_type_t::KEY_ONLY, 0, 0, "");
}

BOOST_AUTO_TEST_CASE(one_segment) {
  TieredMergePolicy tiered_merge_policy;

  segments_t segments{std::make_shared<segment_vec_t>()};
  std::vector<segment_t> selected_segments;
  size_t id;

  segments->push_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(1000)));

  tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id);

  BOOST_CHECK(selected_segments.empty());

  unit_test::SegmentFriend::SetDeletedKeys((*segments)[0], std::unordered_set<std::string>{"key1"});

  tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id);

  BOOST_CHECK(!selected_segments.empty());
}

BOOST_AUTO_TEST_CASE(segment_mix) {
  TieredMergePolicy tiered_merge_policy;

  segments_t segments{std::make_shared<segment_vec_t>()};
  std::vector<segment_t> selected_segments;
  size_t id;

  segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(1000000, 1)));
  segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(1000001, 2)));
  segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(1000002, 3)));

  for (uint64_t i = 100; i < 120; ++i) {
    segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(100000 + i, i)));
  }

  for (uint64_t i = 200; i < 220; ++i) {
    segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(10000 + i, i)));
  }

  BOOST_CHECK(tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id));
  // expect that smaller 20 segments have been selected
  BOOST_CHECK_EQUAL(20, selected_segments.size());
  BOOST_CHECK_EQUAL(200, selected_segments[0]->GetDictionaryProperties()->GetStartState());

  // remove the last 5
  segments->pop_back();
  segments->pop_back();
  segments->pop_back();
  segments->pop_back();
  segments->pop_back();

  BOOST_CHECK(tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id));
  // expect that middle 20 segments have been selected
  BOOST_CHECK_EQUAL(20, selected_segments.size());
  BOOST_CHECK_EQUAL(100, selected_segments[0]->GetDictionaryProperties()->GetStartState());
}

BOOST_AUTO_TEST_CASE(segment_with_deletes) {
  TieredMergePolicy tiered_merge_policy;

  segments_t segments{std::make_shared<segment_vec_t>()};
  std::vector<segment_t> selected_segments;
  size_t id;

  // 2 identical sets of segments
  for (uint64_t i = 0; i < 20; ++i) {
    segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(100100 + i, i)));
  }

  for (uint64_t i = 100; i < 120; ++i) {
    segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(100000 + i, i)));
  }

  BOOST_CHECK(tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id));

  // expect that the 1st set gets selected
  BOOST_CHECK_EQUAL(20, selected_segments.size());
  BOOST_CHECK_EQUAL(0, selected_segments[0]->GetDictionaryProperties()->GetStartState());

  // add some deletes to the 2nd set
  for (uint64_t i = 20; i < 40; ++i) {
    unit_test::SegmentFriend::SetDeletedKeys((*segments)[i],
                                             std::unordered_set<std::string>{"key1", "key2", "key3", "key4", "key5"});
  }

  BOOST_CHECK(tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id));

  // expect that the 2nd set gets selected because it scores better with deletes
  BOOST_CHECK_EQUAL(20, selected_segments.size());
  BOOST_CHECK_EQUAL(100, selected_segments[0]->GetDictionaryProperties()->GetStartState());
}

BOOST_AUTO_TEST_CASE(adjacent_segment_mix) {
  TieredMergePolicy tiered_merge_policy;

  segments_t segments{std::make_shared<segment_vec_t>()};
  std::vector<segment_t> selected_segments;
  size_t id;

  for (uint64_t i = 100; i < 120; ++i) {
    segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(100000 + i, i)));
  }

  for (uint64_t i = 200; i < 220; ++i) {
    segments->emplace_back(unit_test::SegmentFriend::CreateSegment(createDictionaryProperties(10000 + i, i)));
  }

  BOOST_CHECK(tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id));
  // expect that smaller 20 segments have been selected
  BOOST_CHECK_EQUAL(20, selected_segments.size());
  BOOST_CHECK_EQUAL(200, selected_segments[0]->GetDictionaryProperties()->GetStartState());

  // mark some segments

  for (uint64_t i = 24; i < 27; ++i) {
    segments->operator[](i)->ElectedForMerge();
  }

  BOOST_CHECK(tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id));
  // expect that bigger 20 segments have been selected
  BOOST_CHECK_EQUAL(20, selected_segments.size());
  BOOST_CHECK_EQUAL(100, selected_segments[0]->GetDictionaryProperties()->GetStartState());

  for (uint64_t i = 0; i < 20; ++i) {
    segments->operator[](i)->ElectedForMerge();
  }

  BOOST_CHECK(tiered_merge_policy.SelectMergeSegments(segments, &selected_segments, &id));
  // expect that smaller 13 segments have been selected
  BOOST_CHECK_EQUAL(13, selected_segments.size());
  BOOST_CHECK_EQUAL(207, selected_segments[0]->GetDictionaryProperties()->GetStartState());
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace internal
}  // namespace index
}  // namespace keyvi
