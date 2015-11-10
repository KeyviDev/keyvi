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
 * weighted_state_traverser.h
 *
 *  Created on: Jun 17, 2014
 *      Author: hendrik
 */

#ifndef WEIGHTED_STATE_TRAVERSER_H_
#define WEIGHTED_STATE_TRAVERSER_H_

#include "dictionary/fsa/automata.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

class WeightedStateTraverser
final {
   public:
    WeightedStateTraverser(automata_t f)
        : WeightedStateTraverser(f, f->GetStartState()) {
    }

    WeightedStateTraverser(automata_t f, uint32_t start_state, bool advance = true)
        : fsa_(f)
    {
      current_state_ = start_state;

      TRACE("WeightedStateTraverser starting with Start state %d", current_state_);
      f->GetOutGoingTransitions(start_state, stack_.GetStates());
      // todo: sort


      if (advance) {
        this->operator ++(0);
      }
    }

    WeightedStateTraverser() = delete;
    WeightedStateTraverser& operator=(WeightedStateTraverser const&) = delete;
    WeightedStateTraverser(const WeightedStateTraverser& that) = delete;

    WeightedStateTraverser(WeightedStateTraverser&& other):
      fsa_(other.fsa_),
      current_state_(other.current_state_),
      current_label_(other.current_label_),
      current_depth_(other.current_depth_),
      state_traversal_stack_(std::move(other.state_traversal_stack_)),
      entry_traversal_stack_(std::move(other.entry_traversal_stack_))
    {
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

    uint32_t GetInnerWeight() {
      return fsa_->GetWeightValue(current_state_);
    }

    uint32_t GetStateId(){
      return current_state_;
    }

    internal::IValueStoreReader::attributes_t GetValueAsAttributeVector() {
      return fsa_->GetValueAsAttributeVector(GetStateValue());
    }

    void Prune() {
      current_state_ = state_traversal_stack_.back();
      state_traversal_stack_.pop_back();
      entry_traversal_stack_.pop_back();
      --current_depth_;
    }

    void operator++(int) {
      TRACE("weightedstatetraverser++");
      // ignore cases where we are already at the end
      if (current_state_ == 0) {
        TRACE("at the end");
        return;
      }

      current_state_ = stack_.GetStates().GetNextState();
      TRACE ("next state: %ld depth: %ld", current_state_, stack_.GetDepth());

      while (current_state_ == 0) {
        if (stack_.GetDepth() == 0) {
          TRACE("traverser exhausted.");
          current_label_ = 0;
          return;
        }

        TRACE ("state is 0, go up");
        --stack_;
        stack_.GetStates()++;
        current_state_ = stack_.GetStates().GetNextState();
        TRACE ("next state %ld depth %ld", current_state_, stack_.GetDepth());
      }

      current_label_ = stack_.GetStates().GetNextTransition();
      TRACE ("Label: %c", current_label_);
      stack_++;
      fsa_->GetOutGoingTransitions(current_state_, stack_.GetStates());

      std::sort(stack_.GetStates().transitions_.begin(), stack_.GetStates().transitions_.end(), compare);

      TRACE("found %ld outgoing states", stack_.GetStates().size());
/*



      // ignore cases where we are already at the end
      if (current_state_ == 0) {
        TRACE("end of traversal");
        return;
      }

      TRACE("finding next state");
      traversal_entry_t traversal_entry;
      for (;;) {
        uint32_t child_node = 0;

        traversal_entry = entry_traversal_stack_.back();
        TRACE("remaining transitions at %d %d", current_depth_, traversal_entry.size());

        if (traversal_entry.size()) {
          unsigned char c = traversal_entry.front().second;
          //traversal_stack_[current_depth_] = c;
          current_label_ = c;
          entry_traversal_stack_.back().pop_front();
          child_node = fsa_->TryWalkTransition(current_state_, c);
          TRACE ("Walk transition with label %c (%d)", c, c);
        }

        if (child_node) {
          /* Found a valid child node
           * go one level down
           *//*
          TRACE("found child node");

          //current_label_ = traversal_stack_[current_depth_];
          ++current_depth_;
          state_traversal_stack_.push_back(current_state_);
          current_state_ = child_node;

          // todo: refactor
          GetNextTransitionsInSortedOrder();

          return;

        } else {
          // did not found any more transitions at this deep

          if (current_depth_) {
            /* we did not find any path at the current level (deep)
             * go one level up
             *//*
            TRACE("current path exhausted going up");

            //traversal_stack_.pop_back();
            current_state_ = state_traversal_stack_.back();
            state_traversal_stack_.pop_back();
            entry_traversal_stack_.pop_back();
            --current_depth_;
          } else {
            // we are at the very end
            TRACE("reached the end, all states traversed");

            current_state_ = 0;
            current_depth_ = 0;
            current_label_ = 0;
            return;
          }

        }
      }*/
    }

    bool operator==(const WeightedStateTraverser &other) const {
      return (current_state_ == other.current_state_ && current_depth_ == other.current_depth_ && current_label_ == other.current_label_);
    }

    bool operator!=(const WeightedStateTraverser &other) const {
      return !(operator==(other));
    }

    unsigned char GetStateLabel() {
      return current_label_;
    }

   private:
    typedef std::deque<std::pair<uint32_t, unsigned char>> traversal_entry_t;

    automata_t fsa_;
    uint32_t current_state_ = 0;
    unsigned char current_label_ = 0;
    size_t current_depth_ = 0;

    internal::TraversalStack<internal::WeightedTransition> stack_;

    std::vector<uint32_t> state_traversal_stack_;
    std::vector<traversal_entry_t> entry_traversal_stack_;

    static bool compare(const internal::WeightedTransition& a, const internal::WeightedTransition& b) {
      return a.weight < b.weight;
    };

    /*void GetNextTransitionsInSortedOrder() {
      uint32_t child_node;
      traversal_entry_t outgoing_transitions;
      for (int i = 1; i < 256; ++i) {
        child_node = fsa_->TryWalkTransition(current_state_, i);
        if (child_node) {
          TRACE("Found childnode, weight: %d", fsa_->GetWeightValue());

          outgoing_transitions.push_back(
              std::pair<uint32_t, unsigned char>(fsa_->GetWeightValue(child_node), i));
        }
      }

      std::sort(outgoing_transitions.begin(), outgoing_transitions.end(), compare);
      TRACE("number of transitions found: %d", outgoing_transitions.size());
      entry_traversal_stack_.push_back(outgoing_transitions);
    }*/

  };

  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* WEIGHTED_STATE_TRAVERSER_H_ */
