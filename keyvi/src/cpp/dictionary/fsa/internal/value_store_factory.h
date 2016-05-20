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

#ifndef VALUE_STORE_FACTORY_H_
#define VALUE_STORE_FACTORY_H_

#include "dictionary/fsa/internal/memory_map_flags.h"
#include "dictionary/fsa/internal/null_value_store.h"
#include "dictionary/fsa/internal/int_value_store.h"
#include "dictionary/fsa/internal/string_value_store.h"
#include "dictionary/fsa/internal/json_value_store_deprecated.h"
#include "dictionary/fsa/internal/json_value_store.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

class ValueStoreFactory final {
 public:
  static IValueStoreReader* MakeReader(value_store_t type, std::istream& stream,
                                boost::interprocess::file_mapping* file_mapping, loading_strategy_types loading_strategy = loading_strategy_types::lazy){
    switch (type){
      case NULL_VALUE_STORE:
        return new NullValueStoreReader(stream, file_mapping);
      case INT_VALUE_STORE:
        return new IntValueStoreReader(stream, file_mapping);
      case STRING_VALUE_STORE:
        return new StringValueStoreReader(stream, file_mapping, loading_strategy);
      case JSON_VALUE_STORE_DEPRECATED:
        return new JsonValueStoreDeprecatedReader(stream, file_mapping, false);
      case JSON_VALUE_STORE:
        return new JsonValueStoreReader(stream, file_mapping, loading_strategy);
      default:
        throw std::invalid_argument("Unknown Value Storage type");
    }
  }
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* VALUE_STORE_FACTORY_H_ */
