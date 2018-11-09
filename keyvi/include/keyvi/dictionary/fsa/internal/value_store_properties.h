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
 * value_store_properties.h
 *
 *  Created on: Nov 9, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_PROPERTIES_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_PROPERTIES_H_

#include <cstddef>
#include <fstream>

#include <boost/lexical_cast.hpp>

#include "rapidjson/document.h"

#include "util/serialization_utils.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

class ValueStoreProperties final {
 public:
  ValueStoreProperties() {}

  ValueStoreProperties(const size_t offset, const size_t size) {
    offset_ = offset;
    size_ = size;
  }

  size_t GetSize() const { return size_; }

  size_t GetOffset() const { return offset_; }

  static ValueStoreProperties FromJsonStream(std::istream& stream) {
    rapidjson::Document value_store_properties;
    keyvi::util::SerializationUtils::ReadJsonRecord(stream, value_store_properties);
    const size_t offset = stream.tellg();
    const size_t size = boost::lexical_cast<size_t>(value_store_properties["size"].GetString());

    // check for file truncation
    if (size > 0) {
      stream.seekg(size - 1, stream.cur);
      if (stream.peek() == EOF) {
        throw std::invalid_argument("file is corrupt(truncated)");
      }
    }
    return ValueStoreProperties(offset, size);
  }

 private:
  size_t offset_ = 0;
  size_t size_ = 0;
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  //  KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_PROPERTIES_H_
