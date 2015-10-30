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
 * unpacked_state.h
 *
 *  Created on: May 2, 2014
 *      Author: hendrik
 */

#ifndef UNPACKED_STATE_H_
#define UNPACKED_STATE_H_

#include "dictionary/fsa/internal/constants.h"
#include "dictionary/fsa/internal/packed_state.h"
#include "dictionary/fsa/internal/bit_vector.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/util/vint.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

template<class PersistenceT>
class UnpackedState
final {
   public:
    struct Transition
    final {
       public:
        void set(int label, uint64_t value) {
          this->label = label;
          this->value = value;
        }

        //private:
        int label;
        uint64_t value;
      };

      UnpackedState(PersistenceT* persistence)
          : persistence_(persistence) {
      }

      UnpackedState(UnpackedState&& other):
        outgoing_(std::move(other.outgoing_)),
        bitvector_(std::move(other.bitvector_)),
        persistence_(other.persistence_),
        used_(other.used_),
        hashcode_(other.hashcode_),
        no_minimization_counter_(other.no_minimization_counter_),
        weight_(other.weight_),
        final_(other.final_)
      {
        other.persistence_ = 0;
        other.used_ = 0;
        other.hashcode_ = 0;
        other.no_minimization_counter_ = 0;
        other.weight_ = 0;
        other.final_ = false;
      }

      UnpackedState& operator=(UnpackedState&& other) {
         outgoing_(std::move(other.outgoing_)),
         bitvector_ = std::move(other.bitvector_);
         persistence_ = other.persistence_;
         used_ = other.used_;
         hashcode_ = other.hashcode_;
         no_minimization_counter_ = other.no_minimization_counter_;
         weight_ = other.weight_;
         final_ = other.final_;

         other.persistence_ = 0;
         other.used_ = 0;
         other.hashcode_ = 0;
         other.no_minimization_counter_ = 0;
         other.weight_ = 0;
         other.final_ = false;

         return *this;
      }

      ~UnpackedState() {
      }

      bool IsFinal() const {
        return final_;
      }

      void Add(int transition_label, uint64_t transition_value) {
        TRACE("UnpackedState: Adding label %d to %d", transition_label, transition_value);

        outgoing_[used_++].set(transition_label, transition_value);
        bitvector_.Set(transition_label);
      }

      bool HasLabel(int transition_label) const {
        return bitvector_.Get(transition_label);
      }

      void AddFinalState(uint64_t transition_value);

      void SetTransitionValue(uint64_t transition_value){
        outgoing_[used_ - 1].value = transition_value;
      }

      void Clear() {
        used_ = 0;
        hashcode_ = -1;
        bitvector_.Clear();
        no_minimization_counter_ = 0;
        weight_ = 0;
        final_ = false;
      }

      const BitVector<MAX_TRANSITIONS_OF_A_STATE>& get_BitVector() const {
        return bitvector_;
      }

      size_t size() const {
        return used_;
      }

      void IncrementNoMinimizationCounter(int counter = 1) {
        no_minimization_counter_ += counter;
      }

      int GetNoMinimizationCounter() const {
        return no_minimization_counter_;
      }

      void UpdateWeightIfHigher(uint32_t weight);

      uint32_t GetWeight() const{
        return weight_;
      }

      inline int64_t GetHashcode() {
        if (hashcode_ == -1) {
          int64_t b;
          int64_t a = b = 0x9e3779b9;
          int64_t c = weight_ > 0 ? 1 : 0;
          int sz = used_;
          for (int i = 0; i < sz; ++i) {
            Transition t = outgoing_[i];
            a += t.label;
            b += t.value;
            if (i < sz - 1) {
              ++i;
              t = outgoing_[i];
              a += t.label << 16;
              b += t.value << 16;
            }

            // good old Bob Jenkins Hash
            a -= b;
            a -= c;
            a ^= c >> 13;
            b -= c;
            b -= a;
            b ^= a << 8;
            c -= a;
            c -= b;
            c ^= b >> 13;
            a -= b;
            a -= c;
            a ^= c >> 12;
            b -= c;
            b -= a;
            b ^= a << 16;
            c -= a;
            c -= b;
            c ^= b >> 5;
            a -= b;
            a -= c;
            a ^= c >> 3;
            b -= c;
            b -= a;
            b ^= a << 10;
            c -= a;
            c -= b;
            c ^= b >> 15;
          }

          hashcode_ = c;
        }

        return hashcode_;
      }

      template<class OffsetTypeT, class HashCodeTypeT>
      bool operator==(const PackedState<OffsetTypeT, HashCodeTypeT>& l) {

        // First filter - check if hash code and the number of transitions is the same
        if (l.GetHashcode() != static_cast<HashCodeTypeT>(GetHashcode())
            || l.GetNumberOfOutgoingTransitions() != used_) {
          return false;
        }

        // The number of transitions is the same. Verify that they also look the same.
        for (int i = 0; i < used_; ++i) {
          Transition t = outgoing_[i];
          int label = t.label;

          if (label < FINAL_OFFSET_TRANSITION) {
            // Is there a transition of this kind?
            if (persistence_->ReadTransitionLabel(l.GetOffset() + label) != label) {
              return false;
            }

            // Does this transition lead to the same target state?
            int target = persistence_->ReadTransitionValue(l.GetOffset() + label);
            target = persistence_->ResolveTransitionValue(l.GetOffset() + label, target);
            if (t.value != target) {
              return false;
            }
          } else  // (label == FINAL_OFFSET_TRANSITION)
          {
            if (persistence_->ReadTransitionLabel(l.GetOffset() + label) != FINAL_OFFSET_CODE) {
              return false;
            }

            // check if l has final info
            int value = persistence_->ReadFinalValue(l.GetOffset());

            if (t.value != value) {
              return false;
            }
          }
          //// This transition is ok.
        }

        // note: we do not compare the weight entry because if one state has weight and the other not the hashes differ

        // all checks succeeded, states must be equal.
        return true;
      }

      const Transition& operator[](int position) const {
        return outgoing_[position];
      }

     private:
      std::array<Transition, MAX_TRANSITIONS_OF_A_STATE> outgoing_;
      BitVector<MAX_TRANSITIONS_OF_A_STATE> bitvector_;
      PersistenceT* persistence_;

      int used_ = 0;
      int64_t hashcode_ = -1;
      int no_minimization_counter_ = 0;
      uint32_t weight_ = 0;
      bool final_ = false;
    };


