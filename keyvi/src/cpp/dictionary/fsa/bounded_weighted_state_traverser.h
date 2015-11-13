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
 * bounded_weighted_state_traverser.h
 *
 *  Created on: Jul 11, 2014
 *      Author: hendrik
 */

#ifndef BOUNDED_WEIGHTED_STATE_TRAVERSER_H_
#define BOUNDED_WEIGHTED_STATE_TRAVERSER_H_

#include "dictionary/fsa/automata.h"
#include "dictionary/util/bounded_priority_queue.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

class BoundedWeightedStateTraverser
final {
   public:
    BoundedWeightedStateTraverser(automata_t f, size_t top_number_of_results)
        : BoundedWeightedStateTraverser(f, f->GetStartState(),
                                        top_number_of_results) {
    }

    BoundedWeightedStateTraverser(automata_t f, uint64_t start_state,
                                  size_t top_number_of_results, bool advance =
                                      true)
        : fsa_(f),
          priority_queue_(top_number_of_results) {
      current_state_ = start_state;

      TRACE("BoundedWeightedStateTraverser starting with Start state %d", current_state_);
      GetNextTransitionsInSortedOrder(0);
      if (advance) {
        this->operator ++(0);
      }
    }

    BoundedWeightedStateTraverser() = delete;
    BoundedWeightedStateTraverser& operator=(
        BoundedWeightedStateTraverser const&) = delete;
    BoundedWeightedStateTraverser(const BoundedWeightedStateTraverser& that) = delete;

    BoundedWeightedStateTraverser(BoundedWeightedStateTraverser&& other)
        : fsa_(other.fsa_),
          current_state_(other.current_state_),
          current_label_(other.current_label_),
          current_depth_(other.current_depth_),
          state_traversal_stack_(std::move(other.state_traversal_stack_)),
          entry_traversal_stack_(std::move(other.entry_traversal_stack_)),
          priority_queue_ (std::move(other.priority_queue_)){
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

    uint64_t GetStateId() {
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

    void TryReduceResultQueue() {
      // todo: implement optimization to reduce queue for every good result
      /*if (fsa_->GetWeightValue(state_traversal_stack_.back()) > priority_queue_.Back()) {
        priority_queue_.ReduceSize();
      }*/
    }

    void operator++(int) {

      // ignore cases where we are already at the end
      if (current_state_ == 0) {
        TRACE("end of traversal");
        return;
      }

      TRACE("finding next state");
      traversal_entry_t traversal_entry;
      for (;;) {
        uint64_t child_node = 0;
        uint32_t weight = 0;
        traversal_entry = entry_traversal_stack_.back();
        TRACE("remaining transitions at %d %d", current_depth_, traversal_entry.size());

        if (traversal_entry.size()) {
          // in case the priority queue got better items between creating the transition list and now,
          // we have to skip here
          // note: child_node remains 0, so all remaining transitions are dropped later on
          weight = traversal_entry.front().first;
          if (weight >= priority_queue_.Back()) {
            current_label_ = traversal_entry.front().second;;
            entry_traversal_stack_.back().pop_front();
            child_node = fsa_->TryWalkTransition(current_state_, current_label_);
            TRACE ("Walk transition with label %c (%d)", current_label_, current_label_);
          }
        }

        if (child_node) {
          /* Found a valid child node
           * go one level down
           */
          TRACE("found child node");

          //current_label_ = traversal_stack_[current_depth_];
          ++current_depth_;
          state_traversal_stack_.push_back(current_state_);
          current_state_ = child_node;

          // todo: refactor
          TRACE("moving down to depth: %d", current_depth_);
          GetNextTransitionsInSortedOrder(weight);

          return;

        } else {
          // did not found any more transitions at this deep

          if (current_depth_) {
            /* we did not find any path at the current level (deep)
             * go one level up
             */
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
      }
    }

    bool operator==(const BoundedWeightedStateTraverser &other) const {
      return (current_state_ == other.current_state_
          && current_depth_ == other.current_depth_
          && current_label_ == other.current_label_);
    }

    bool operator!=(const BoundedWeightedStateTraverser &other) const {
      return !(operator==(other));
    }

    unsigned char GetStateLabel() {
      return current_label_;
    }

   private:
    typedef std::deque<std::pair<uint32_t, unsigned char>> traversal_entry_t;

    automata_t fsa_;
    uint64_t current_state_ = 0;
    unsigned char current_label_ = 0;
    int current_depth_ = 0;
    std::vector<uint64_t> state_traversal_stack_;
    std::vector<traversal_entry_t> entry_traversal_stack_;
    util::BoundedPriorityQueue<uint32_t> priority_queue_;

    static bool compare(std::pair<uint32_t, unsigned char> i,
                        std::pair<uint32_t, unsigned char> j) {
      return i.first > j.first;
    }
    ;

    void GetNextTransitionsInSortedOrder(uint32_t parent_weight) {
      uint64_t child_node;
      traversal_entry_t outgoing_transitions;
      for (int i = 1; i < 256; ++i) {
        child_node = fsa_->TryWalkTransition(current_state_, i);
        if (child_node) {
          // todo: stop reading the weight dependent on the depth of traversal
          uint32_t weight = fsa_->GetWeightValue(child_node);

          // if weight is not set take the weight of the parent
          if (weight == 0) {
            TRACE("Received no weight, take weight from parent.");
            weight = parent_weight;
          }

          TRACE("transition with weight %d queue last: %d", weight, priority_queue_.Back());
          if (weight < priority_queue_.Back()) {
            continue;
          } else if (parent_weight != weight && weight > priority_queue_.Back()) {
            priority_queue_.Put(weight);
          }

          outgoing_transitions.push_back(
              std::pair<uint32_t, unsigned char>(weight, i));

        }
      }

      std::sort(outgoing_transitions.begin(), outgoing_transitions.end(),
                compare);
      TRACE("first transition after sort: %c %d", outgoing_transitions[0].second, outgoing_transitions[0].first);

      TRACE("number of transitions found: %d", outgoing_transitions.size());
      entry_traversal_stack_.push_back(outgoing_transitions);
    }

  };

  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* BOUNDED_WEIGHTED_STATE_TRAVERSER_H_ */
