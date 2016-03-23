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
 * sparse_array_packer.h
 *
 *  Created on: May 2, 2014
 *      Author: hendrik
 */

#ifndef SPARSE_ARRAY_BUILDER_H_
#define SPARSE_ARRAY_BUILDER_H_

#include "dictionary/fsa/internal/constants.h"
#include "dictionary/fsa/internal/lru_generation_cache.h"
#include "dictionary/fsa/internal/minimization_hash.h"
#include "dictionary/fsa/internal/unpacked_state.h"
#include "dictionary/fsa/internal/sliding_window_bit_vector_position_tracker.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/util/vint.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

template<class PersistenceT, class OffsetTypeT = uint32_t, class HashCodeTypeT = int32_t>
class SparseArrayBuilder final {
 public:
  SparseArrayBuilder(size_t memory_limit, PersistenceT* persistence,
                     bool inner_weight, bool minimize = true){
    throw std::invalid_argument("unsupported");
  }
};

template<class OffsetTypeT, class HashCodeTypeT>
class SparseArrayBuilder<SparseArrayPersistence<uint16_t>, OffsetTypeT, HashCodeTypeT> final {
 public:
  SparseArrayBuilder(size_t memory_limit, SparseArrayPersistence<uint16_t>* persistence,
                     bool inner_weight, bool minimize = true)
      : number_of_states_(0),
        highest_persisted_state_(0),
        persistence_(persistence),
        inner_weight_(inner_weight),
        minimize_(minimize)
 {

    state_hashtable_ = new LeastRecentlyUsedGenerationsCache<PackedState<OffsetTypeT, HashCodeTypeT>>(memory_limit);
  }

  ~SparseArrayBuilder() {
    delete state_hashtable_;
  }

  SparseArrayBuilder() = delete;
  SparseArrayBuilder& operator=(SparseArrayBuilder const&) = delete;
  SparseArrayBuilder(const SparseArrayBuilder& that) = delete;

  OffsetTypeT PersistState(UnpackedState<SparseArrayPersistence<uint16_t>>& unpacked_state) {
    if (unpacked_state.GetNoMinimizationCounter() == 0) {
      // try to find a match of two equal states to minimize automata
      const PackedState<OffsetTypeT, HashCodeTypeT> existing = state_hashtable_->Get(unpacked_state);
      if (!existing.IsEmpty()) {
        // if we are hitting this line minimization succeeded
        TRACE("found minimization, equal state: %d this->weight %d", existing.GetOffset(), unpacked_state.GetWeight());
        OffsetTypeT offset = existing.GetOffset();
        if (unpacked_state.GetWeight() > 0) {
          UpdateWeightIfNeeded(offset, unpacked_state.GetWeight());
        }

        return offset;
      }
    } TRACE("no minimization found, write a new state");

    // minimization failed, all predecessors of this state will not be minimized, so stop trying
    unpacked_state.IncrementNoMinimizationCounter();
    OffsetTypeT offset = FindFreeBucket(unpacked_state);
    //TRACE("write at %d", offset);

    WriteState(offset, unpacked_state);
    ++number_of_states_;
    const PackedState<OffsetTypeT, HashCodeTypeT> packed_state(offset, static_cast<HashCodeTypeT>(unpacked_state.GetHashcode()) ,
                                   unpacked_state.size());

    // if minimization failed several time in a row while the minimization hash has decent amount of data,
    // do not push the state to the minimization hash to avoid unnecessary overhead
    if (minimize_ && (number_of_states_ < 1000000
        || unpacked_state.GetNoMinimizationCounter() < 8)) {
      state_hashtable_->Add(packed_state);
    }

    return offset;
  }

  // todo: this is not correct for compact mode!!!
  size_t GetSize() const {
    return (highest_persisted_state_ + MAX_TRANSITIONS_OF_A_STATE) * 5;
  }

  /**
   * Get the number of states in the FSA.
   * @return number of states created
   */
  uint64_t GetNumberOfStates() const {
    return number_of_states_;
  }

#ifndef SPARSE_ARRAY_BUILDER_UNIT_TEST
 private:
#endif
  uint64_t number_of_states_;
  uint64_t highest_persisted_state_;
  SparseArrayPersistence<uint16_t>* persistence_;
  bool inner_weight_;
  bool minimize_;
  LeastRecentlyUsedGenerationsCache<PackedState<OffsetTypeT, HashCodeTypeT>>* state_hashtable_;
  SlidingWindowBitArrayPositionTracker state_start_positions_;
  SlidingWindowBitArrayPositionTracker taken_positions_in_sparsearray_;

