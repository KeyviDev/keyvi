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
 * unpacked_state_stack.h
 *
 *  Created on: May 5, 2014
 *      Author: hendrik
 */

#ifndef UNPACKED_STATE_STACK_H_
#define UNPACKED_STATE_STACK_H_

#include <vector>
#include "dictionary/fsa/internal/unpacked_state.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

template<class PersistenceT>
class UnpackedStateStack
final {
   public:
    UnpackedStateStack(PersistenceT* persistence, size_t initial_size, int weight_cut_off = 30)
        : persistence_(persistence), weight_cut_off_(weight_cut_off) {
      unpacked_state_pool_.reserve(initial_size);
    }

    ~UnpackedStateStack() {
      for (const auto state : unpacked_state_pool_) {
        delete state;
      }
    }

    UnpackedState<PersistenceT>* Get(size_t position) {
      while (position >= unpacked_state_pool_.size()) {
        unpacked_state_pool_.push_back(
            new UnpackedState<PersistenceT>(persistence_));
      }
      return unpacked_state_pool_[position];
    }

    void Insert(size_t pos, int transition_label, uint64_t transition_value) {
      UnpackedState<PersistenceT>* state = Get(pos);
      state->Add(transition_label, transition_value);
    }

    void InsertFinalState(size_t pos, uint64_t transition_value, bool no_minimization) {
      UnpackedState<PersistenceT>* state = Get(pos);
      state->AddFinalState(transition_value);

      // only do it explicit if no_minimization is true, it could be that the state is already marked, so we should
      // not overwrite it.
      if (no_minimization) {
          TRACE("Set no minimization based on value.");
          state->IncrementNoMinimizationCounter();
      }
    }

    void UpdateWeights(int start, int end, uint32_t weight) {
      if (start > weight_cut_off_) {
        return;
      }

      if (end > weight_cut_off_) {
        end = weight_cut_off_;
      }

      for (int i = start; i<end; ++i){
        UnpackedState<PersistenceT>* state = Get(i);
        state->UpdateWeightIfHigher(weight);
      }
    }

    void PushTransitionPointer(size_t pos, uint64_t transition_value,
                               int minimization_counter) {
      TRACE("UnpackedStateStack: push transition pointer at %d to %d", pos, transition_value );

      UnpackedState<PersistenceT>* state = Get(pos);
      state->SetTransitionValue(transition_value);
      state->IncrementNoMinimizationCounter(minimization_counter);
    }

    void Erase(size_t pos) {
      Get(pos)->Clear();
    }

   private:
    std::vector<UnpackedState<PersistenceT>*> unpacked_state_pool_;
    PersistenceT* persistence_;
    int weight_cut_off_;
  };

  } /* namespace internal */
  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* UNPACKED_STATE_STACK_H_ */
