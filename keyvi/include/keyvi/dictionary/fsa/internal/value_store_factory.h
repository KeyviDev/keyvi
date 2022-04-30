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
 * value_store_factory.h
 *
 *  Created on: Jun 13, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_FACTORY_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_FACTORY_H_

#include "keyvi/dictionary/fsa/internal/float_vector_value_store.h"
#include "keyvi/dictionary/fsa/internal/int_inner_weights_value_store.h"
#include "keyvi/dictionary/fsa/internal/int_value_store.h"
#include "keyvi/dictionary/fsa/internal/ivalue_store.h"
#include "keyvi/dictionary/fsa/internal/json_value_store.h"
#include "keyvi/dictionary/fsa/internal/memory_map_flags.h"
#include "keyvi/dictionary/fsa/internal/null_value_store.h"
#include "keyvi/dictionary/fsa/internal/string_value_store.h"
#include "keyvi/dictionary/fsa/internal/value_store_properties.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

class ValueStoreFactory final {
 public:
  static IValueStoreReader* MakeReader(value_store_t type, boost::interprocess::file_mapping* file_mapping,
                                       const ValueStoreProperties& properties,
                                       loading_strategy_types loading_strategy = loading_strategy_types::lazy) {
    switch (type) {
      case value_store_t::KEY_ONLY:
        return new ValueStoreComponents<value_store_t::KEY_ONLY>::value_store_reader_t(file_mapping, properties);
      case value_store_t::INT:
        return new ValueStoreComponents<value_store_t::INT>::value_store_reader_t(file_mapping, properties);
      case value_store_t::STRING:
        return new ValueStoreComponents<value_store_t::STRING>::value_store_reader_t(file_mapping, properties,
                                                                                     loading_strategy);
      case value_store_t::JSON_DEPRECATED:
        throw std::invalid_argument("Deprecated Value Storage type");
      case value_store_t::JSON:
        return new ValueStoreComponents<value_store_t::JSON>::value_store_reader_t(file_mapping, properties,
                                                                                   loading_strategy);
      case value_store_t::INT_WITH_WEIGHTS:
        return new ValueStoreComponents<value_store_t::INT_WITH_WEIGHTS>::value_store_reader_t(file_mapping,
                                                                                               properties);
      case value_store_t::FLOAT_VECTOR:
        return new ValueStoreComponents<value_store_t::FLOAT_VECTOR>::value_store_reader_t(file_mapping, properties);
      default:
        throw std::invalid_argument("Unknown Value Storage type");
    }
  }
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_FACTORY_H_