  OffsetTypeT FindFreeBucket(
      const UnpackedState<SparseArrayPersistence<uint16_t>>& unpacked_state) const {
    OffsetTypeT start_position =
        highest_persisted_state_ > SPARSE_ARRAY_SEARCH_OFFSET ?
            highest_persisted_state_ - SPARSE_ARRAY_SEARCH_OFFSET : 1;

    // further shift it taking the first outgoing transition and find the slot where it fits in
    start_position = taken_positions_in_sparsearray_.NextFreeSlot(
        start_position + unpacked_state[0].label) - unpacked_state[0].label;

    do {
      TRACE ("Find free position, probing %d", start_position);
      start_position = state_start_positions_.NextFreeSlot(start_position);

      if (unpacked_state.IsFinal()) {
        if (state_start_positions_.IsSet(
            start_position + NUMBER_OF_STATE_CODINGS)) {
          ++start_position;
          //TRACE ("clash with start positions, jump to next position");

          continue;
        }
      }

      int shift = taken_positions_in_sparsearray_.IsAvailable(
          unpacked_state.get_BitVector(), start_position);

      if (shift == 0) {

        // check for potential conflict with existing state which could become final if the current state has
        // a outgoing transition with label 1
        if (start_position > NUMBER_OF_STATE_CODINGS
            && unpacked_state.HasLabel(FINAL_OFFSET_CODE)
            && state_start_positions_.IsSet(
                start_position - NUMBER_OF_STATE_CODINGS)) {

          ++start_position;
          TRACE ("interference with other state, continue search");

          continue;
        }

        TRACE ("found slot at %d", start_position);
        return start_position;
      }

      TRACE ("state does not fit in, got shift of %d (statesize %d)", shift, unpacked_state
          .size());

      start_position += shift;
    } while (true);

    // not reachable
    return -1;
  }

  void WriteState(const OffsetTypeT offset,
                  const UnpackedState<SparseArrayPersistence<uint16_t>>& unpacked_state) {
    int i;
    int len = unpacked_state.size();
    uint32_t weight = unpacked_state.GetWeight();

    if (offset > highest_persisted_state_) {
      highest_persisted_state_ = offset;
    }

    // make sure no other state is placed at offset - 255, which could cause interference
    if ((unpacked_state[0].label == 1) && offset >= NUMBER_OF_STATE_CODINGS) {
      state_start_positions_.Set(offset - NUMBER_OF_STATE_CODINGS);
    }

    persistence_->BeginNewState(offset);

    //TRACE ("WriteState at offset %d, state size %d", offset, unpacked_state.size());

    // 1st pass: reserve the buckets in the sparse array
    taken_positions_in_sparsearray_.SetVector(unpacked_state.get_BitVector(),
                                              offset);

    if (unpacked_state.IsFinal()) {
      // Make sure no other state is placed at offset + 255, which could cause interference
      state_start_positions_.Set(offset + NUMBER_OF_STATE_CODINGS);
    }

//#define STATE_WRITING_DEBUG
#ifdef STATE_WRITING_DEBUG
    for (i = 0; i < len; ++i) {
      typename UnpackedState<SparseArrayPersistence<uint16_t>>::Transition e = unpacked_state[i];
      if (e.label < FINAL_OFFSET_TRANSITION) {
        if (!taken_positions_in_sparsearray_.IsSet(offset + e.label)) {
          std::cerr << "transition bit not set " << offset<< " " << e.label << std::endl;
          std::cerr << "last transition bit: " << unpacked_state[len-1].label << std::endl;
          std::cerr << "check bit vector: " << unpacked_state.get_BitVector().Get(e.label) << std::endl;

        }
      } else {
        if (e.label == FINAL_OFFSET_TRANSITION) {

          size_t vshort_size = util::getVarshortLength(e.value);
          for (size_t i=0; i < vshort_size; ++i) {
            if (!taken_positions_in_sparsearray_.IsSet(offset + FINAL_OFFSET_TRANSITION + i)) {
              std::cerr << "final state bit not set " << offset<< " " << i << " " << vshort_size << " " << e.value << std::endl;
            }
            //taken_positions_in_sparsearray_.Set(offset + FINAL_OFFSET_TRANSITION + i);
          }

          // Make sure no other state is placed at offset + 255, which could cause interference
          if (!state_start_positions_.IsSet(offset + NUMBER_OF_STATE_CODINGS)) {
            std::cerr << "state marker not set " << std::endl;

          }
        }
      }
    }
#endif
    // 2nd pass: write the actual values into the buckets

    // index 0 is reserved for control mechanisms in langs, chs start from 0
    for (i = 0; i < len; ++i) {
      typename UnpackedState<SparseArrayPersistence<uint16_t>>::Transition e = unpacked_state[i];
      if (e.label < FINAL_OFFSET_TRANSITION) {
        WriteTransition(offset + e.label, e.label, e.value);
      } else {
        if (e.label == FINAL_OFFSET_TRANSITION) {
          //WriteTransition(offset + FINAL_OFFSET_TRANSITION, FINAL_OFFSET_CODE,
          //                              e.value);

          WriteFinalTransition(offset, e.value);
          //TRACE("Write final marker at %d, value %d", offset, e.value);
        }
      }
    }

    if (weight) {
      //TRACE("Write inner weight at %d, value %d", offset, weight);
      // as all states have this, no need to code it specially
      UpdateWeightIfNeeded(offset, weight);
    }

    // no other state should start at this offset
    state_start_positions_.Set(offset);
  }

