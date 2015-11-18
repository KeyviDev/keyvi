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
 * near_traversal.h
 *
 *  Created on: Nov 18, 2015
 *      Author: hendrik
 */

#ifndef NEAR_TRAVERSAL_H_
#define NEAR_TRAVERSAL_H_

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"
#include "dictionary/fsa/traversal/traversal_base.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace traversal {


struct NearTransition: public Transition {
  using Transition::Transition;
};


template<>
struct TraversalPayload<NearTransition> {
  TraversalPayload(): current_depth(0), lookup_key(){}
  TraversalPayload(const std::string& lookup_key): current_depth(0), lookup_key(lookup_key){}

  size_t current_depth;
  std::string lookup_key;
};

template<>
struct TraversalStatePayload<NearTransition> {
  std::vector<NearTransition> transitions;
  size_t position = 0;
};


template<>
inline void TraversalState<NearTransition>::PostProcess() {
}


template<>
inline void TraversalState<NearTransition>::Add(uint64_t s, unsigned char l, TraversalPayload<NearTransition>& payload) {
  // check exact match
  TRACE("Add %c depth %d, exact %c", l, payload.current_depth, payload.lookup_key[payload.current_depth]);

  if (payload.lookup_key[payload.current_depth] == l) {
    // fill in and set position 0, so that we start traversal there
    traversal_state_payload.position = 0;
    traversal_state_payload.transitions[0] = NearTransition(s, l);
    return;
  }
  traversal_state_payload.transitions.push_back(NearTransition(s, l));
}


template<>
inline void TraversalState<NearTransition>::Clear() {
  // keep the 1st bucket empty for an exact match
  traversal_state_payload.position = 1;
  traversal_state_payload.transitions.clear();
  traversal_state_payload.transitions.push_back(NearTransition(0, 0));
}

} /* namespace traversal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */


#endif /* NEAR_TRAVERSAL_H_ */
