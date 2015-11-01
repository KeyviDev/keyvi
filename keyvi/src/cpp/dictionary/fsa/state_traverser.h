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
 * state_traverser.h
 *
 *  Created on: Jun 3, 2014
 *      Author: hendrik
 */

#ifndef STATE_TRAVERSER_H_
#define STATE_TRAVERSER_H_

#include "dictionary/fsa/automata.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

class StateTraverser
final {
   public:
    StateTraverser(automata_t f) :StateTraverser (f, f->GetStartState()) {
    }

    StateTraverser(automata_t f, uint64_t start_state, bool advance = true)
        : fsa_(f),
          current_depth_(0),
          current_label_(0) {
      current_state_ = start_state;

      TRACE("StateTraverser starting with Start state %d", current_state_);
      traversal_stack_.push_back(0);
      if (advance){
        this->operator ++(0);
      }
    }

    StateTraverser() = delete;
    StateTraverser& operator=(StateTraverser const&) = delete;
    StateTraverser(const StateTraverser& that) = delete;

    StateTraverser(StateTraverser&& other)
        : fsa_(other.fsa_),
          current_depth_(other.current_depth_),
          current_label_(other.current_label_),
          current_state_(other.current_state_),
          traversal_stack_(std::move(other.traversal_stack_)),
          state_traversal_stack_(std::move(other.state_traversal_stack_)) {
      other.fsa_ = 0;
      other.current_state_ = 0;
      other.current_label_ = 0;
      other.current_depth_ = 0;
    }

    automata_t GetFsa() const {
      return fsa_;
    }

    bool IsFinalState() {
      return fsa_->IsFinalState(current_state_);
    }

    size_t GetDepth() {
      return current_depth_;
    }

    uint64_t GetStateValue() {
      return fsa_->GetStateValue(current_state_);
    }

    uint64_t GetStateId(){
      return current_state_;
    }

    internal::IValueStoreReader::attributes_t GetValueAsAttributeVector(){
          return fsa_->GetValueAsAttributeVector(GetStateValue());
    }

    void Prune() {
      traversal_stack_.pop_back();
      current_state_ = state_traversal_stack_.back();
      state_traversal_stack_.pop_back();
      --current_depth_;
    }

    void operator++(int) {

      // ignore cases where we are already at the end
      if (current_state_ == 0) {
        return;
      }

      for (;;) {
        uint64_t child_node = 0;

        do {
          ++traversal_stack_[current_depth_];
          child_node = fsa_->TryWalkTransition(
              current_state_, traversal_stack_[current_depth_]);

          if (traversal_stack_[current_depth_] == 255) {
            break;
          }
        } while (!child_node);

        if (child_node) {
          /* Found a valid child node
           * go one level down
           */
          current_label_ = traversal_stack_[current_depth_];
          ++current_depth_;
          state_traversal_stack_.push_back(current_state_);
          current_state_ = child_node;
          traversal_stack_.push_back(1);

          return;

        } else {
          // did not found any more transitions at this deep

          if (current_depth_) {
            /* we did not find any path at the current level (deep)
             * go one level up
             */
            traversal_stack_.pop_back();
            current_state_ = state_traversal_stack_.back();
            state_traversal_stack_.pop_back();
            --current_depth_;
          } else {
            // we are at the very end
            current_state_ = 0;
            current_depth_ = 0;
            current_label_ = 0;
            return;
          }
        }
      }
    }

    unsigned char GetStateLabel() {
      return current_label_;
    }

   private:
    automata_t fsa_;
    size_t current_depth_;
    unsigned char current_label_;
    uint64_t current_state_;
    std::vector<unsigned char> traversal_stack_;
    std::vector<uint64_t> state_traversal_stack_;
  };

  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* STATE_TRAVERSER_H_ */