  inline void UpdateWeightIfNeeded(const size_t offset,
                                   const uint32_t weight)
  {
    TRACE("Check for Update Weight");
    auto n_weight =
        (weight < COMPACT_SIZE_INNER_WEIGHT_MAX_VALUE) ?
            weight : COMPACT_SIZE_INNER_WEIGHT_MAX_VALUE;

    if (persistence_->ReadTransitionValue(
        offset + INNER_WEIGHT_TRANSITION_COMPACT) < n_weight) {
      TRACE("Update weight from %d to %d", persistence_->ReadTransitionValue(offset + INNER_WEIGHT_TRANSITION_COMPACT), n_weight);
      persistence_->WriteTransition(offset + INNER_WEIGHT_TRANSITION_COMPACT, 0,
                                    n_weight);
      // it might be, that the slot is not taken yet
      taken_positions_in_sparsearray_.Set(
          offset + INNER_WEIGHT_TRANSITION_COMPACT);
    }
  }

  /**
   * Compact Encode for ushorts
   *
   * bit
   *  1             0: value fits into bits 1-16 (value <32768)     1: overflow encoding
   *  2             0: overflow encoding                            1: absolute value fits in bits 2-16 (value<16384)
   *
   * compact (0x): value is the difference of offset + 1024 - transitionPointer
   *
   * absolute compact (11): value is the absolute address coded in bits 2-16
   *
   * overflow: (10)
   *
   * bits 3-12 pointer to extra bucket in the range -512 -> +511 from transitionPointer
   * bit 13 whether pointer is absolute(0) or relative(1)
   * bits 14-16 lower part (3 bits) of absolute value coded in extra bucket
   * extra bucket: variable length encoded absolute address of transition Pointer, higher bits
   *
   */
  inline void WriteTransition(size_t offset, unsigned char transitionId,
                              uint64_t transitionPointer)
  {
    size_t difference = SIZE_MAX;

    if (offset + 512 > transitionPointer) {
      difference = offset + 512 - transitionPointer;
    }

    if (difference < COMPACT_SIZE_RELATIVE_MAX_VALUE) {

      ushort diff_as_short = static_cast<ushort>(difference);

      TRACE("Transition fits in uint16: %d->%d (%d)", offset, transitionPointer, diff_as_short);

      persistence_->WriteTransition(offset, transitionId, diff_as_short);
      return;
    }

    if (transitionPointer < COMPACT_SIZE_ABSOLUTE_MAX_VALUE) {
      ushort absolute_compact_coding = static_cast<ushort>(transitionPointer)
          | 0xC000;
      persistence_->WriteTransition(offset, transitionId,
                                    absolute_compact_coding);
      return;
    }

    TRACE("Transition requires overflow %d->%d", offset, transitionPointer);

    // pt to overflow bucket with various other codings
    // set first bit to indicate overflow
    ushort pt_to_overflow_bucket = 0x8000;

    size_t overflow_code = transitionPointer;

    if (difference < transitionPointer) {
      // do relative coding
      // set corresponding bit
      pt_to_overflow_bucket |= 0x8;
      overflow_code = difference;
    }

    auto transitionPointer_low = overflow_code & 0x7;  // get the lower part
    auto transitionPointer_high = overflow_code >> 3;  // the higher part

    // else overflow encoding
    uint16_t vshort_pointer[8];
    size_t vshort_size = 0;

    util::encodeVarshort(transitionPointer_high, vshort_pointer, &vshort_size);

    // find free spots in the sparse array where the pointer fits in
    uint64_t start_position = offset > 512 ? offset - 512 : 0;

    for (;;) {
      start_position = taken_positions_in_sparsearray_.NextFreeSlot(
          start_position);

      // prevent that states without a weight get a 'zombie weight'.
      // check that we do not write into a bucket that is used for an inner weight of another transition
      if (inner_weight_
          && state_start_positions_.IsSet(
              start_position + INNER_WEIGHT_TRANSITION_COMPACT)) {
        //TRACE("found clash wrt. weight transition, skipping %d", start_position);

        start_position += 1;
        continue;
      }

      if (taken_positions_in_sparsearray_.IsSet(start_position)) {
        TRACE("Start position taken at %d",start_position);
        start_position += 1;
        continue;
      }

      size_t found_slots = 1;
      for (; found_slots < vshort_size; found_slots++) {

        if (taken_positions_in_sparsearray_.IsSet(start_position + found_slots)) {
          start_position += found_slots + 1;
          found_slots = 0;
          break;
        }
        // check that we do not write into a bucket that is used for an inner weight of another transition
        if (inner_weight_
            && state_start_positions_.IsSet(
                start_position + found_slots - INNER_WEIGHT_TRANSITION_COMPACT)) {
          TRACE("found clash wrt. weight transition, skipping %d", start_position);

          start_position += found_slots + 1;
          found_slots = 0;
          break;
        }
      }

      if (found_slots == vshort_size) {
        break;
      }
    }

    TRACE("Write Overflow transition at %d, length %d", start_position, vshort_size);

    persistence_->WriteRawValue(start_position, &vshort_pointer,
                                vshort_size * sizeof(uint16_t));
    // write the values
    for (auto i = 0; i < vshort_size; ++i) {
      taken_positions_in_sparsearray_.Set(start_position + i);
    }

    // encode the pointer to that bucket
    auto overflow_bucket = (512 + start_position) - offset;
    pt_to_overflow_bucket |= overflow_bucket << 4;

    // add the lower part (4 bits)
    pt_to_overflow_bucket += transitionPointer_low;

    persistence_->WriteTransition(offset, transitionId, pt_to_overflow_bucket);
  }


