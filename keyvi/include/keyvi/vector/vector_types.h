/* * keyvi - A key value store.
 *
 * Copyright 2018   Narek Gharibyan<narekgharibyan@gmail.com>
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
 *  vector_types.h
 *
 *  Created on: March 17, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#ifndef KEYVI_VECTOR_VECTOR_TYPES_H_
#define KEYVI_VECTOR_VECTOR_TYPES_H_

#include "keyvi/vector/types.h"
#include "keyvi/vector/vector.h"
#include "keyvi/vector/vector_generator.h"

namespace keyvi {
namespace vector {

using JsonVectorGenerator = VectorGenerator<value_store_t::JSON>;
using StringVectorGenerator = VectorGenerator<value_store_t::STRING>;

using JsonVector = Vector<value_store_t::JSON>;
using StringVector = Vector<value_store_t::STRING>;

} /* namespace vector */
} /* namespace keyvi */

#endif  // KEYVI_VECTOR_VECTOR_TYPES_H_
