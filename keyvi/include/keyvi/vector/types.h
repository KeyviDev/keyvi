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
 *  types.h
 *
 *  Created on: March 17, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#ifndef KEYVI_VECTOR_TYPES_H_
#define KEYVI_VECTOR_TYPES_H_

#include "keyvi/dictionary/fsa/internal/memory_map_manager.h"
#include "keyvi/dictionary/fsa/internal/value_store_types.h"

namespace keyvi {
namespace vector {

using offset_type = uint64_t;
using MemoryMapManager = dictionary::fsa::internal::MemoryMapManager;
using value_store_t = dictionary::fsa::internal::value_store_t;

} /* namespace vector */
} /* namespace keyvi */

#endif  // KEYVI_VECTOR_TYPES_H_
