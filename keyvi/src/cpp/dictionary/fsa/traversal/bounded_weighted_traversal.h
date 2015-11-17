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
 * bounded_weighted_traversal.h
 *
 *  Created on: Nov 17, 2015
 *      Author: hendrik
 */

#ifndef BOUNDED_WEIGHTED_TRAVERSAL_H_
#define BOUNDED_WEIGHTED_TRAVERSAL_H_

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"
#include "dictionary/fsa/traversal/weighted_traversal.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace traversal {

struct BoundedWeightedTransition: public WeightedTransition {
  using WeightedTransition::WeightedTransition;
};

//struct BTraversalState: public TraversalState<BoundedWeightedTransition> {
//};


template<>
inline void TraversalState<BoundedWeightedTransition>::PostProcess() {
  if (transitions_.size() > 0) {
    std::sort(transitions_.begin(), transitions_.end(), WeightedTransitionCompare);
  }
}


} /* namespace traversal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* BOUNDED_WEIGHTED_TRAVERSAL_H_ */
