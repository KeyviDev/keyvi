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
 * int_value_store.h
 *
 *  Created on: May 13, 2014
 *      Author: hendrik
 */

#ifndef INT_VALUE_STORE_H_
#define INT_VALUE_STORE_H_

#include "dictionary/fsa/internal/ivalue_store.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * Value store where the value consists of a single integer.
 */
class IntValueStore final{
 public:
  typedef uint32_t value_t;
  static const uint64_t no_value = 0;
  static const bool inner_weight = false;

  IntValueStore(boost::filesystem::path temporary_path = ""){
  }

  IntValueStore& operator=(IntValueStore const&) = delete;
  IntValueStore(const IntValueStore& that) = delete;

  uint64_t GetValue(value_t value, bool& no_minimization) const {
    TRACE("Value store value: %d", value);

    return value;
  }

  uint32_t GetWeightValue(value_t value) const {
    return 0;
  }

  value_store_t GetValueStoreType() const {
    return INT_VALUE_STORE;
  }

  void Write(std::ostream& stream) {}
};

/**
 * An IntValue store which uses the values as inner weights.
 */
class IntValueStoreWithInnerWeights final{
 public:
  typedef uint32_t value_t;
  static const uint32_t no_value = 0;
  static const bool inner_weight = true;

  IntValueStoreWithInnerWeights(boost::filesystem::path temporary_path = ""){
  }

  IntValueStoreWithInnerWeights& operator=(IntValueStoreWithInnerWeights const&) = delete;
  IntValueStoreWithInnerWeights(const IntValueStoreWithInnerWeights& that) = delete;

  uint64_t GetValue(value_t value, bool& no_minimization) const {
    TRACE("Value store value: %d", value);

    return value;
  }

  uint32_t GetWeightValue(value_t value) const {
    return value;
  }

  value_store_t GetValueStoreType() const {
    return INT_VALUE_STORE;
  }

  void Write(std::ostream& stream) {}
};

class IntValueStoreReader final: public IValueStoreReader{
 public:
  using IValueStoreReader::IValueStoreReader;

  virtual attributes_t GetValueAsAttributeVector(uint64_t fsa_value) override {
    attributes_t attributes(new attributes_raw_t());

    (*attributes)["weight"] = std::to_string(fsa_value);
    return attributes;
  }

  virtual std::string GetValueAsString(uint64_t fsa_value) override {
    return std::to_string(fsa_value);
  }
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */



#endif /* INT_VALUE_STORE_H_ */
