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
 * value_store_types.h
 *
 *  Created on: Sep 11, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_TYPES_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_TYPES_H_

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * "Registry" for value stores, required to differ between the stores.
 *
 * Do not forget to add the new value store to ValueStoreFactory
 */
enum class value_store_t {
  KEY_ONLY = 1,          //!< NullValueStore
  INT = 2,               //!< IntValueStore
  STRING = 3,            //!< StringValueStore
  JSON_DEPRECATED = 4,   //!< deprecated, not used
  JSON = 5,              //!< JsonValueStore
  INT_WITH_WEIGHTS = 6,  //!< IntInnerWeightsValueStore
  FLOAT_VECTOR = 7,      //!< FloatVectorValueStore
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_TYPES_H_
