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
 * traversal_base.h
 *
 *  Created on: Nov 17, 2015
 *      Author: hendrik
 */

#ifndef TRAVERSAL_BASE_H_
#define TRAVERSAL_BASE_H_

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace traversal {

struct Transition {
  Transition(uint64_t s, unsigned char l): state(s), label(l) {}

  uint64_t state;
  unsigned char label;
};

template<class TransitionT = Transition>
struct TraversalPayload {
  TraversalPayload(): current_depth(0){}

  size_t current_depth;
};

template<class TransitionT = Transition>
struct TraversalStatePayload {
  std::vector<TransitionT> transitions;
  size_t position = 0;
};

/**
 * A helper data structure to hold a state in graph traversal
 */
template<class TransitionT = Transition>
struct TraversalState {

  void Add(uint64_t s, unsigned char l, TraversalPayload<TransitionT>& payload) {
    traversal_state_payload.transitions.push_back(TransitionT(s, l));
  }

  void Add(uint64_t s, uint32_t weight, unsigned char l, TraversalPayload<TransitionT>& payload) {
    traversal_state_payload.transitions.push_back(TransitionT(s, weight, l));
  }

  uint64_t GetNextState() const {
    if (traversal_state_payload.position < traversal_state_payload.transitions.size()) {
      return traversal_state_payload.transitions[traversal_state_payload.position].state;
    }

    // else
    return 0;
  }

  unsigned char GetNextTransition() const {
    return traversal_state_payload.transitions[traversal_state_payload.position].label;
  }

  uint32_t GetNextInnerWeight() const {
    return 0;
  }

  size_t size() const {
    return traversal_state_payload.transitions.size();
  }

  size_t& operator++ (){
    return ++traversal_state_payload.position;
  }

  size_t operator++ (int){
    return traversal_state_payload.position++;
  }

  void Clear(){
    traversal_state_payload.position = 0;
    traversal_state_payload.transitions.clear();
  }

  void PostProcess(TraversalPayload<TransitionT>& payload){
  }

  TraversalStatePayload<TransitionT> traversal_state_payload;
};

/**
 * A helper data structure memorize the path of a graph traversal.
 */
template<class TransitionT = Transition>
struct TraversalStack {
  TraversalStack():traversal_states(), traversal_stack_payload() {
    traversal_states.resize(20);
  }

  TraversalStack(TraversalPayload<TransitionT>& payload):traversal_states(), traversal_stack_payload(payload) {
    traversal_states.resize(20);
  }

  TraversalState<TransitionT>& GetStates() {
    return traversal_states[traversal_stack_payload.current_depth];
  }

  size_t GetDepth() const {
    return traversal_stack_payload.current_depth;
  }

  size_t& operator++ (){
    // resize if needed
    if (traversal_states.size() < traversal_stack_payload.current_depth + 2) {
      traversal_states.resize(traversal_stack_payload.current_depth + 10);
    }
    return ++traversal_stack_payload.current_depth;
  }

  size_t operator++ (int){
    traversal_stack_payload.current_depth++;
    // resize if needed
    if (traversal_states.size() < traversal_stack_payload.current_depth + 1) {
      traversal_states.resize(traversal_stack_payload.current_depth + 10);
    }

    return traversal_stack_payload.current_depth;
  }

  size_t& operator-- (){
    return --traversal_stack_payload.current_depth;
  }

  size_t operator-- (int){
    return traversal_stack_payload.current_depth--;
  }

  std::vector<TraversalState<TransitionT>> traversal_states;
  TraversalPayload<TransitionT> traversal_stack_payload;
};


} /* namespace traversal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* TRAVERSAL_BASE_H_ */
