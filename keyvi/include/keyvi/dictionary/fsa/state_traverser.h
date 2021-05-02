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

#ifndef KEYVI_DICTIONARY_FSA_STATE_TRAVERSER_H_
#define KEYVI_DICTIONARY_FSA_STATE_TRAVERSER_H_

#include <utility>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/traversal/traversal_base.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

template <class innerTraverserType>
class ComparableStateTraverser;

template <class TransitionT = traversal::Transition>
class StateTraverser final {
 public:
  using label_t = unsigned char;
  using transition_t = TransitionT;

  explicit StateTraverser(automata_t f)
      : fsa_(f), current_state_(f->GetStartState()), current_weight_(0), current_label_(0), at_end_(false), stack_() {
    TRACE("StateTraverser starting with Start state %d", current_state_);
    f->GetOutGoingTransitions(current_state_, &stack_.GetStates(), &stack_.traversal_stack_payload);

    this->operator++(0);
  }

  StateTraverser(automata_t f, const uint64_t start_state, traversal::TraversalPayload<TransitionT> &&payload,
                 const bool advance = true)
      : fsa_(f), current_weight_(0), current_label_(0), at_end_(false), stack_(std::move(payload)) {
    current_state_ = start_state;

    TRACE("StateTraverser starting with Start state %d", current_state_);
    f->GetOutGoingTransitions(start_state, &stack_.GetStates(), &stack_.traversal_stack_payload);

    if (advance) {
      this->operator++(0);
    }
  }

  StateTraverser(automata_t f, const uint64_t start_state, const bool advance = true)
      : fsa_(f), current_state_(start_state), current_weight_(0), current_label_(0), at_end_(false), stack_() {
    TRACE("StateTraverser starting with Start state %d", current_state_);
    f->GetOutGoingTransitions(start_state, &stack_.GetStates(), &stack_.traversal_stack_payload);

    if (advance) {
      this->operator++(0);
    }
  }

  StateTraverser() = delete;
  StateTraverser &operator=(StateTraverser const &) = delete;
  StateTraverser(const StateTraverser &that) = delete;

  StateTraverser(StateTraverser &&other)
      : fsa_(other.fsa_),
        current_state_(other.current_state_),
        current_weight_(other.current_weight_),
        current_label_(other.current_label_),
        at_end_(other.at_end_),
        stack_(std::move(other.stack_)) {
    other.fsa_ = 0;
    other.current_state_ = 0;
    other.current_weight_ = 0;
    other.current_label_ = 0;
    other.at_end_ = true;
  }

  automata_t GetFsa() const { return fsa_; }

  bool IsFinalState() const { return fsa_->IsFinalState(current_state_); }

  size_t GetDepth() const { return stack_.GetDepth(); }

  uint64_t GetStateValue() const { return fsa_->GetStateValue(current_state_); }

  uint32_t GetInnerWeight() const { return current_weight_; }

  uint64_t GetStateId() const { return current_state_; }

  internal::IValueStoreReader::attributes_t GetValueAsAttributeVector() const {
    return fsa_->GetValueAsAttributeVector(GetStateValue());
  }

  void Prune() {
    TRACE("statetraverser Prune.");
    --stack_;
    stack_.GetStates()++;
  }

  void operator++(int) {
    TRACE("statetraverser++");
    // ignore cases where we are already at the end
    if (current_state_ == 0) {
      TRACE("at the end");
      return;
    }

    current_state_ = stack_.GetStates().GetNextState();
    TRACE("next state: %ld depth: %ld", current_state_, stack_.GetDepth());

    while (current_state_ == 0) {
      if (stack_.GetDepth() == 0) {
        TRACE("traverser exhausted.");
        current_label_ = 0;
        at_end_ = true;
        return;
      }

      TRACE("state is 0, go up");
      --stack_;
      stack_.GetStates()++;
      current_state_ = stack_.GetStates().GetNextState();
      TRACE("next state %ld depth %ld", current_state_, stack_.GetDepth());
    }

    current_label_ = stack_.GetStates().GetNextTransition();
    current_weight_ = stack_.GetStates().GetNextInnerWeight();
    TRACE("Label: %c", current_label_);
    stack_++;
    fsa_->GetOutGoingTransitions(current_state_, &stack_.GetStates(), &stack_.traversal_stack_payload);
    TRACE("found %ld outgoing states", stack_.GetStates().size());
  }

  label_t GetStateLabel() const { return current_label_; }

  traversal::TraversalPayload<TransitionT> &GetTraversalPayload() { return stack_.traversal_stack_payload; }

  operator bool() const { return !at_end_; }

  bool AtEnd() const { return at_end_; }

 private:
  automata_t fsa_;
  uint64_t current_state_;
  uint32_t current_weight_;
  label_t current_label_;
  bool at_end_;
  traversal::TraversalStack<TransitionT> stack_;

  template <class innerTraverserType>
  friend class ComparableStateTraverser;
  const traversal::TraversalStack<TransitionT> &GetStack() const { return stack_; }

  traversal::TraversalState<transition_t> &GetStates() { return stack_.GetStates(); }

  const traversal::TraversalPayload<TransitionT> &GetTraversalPayload() const { return stack_.traversal_stack_payload; }
};

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_STATE_TRAVERSER_H_