template<>
inline void UnpackedState<SparseArrayPersistence<uint32_t>>::AddFinalState(uint64_t transition_value)
{
  TRACE("UnpackedState: Adding final state %d", transition_value);
  outgoing_[used_++].set(FINAL_OFFSET_TRANSITION, transition_value);
  bitvector_.Set(FINAL_OFFSET_TRANSITION);
  final_=true;
}

template<>
inline void UnpackedState<SparseArrayPersistence<uint16_t>>::AddFinalState(uint64_t transition_value)
{
  TRACE("UnpackedState: Adding final state %d", transition_value);

  outgoing_[used_++].set(FINAL_OFFSET_TRANSITION, transition_value);

  size_t vshort_size = util::getVarshortLength(transition_value);
  for (size_t i=0; i < vshort_size; ++i) {
    bitvector_.Set(FINAL_OFFSET_TRANSITION + i);
  }
  final_ = true;
}

template<>
inline void UnpackedState<SparseArrayPersistence<uint32_t>>::UpdateWeightIfHigher(uint32_t weight)
{
  if (weight > weight_){
    weight_ = weight;
    bitvector_.Set(INNER_WEIGHT_TRANSITION);
  }
}

template<>
inline void UnpackedState<SparseArrayPersistence<uint16_t>>::UpdateWeightIfHigher(uint32_t weight)
{
  if (weight > weight_){
    weight_ = weight;
    bitvector_.Set(INNER_WEIGHT_TRANSITION_COMPACT);
  }
}

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* UNPACKED_STATE_H_ */
