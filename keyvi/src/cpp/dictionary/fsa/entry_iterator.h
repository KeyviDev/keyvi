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

#include <cstring>
#include "dictionary/fsa/automata.h"
#include "dictionary/fsa/traversal/traversal_base.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

class EntryIterator final{

 public:
  EntryIterator()
      : fsa_(nullptr),
        current_state_(0),
        current_value_(0) {
  }

  EntryIterator(automata_t f) : EntryIterator(f, f->GetStartState()) {}

  EntryIterator(automata_t f, uint64_t start_state)
      : fsa_(f),stack_() {
    if (f && f->GetNumberOfKeys() > 0) {
      current_state_ = start_state;
      traversal_stack_.reserve(50);
      f->GetOutGoingTransitions(start_state, stack_.GetStates(), stack_.traversal_stack_payload);

      TraverseToNextFinalState();
    } else {
      Clear();
    }
  }

  std::string GetKey() const {
    TRACE ("depth: %d", GetDepth());
    return std::string((const char*) traversal_stack_.data(), GetDepth());
  }

  void WriteKey(std::ostream& stream) const {
    stream.write((const char*) traversal_stack_.data(), GetDepth());
  }

  uint64_t GetValueId() const {
    return current_value_;
  }

  size_t GetDepth() const {
    return stack_.GetDepth();
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
    stack_ = other.stack_;
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

  bool operator==(const std::string& other_key) const {
    if (GetDepth() != other_key.size()) {
      return false;
    }

    return std::memcmp(other_key.c_str(), (const char*) traversal_stack_.data(), other_key.size()) == 0;
  }

  bool operator!=(const std::string& other_key) const {
    return !(operator==(other_key));
  }

  bool operator<(const EntryIterator& other) const {

    int compare_result = memcmp((const char*) other.traversal_stack_.data(),
                  (const char*) traversal_stack_.data(),
                  std::min(traversal_stack_.size(), other.traversal_stack_.size()));

    if (compare_result == 0) {
      return traversal_stack_.size() < other.traversal_stack_.size();
    }

    return compare_result > 0;
  }

  bool operator>(const EntryIterator& other) const {
    return !(operator<(other));
  }

  automata_t GetFsa() const {
    return fsa_;
  }

 private:
  /** Clears the iterator, i.e. makes it equal to the empty iterator. */
  void Clear() {
    fsa_ = nullptr;
    current_state_ = 0;
    current_value_ = 0;
  }

  void TraverseToNextFinalState() {

    if (current_state_ == 0) {
      return;
    }

    for (;;){
      current_state_ = stack_.GetStates().GetNextState();
      TRACE ("next state: %ld depth: %ld", current_state_, stack_.GetDepth());

      while (current_state_ == 0) {
        if (stack_.GetDepth() == 0) {
          TRACE("traverser exhausted.");
          Clear();
          return;
        }

        TRACE ("state is 0, go up");
        --stack_;
        traversal_stack_.pop_back();
        stack_.GetStates()++;
        current_state_ = stack_.GetStates().GetNextState();
        TRACE ("next state %ld depth %ld", current_state_, stack_.GetDepth());
      }

      traversal_stack_.push_back(stack_.GetStates().GetNextTransition());
      stack_++;
      fsa_->GetOutGoingTransitions(current_state_, stack_.GetStates(), stack_.traversal_stack_payload);

      TRACE("found %ld outgoing states", stack_.GetStates().size());

      if (fsa_->IsFinalState(current_state_)) {
        // we found a final state
        TRACE("found final state");
        current_value_ = fsa_->GetStateValue(current_state_);
        return;
      }

    }
  }

  automata_t fsa_;
  uint64_t current_state_;
  uint64_t current_value_;
  std::vector<unsigned char> traversal_stack_;

  traversal::TraversalStack<> stack_;
};

typedef std::shared_ptr<EntryIterator> entry_iterator_t;

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* ENTRY_ITERATOR_H_ */
