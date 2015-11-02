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
 * entry_iterator.h
 *
 *  Created on: May 12, 2014
 *      Author: hendrik
 */

#ifndef ENTRY_ITERATOR_H_
#define ENTRY_ITERATOR_H_

#include "dictionary/fsa/automata.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

class EntryIterator final{

 public:
  EntryIterator()
      : current_state_(0),
        current_value_(0),
        fsa_(nullptr) {
  }

  EntryIterator(automata_t f) : EntryIterator(f, f->GetStartState()) {}

  EntryIterator(automata_t f, uint64_t start_state)
      : fsa_(f) {
    if (f && f->GetNumberOfKeys() > 0) {
      current_state_ = start_state;
      state_vector_traversal_stack_.resize(10);
      transitions_vector_traversal_stack_.resize(10);

      fsa_->GetOutGoingTransitions(start_state, state_vector_traversal_stack_[0], transitions_vector_traversal_stack_[0]);

      vector_offset_traversal_stack_.push_back(0);
      // If there are no transitions from the start state...
      if (transitions_vector_traversal_stack_[0].size() > 0) {
        traversal_stack_.push_back(transitions_vector_traversal_stack_[0][0]);
        TraverseToNextFinalState();
      } else {
        Clear();
      }
    } else {
      Clear();
    }
  }

  std::string GetKey() {
    TRACE ("depth: %d", current_depth_);
    return std::string((const char*) traversal_stack_.data(), current_depth_ + 1);
  }

  void WriteKey(std::ostream& stream){
    stream.write((const char*) traversal_stack_.data(), current_depth_ + 1);
  }

  uint64_t GetValueId() const {
    return current_value_;
  }

  internal::IValueStoreReader::attributes_t GetValueAsAttributeVector() const {
    return fsa_->GetValueAsAttributeVector(current_value_);
  }

  std::string GetValueAsString() const {
    return fsa_->GetValueAsString(current_value_);
  }

  EntryIterator &operator=(const EntryIterator &other) {
    fsa_ = other.fsa_;
    current_state_ = other.current_state_;
    current_value_ = other.current_value_;
    traversal_stack_ = other.traversal_stack_;
    state_vector_traversal_stack_ = other.state_vector_traversal_stack_;
    transitions_vector_traversal_stack_ = other.transitions_vector_traversal_stack_;
    vector_offset_traversal_stack_ = other.vector_offset_traversal_stack_;
    current_depth_ = other.current_depth_;
    return *this;
  }

  EntryIterator &operator++() {
    TraverseToNextFinalState();
    return *this;
  }

  bool operator==(const EntryIterator &other) const {
    return (current_state_ == other.current_state_);
  }

  bool operator!=(const EntryIterator &other) const {
    return !(operator==(other));
  }

 private:
  /** Cleares the iterator, i.e. makes it equal to the empty iterator. */
  void Clear() {
    fsa_ = nullptr;
    current_state_ = 0;
    current_state_ = 0;
  }

  void TraverseToNextFinalState() {

    if (current_state_ == 0) {
      return;
    }

    // clean the current state
    current_state_ = 0;
    current_value_ = 0;

    for (;;){

      current_depth_ = vector_offset_traversal_stack_.size() - 1;
      int current_offset = vector_offset_traversal_stack_[current_depth_];
      TRACE("current depth %d", current_depth_);
      // check whether the current state has outgoing transitions left
      if (current_offset < transitions_vector_traversal_stack_[current_depth_].size()) {
        traversal_stack_[traversal_stack_.size()-1] = transitions_vector_traversal_stack_[current_depth_][vector_offset_traversal_stack_[current_depth_]];

        // go further deep down
        size_t new_depth = current_depth_ + 1;

        // ensure that the traversal stack is large enough
        if (state_vector_traversal_stack_.size() < new_depth + 1){
          TRACE("resize vectors");

          state_vector_traversal_stack_.resize(state_vector_traversal_stack_.size() + 10);
          transitions_vector_traversal_stack_.resize(transitions_vector_traversal_stack_.size() + 10);
        }

        // get the outgoing transitions
        fsa_->GetOutGoingTransitions(state_vector_traversal_stack_[current_depth_][vector_offset_traversal_stack_[current_depth_]],
                                     state_vector_traversal_stack_[new_depth], transitions_vector_traversal_stack_[current_depth_+1]);

        TRACE("number of outgoing states found: %d", state_vector_traversal_stack_[current_depth_+1].size());

        if (fsa_->IsFinalState(state_vector_traversal_stack_[current_depth_][current_offset])) {
          // we found a final state
          TRACE("found final state");
          current_state_ = state_vector_traversal_stack_[current_depth_][current_offset];
          current_value_ = fsa_->GetStateValue(current_state_);
        }

        if (state_vector_traversal_stack_[new_depth].size() > 0) {

          TRACE("going 1 level deeper");

          vector_offset_traversal_stack_.push_back(0);
          traversal_stack_.push_back(transitions_vector_traversal_stack_[new_depth][0]);
        } else {
          TRACE("no more states");
          vector_offset_traversal_stack_[current_depth_]++;
        }

      } else {
        // go up
        TRACE("going 1 level up");

        vector_offset_traversal_stack_.pop_back();

        if (vector_offset_traversal_stack_.size() == 0) {
          return;
        }

        traversal_stack_.pop_back();
        --current_depth_;

        vector_offset_traversal_stack_[current_depth_]++;
      }

      // if current_state is set, we found a final state -> return
      if (current_state_ != 0){
        return;
      }
    }
  }

  automata_t fsa_;
  uint64_t current_state_;
  uint64_t current_value_;
  std::vector<unsigned char> traversal_stack_;
  std::vector<std::vector<uint64_t>> state_vector_traversal_stack_;
  std::vector<std::vector<unsigned char>> transitions_vector_traversal_stack_;
  std::vector<int> vector_offset_traversal_stack_;
  size_t current_depth_ = 0;
};

typedef std::shared_ptr<EntryIterator> entry_iterator_t;

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* ENTRY_ITERATOR_H_ */