  inline void WriteFinalTransition(size_t offset, uint64_t value)
  {
    persistence_->WriteTransition(offset + FINAL_OFFSET_TRANSITION,
                                  FINAL_OFFSET_CODE, 0);

    uint16_t vshort_pointer[8];
    size_t vshort_size = 0;

    util::encodeVarshort(value, vshort_pointer, &vshort_size);

    persistence_->WriteRawValue(offset + FINAL_OFFSET_TRANSITION, &vshort_pointer,
                                vshort_size * sizeof(uint16_t));
  }

};


/**
 * DEPRECATED: Specialization for the 32bit based persistence, will be removed sooner or later!
 */

template<class OffsetTypeT, class HashCodeTypeT>
class SparseArrayBuilder<SparseArrayPersistence<uint32_t>, OffsetTypeT, HashCodeTypeT> final {
 public:
  SparseArrayBuilder(size_t memory_limit, SparseArrayPersistence<uint32_t>* persistence,
                     bool inner_weight, bool minimize = true)
      : number_of_states_(0),
        highest_persisted_state_(0),
        persistence_(persistence),
        inner_weight_(inner_weight),
        minimize_(minimize){

    state_hashtable_ = new LeastRecentlyUsedGenerationsCache<PackedState<OffsetTypeT, HashCodeTypeT>>(memory_limit);
  }

  ~SparseArrayBuilder() {
    delete state_hashtable_;
  }

  SparseArrayBuilder() = delete;
  SparseArrayBuilder& operator=(SparseArrayBuilder const&) = delete;
  SparseArrayBuilder(const SparseArrayBuilder& that) = delete;

