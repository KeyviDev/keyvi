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
 * weighted_traversal.h
 *
 *  Created on: Nov 17, 2015
 *      Author: hendrik
 */

#ifndef WEIGHTED_TRAVERSAL_H_
#define WEIGHTED_TRAVERSAL_H_

#include "dictionary/fsa/traversal/traversal_base.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace traversal {

struct WeightedTransition {
  WeightedTransition(uint64_t s, uint32_t w, unsigned char l): state(s), weight(w), label(l) {}

  uint64_t state;
  uint32_t weight;
  unsigned char label;
};

static bool WeightedTransitionCompare(const WeightedTransition& a, const WeightedTransition& b) {
  TRACE("compare %d %d", a.weight, b.weight);

  return a.weight > b.weight;
};

template<>
inline void TraversalState<WeightedTransition>::PostProcess(TraversalPayload<WeightedTransition>& payload) {
  if (traversal_state_payload.transitions.size() > 0) {
    std::sort(traversal_state_payload.transitions.begin(), traversal_state_payload.transitions.end(), WeightedTransitionCompare);
  }
}

} /* namespace traversal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */


#endif /* WEIGHTED_TRAVERSAL_H_ */
