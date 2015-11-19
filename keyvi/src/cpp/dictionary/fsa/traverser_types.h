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
 * traverser_types.h
 *
 *  Created on: Nov 17, 2015
 *      Author: hendrik
 */

#ifndef TRAVERSER_TYPES_H_
#define TRAVERSER_TYPES_H_

#include "dictionary/fsa/state_traverser.h"
#include "dictionary/fsa/traversal/weighted_traversal.h"
#include "dictionary/fsa/traversal/bounded_weighted_traversal.h"
#include "dictionary/fsa/traversal/near_traversal.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

// weighted state traverser
typedef  StateTraverser<traversal::WeightedTransition> WeightedStateTraverser;
typedef  StateTraverser<traversal::BoundedWeightedTransition> BoundedWeightedStateTraverser2;
typedef  StateTraverser<traversal::NearTransition> NearStateTraverser;

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */



#endif /* TRAVERSER_TYPES_H_ */