  OffsetTypeT PersistState(UnpackedState<SparseArrayPersistence<uint32_t>>& unpacked_state) {
    if (unpacked_state.GetNoMinimizationCounter() == 0) {
      // try to find a match of two equal states to minimize automata
      const PackedState<OffsetTypeT, HashCodeTypeT> existing = state_hashtable_->Get(unpacked_state);
      if (!existing.IsEmpty()) {
        // if we are hitting this line minimization succeeded
        TRACE("found minimization, equal state: %d this->weight %d", existing.GetOffset(), unpacked_state.GetWeight());
        OffsetTypeT offset = existing.GetOffset();
        if (unpacked_state.GetWeight() > 0) {
          UpdateWeightIfNeeded(offset, unpacked_state.GetWeight());
        }

        return offset;
      }
    } TRACE("no minimization found, write a new state");

    // minimization failed, all predecessors of this state will not be minimized, so stop trying
    unpacked_state.IncrementNoMinimizationCounter();
    OffsetTypeT offset = FindFreeBucket(unpacked_state);
    //TRACE("write at %d", offset);

    WriteState(offset, unpacked_state);
    ++number_of_states_;
    const PackedState<OffsetTypeT, HashCodeTypeT> packed_state(offset, static_cast<HashCodeTypeT>(unpacked_state.GetHashcode()) ,
                                   unpacked_state.size());

    // if minimization failed several time in a row while the minimization hash has decent amount of data,
    // do not push the state to the minimization hash to avoid unnecessary overhead
    if (minimize_ && (number_of_states_ < 1000000
        || unpacked_state.GetNoMinimizationCounter() < 8)) {
      state_hashtable_->Add(packed_state);
    }

    return offset;
  }

  // todo: this is not correct for compact mode!!!
  size_t GetSize() const {
    return (highest_persisted_state_ + MAX_TRANSITIONS_OF_A_STATE) * 5;
  }

  /**
   * Get the number of states in the FSA.
   * @return number of states created
   */
  uint64_t GetNumberOfStates() const {
    return number_of_states_;
  }

#ifndef SPARSE_ARRAY_BUILDER_UNIT_TEST
 private:
#endif
  uint64_t number_of_states_;
  uint64_t highest_persisted_state_;
  SparseArrayPersistence<uint32_t>* persistence_;
  bool inner_weight_;
  bool minimize_;
  LeastRecentlyUsedGenerationsCache<PackedState<OffsetTypeT, HashCodeTypeT>>* state_hashtable_;
  SlidingWindowBitArrayPositionTracker state_start_positions_;
  SlidingWindowBitArrayPositionTracker taken_positions_in_sparsearray_;

  OffsetTypeT FindFreeBucket(
      const UnpackedState<SparseArrayPersistence<uint32_t>>& unpacked_state) const {
    OffsetTypeT start_position =
        highest_persisted_state_ > SPARSE_ARRAY_SEARCH_OFFSET ?
            highest_persisted_state_ - SPARSE_ARRAY_SEARCH_OFFSET : 1;

    // further shift it taking the first outgoing transition and find the slot where it fits in
    start_position = taken_positions_in_sparsearray_.NextFreeSlot(
        start_position + unpacked_state[0].label) - unpacked_state[0].label;

    do {
      TRACE ("Find free position, probing %d", start_position);
      start_position = state_start_positions_.NextFreeSlot(start_position);

      if (unpacked_state.IsFinal()) {
        if (state_start_positions_.IsSet(
            start_position + NUMBER_OF_STATE_CODINGS)) {
          ++start_position;
          //TRACE ("clash with start positions, jump to next position");

          continue;
        }
      }

      int shift = taken_positions_in_sparsearray_.IsAvailable(
          unpacked_state.get_BitVector(), start_position);

      if (shift == 0) {

        // check for potential conflict with existing state which could become final if the current state has
        // a outgoing transition with label 1
        if (start_position > NUMBER_OF_STATE_CODINGS
            && unpacked_state.HasLabel(FINAL_OFFSET_CODE)
            && state_start_positions_.IsSet(
                start_position - NUMBER_OF_STATE_CODINGS)) {

          ++start_position;
          TRACE ("interference with other state, continue search");

          continue;
        }

        TRACE ("found slot at %d", start_position);
        return start_position;
      }

      TRACE ("state does not fit in, got shift of %d (statesize %d)", shift, unpacked_state
          .size());

      start_position += shift;
    } while (true);

    // not reachable
    return -1;
  }

