/* * keyvi - A key value store.
 *
 * Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * sliding_window_bit_vector_position_tracker.h
 *
 *  Created on: May 3, 2014
 *      Author: hendrik
 */

#ifndef SLIDING_WINDOW_BIT_VECTOR_POSITION_TRACKER_H_
#define SLIDING_WINDOW_BIT_VECTOR_POSITION_TRACKER_H_

#include <utility>
#include "dictionary/fsa/internal/constants.h"
#include "dictionary/fsa/internal/bit_vector.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/*
 *
 */
class SlidingWindowBitArrayPositionTracker final {
 public:
  SlidingWindowBitArrayPositionTracker(): window_start_position_(0), current_vector_(), previous_vector_() {}

  SlidingWindowBitArrayPositionTracker(SlidingWindowBitArrayPositionTracker&& other):
    current_vector_(std::move(other.current_vector_)),
    previous_vector_(std::move(other.previous_vector_))
  {
    window_start_position_ = other.window_start_position_;
  }

  SlidingWindowBitArrayPositionTracker& operator=(const SlidingWindowBitArrayPositionTracker&& other) {
    current_vector_ = std::move(other.current_vector_);
    previous_vector_ = std::move(other.previous_vector_);
    window_start_position_ = other.window_start_position_;
    return *this;
  }


  inline bool IsSet(size_t position) const {
    // divide by 1024
    size_t blocker_window = position >> 10;

    size_t blocker_offset = position & 1023;

    if (blocker_window == window_start_position_) {
      return current_vector_.Get(blocker_offset);
    }

    if (blocker_window > window_start_position_) {
      return false;
    }

    return previous_vector_.Get(blocker_offset);
  }

  inline size_t NextFreeSlot(size_t position) const {
    // divide by 1024
    size_t blocker_window = position >> 10;

    size_t blocker_offset = position & 1023;

    if (blocker_window > window_start_position_) {
      return position;
    }

    if (blocker_window < window_start_position_) {
      size_t offset = previous_vector_.GetNextNonSetBit(blocker_offset);
      if (offset < 1024) {
        // offset + (blockerWindow * 1024)
        return offset + (blocker_window << 10);
      }

      // else: check currentVector
      ++blocker_window;
      blocker_offset = 0;
    }

    return current_vector_.GetNextNonSetBit(blocker_offset) + (blocker_window << 10);
  }

  inline void Set(size_t position) {
    // divide by 1024
    size_t blocker_window = position >> 10;

    size_t blocker_offset = position & 1023;

    if (blocker_window > window_start_position_) {
      // swap and reset
      std::swap(previous_vector_,current_vector_);

      current_vector_.Clear();
      window_start_position_ = blocker_window;

      TRACE ("Sliding Window slide:   new start position %d", window_start_position_);
    }

    if (blocker_window == window_start_position_) {
      current_vector_.Set(blocker_offset);
    } else if (window_start_position_ > 0 && blocker_window == window_start_position_ - 1 ) {
      previous_vector_.Set(blocker_offset);
    }
  }

  template<std::size_t TsizeOther>
  inline void SetVector(const BitVector<TsizeOther>& requested_positions, size_t position) {
    // divide by 1024
    auto blocker_window = position >> 10;
    auto blocker_window_end = (requested_positions.Size() + position) >> 10;
    auto blocker_offset = position & 1023;

    // check if start position is already over the boundary now
    if (blocker_window_end > window_start_position_) {
      // swap and reset
      std::swap(previous_vector_,current_vector_);

      current_vector_.Clear();
      window_start_position_ = blocker_window_end;

      TRACE ("Sliding Window slide:   new start position %d", window_start_position_);
    }

    if (blocker_window == window_start_position_) {
      current_vector_.SetVector(requested_positions, blocker_offset);
    } else if (window_start_position_ > 0 && blocker_window == window_start_position_ - 1 ) {
      previous_vector_.SetVector(requested_positions, blocker_offset);
      if (blocker_window_end == window_start_position_) {
        current_vector_.SetVectorAndShiftOther(requested_positions, 1024 - blocker_offset);
      }
    }

  }

  template<std::size_t TsizeOther>
  inline int IsAvailable(const BitVector<TsizeOther>& requested_positions, size_t position) const {
    // divide by 1024
    size_t blocker_window = position >> 10;

    size_t blocker_offset = position & 1023;

    TRACE("Sliding Window IsAvailable for window %d(current: %d) offset %d position %d", blocker_window
          , window_start_position_, blocker_offset, position);

    if (blocker_window == window_start_position_) {
      return current_vector_.DisjointAndShiftThis(requested_positions,
                                                blocker_offset);
    }

    if (blocker_window > window_start_position_) {
      return 0;
    }

    size_t shift = previous_vector_.DisjointAndShiftThis(requested_positions,
                                                    blocker_offset);

    if (shift == 0 && (1024 - blocker_offset < MAX_TRANSITIONS_OF_A_STATE)) {
      return requested_positions.DisjointAndShiftOther(current_vector_,
                                                      1024 - blocker_offset);
    }

    return shift;
  }

 private:
  size_t window_start_position_ = 0;

  BitVector<1024> current_vector_;
  BitVector<1024> previous_vector_;
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* SLIDING_WINDOW_BIT_VECTOR_POSITION_TRACKER_H_ */