  void WriteState(const OffsetTypeT offset,
                  const UnpackedState<SparseArrayPersistence<uint32_t>>& unpacked_state) {
    int i;
    int len = unpacked_state.size();
    uint32_t weight = unpacked_state.GetWeight();

    if (offset > highest_persisted_state_) {
      highest_persisted_state_ = offset;
    }

    // make sure no other state is placed at offset - 255, which could cause interference
    if ((unpacked_state[0].label == 1) && offset >= NUMBER_OF_STATE_CODINGS) {
      state_start_positions_.Set(offset - NUMBER_OF_STATE_CODINGS);
    }

    persistence_->BeginNewState(offset);

    //TRACE ("WriteState at offset %d, state size %d", offset, unpacked_state.size());

    // 1st pass: reserve the buckets in the sparse array
    taken_positions_in_sparsearray_.SetVector(unpacked_state.get_BitVector(),
                                              offset);

    if (unpacked_state.IsFinal()) {
      // Make sure no other state is placed at offset + 255, which could cause interference
      state_start_positions_.Set(offset + NUMBER_OF_STATE_CODINGS);
    }

//#define STATE_WRITING_DEBUG
#ifdef STATE_WRITING_DEBUG
    for (i = 0; i < len; ++i) {
      typename UnpackedState<SparseArrayPersistence<uint32_t>>::Transition e = unpacked_state[i];
      if (e.label < FINAL_OFFSET_TRANSITION) {
        if (!taken_positions_in_sparsearray_.IsSet(offset + e.label)) {
          std::cerr << "transition bit not set " << offset<< " " << e.label << std::endl;
          std::cerr << "last transition bit: " << unpacked_state[len-1].label << std::endl;
          std::cerr << "check bit vector: " << unpacked_state.get_BitVector().Get(e.label) << std::endl;

        }
      } else {
        if (e.label == FINAL_OFFSET_TRANSITION) {

          size_t vshort_size = util::getVarshortLength(e.value);
          for (size_t i=0; i < vshort_size; ++i) {
            if (!taken_positions_in_sparsearray_.IsSet(offset + FINAL_OFFSET_TRANSITION + i)) {
              std::cerr << "final state bit not set " << offset<< " " << i << " " << vshort_size << " " << e.value << std::endl;
            }
            //taken_positions_in_sparsearray_.Set(offset + FINAL_OFFSET_TRANSITION + i);
          }

          // Make sure no other state is placed at offset + 255, which could cause interference
          if (!state_start_positions_.IsSet(offset + NUMBER_OF_STATE_CODINGS)) {
            std::cerr << "state marker not set " << std::endl;

          }
        }
      }
    }
#endif
    // 2nd pass: write the actual values into the buckets

    // index 0 is reserved for control mechanisms in langs, chs start from 0
    for (i = 0; i < len; ++i) {
      typename UnpackedState<SparseArrayPersistence<uint32_t>>::Transition e = unpacked_state[i];
      if (e.label < FINAL_OFFSET_TRANSITION) {
        WriteTransition(offset + e.label, e.label, e.value);
      } else {
        if (e.label == FINAL_OFFSET_TRANSITION) {
          //WriteTransition(offset + FINAL_OFFSET_TRANSITION, FINAL_OFFSET_CODE,
          //                              e.value);

          WriteFinalTransition(offset, e.value);
          //TRACE("Write final marker at %d, value %d", offset, e.value);
        }
      }
    }

    if (weight) {
      //TRACE("Write inner weight at %d, value %d", offset, weight);
      // as all states have this, no need to code it specially
      UpdateWeightIfNeeded(offset, weight);
    }

    // no other state should start at this offset
    state_start_positions_.Set(offset);
  }

  inline void UpdateWeightIfNeeded(const size_t offset,
                                   const uint32_t weight)
  {
    TRACE("Check for Update Weight");
    if (persistence_->ReadTransitionValue(offset + INNER_WEIGHT_TRANSITION)
        < weight) {
      TRACE("Update weight from %d to %d", persistence_->ReadTransitionValue(offset + INNER_WEIGHT_TRANSITION), weight);
      WriteTransition(offset + INNER_WEIGHT_TRANSITION,
                      0 /*no INNER_WEIGHT_CODE*/, weight);
      // it might be, that the slot is not taken yet
      taken_positions_in_sparsearray_.Set(offset + INNER_WEIGHT_TRANSITION);
    }
  }

  inline void WriteTransition(size_t offset, unsigned char transitionId,
                              uint64_t transitionPointer)
  {
    uint32_t transition_pointer_u32 = static_cast<uint32_t>(transitionPointer);

    persistence_->WriteTransition(offset, transitionId, transition_pointer_u32);
  }

  inline void WriteFinalTransition(size_t offset, uint64_t value){
    WriteTransition(offset + FINAL_OFFSET_TRANSITION, FINAL_OFFSET_CODE, value);
  }

};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* SPARSE_ARRAY_BUILDER_H_ */
