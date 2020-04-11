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
 *  Created on: Jan 8, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_INT_INNER_WEIGHTS_VALUE_STORE_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_INT_INNER_WEIGHTS_VALUE_STORE_H_

#include <string>
#include <vector>

#include "keyvi/dictionary/fsa/internal/ivalue_store.h"
#include "keyvi/dictionary/fsa/internal/value_store_types.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

class IntInnerWeightsValueStoreBase {
 public:
  typedef uint64_t value_t;
  static const bool inner_weight = true;

  IntInnerWeightsValueStoreBase() {}

  IntInnerWeightsValueStoreBase(const IntInnerWeightsValueStoreBase& that) = delete;
  IntInnerWeightsValueStoreBase& operator=(IntInnerWeightsValueStoreBase const&) = delete;

  uint64_t AddValue(value_t value, bool* no_minimization) const {
    TRACE("Value store value: %d", value);

    return value;
  }
  uint32_t GetWeightValue(value_t value) const { return value; }

  uint32_t GetMergeWeight(uint64_t fsa_value) { return fsa_value; }

  void CloseFeeding() {}
  void Write(std::ostream& stream) const {}

  static value_store_t GetValueStoreType() { return value_store_t::INT_WITH_WEIGHTS; }
};

/**
 * An IntValue store which uses the values as inner weights.
 */
class IntInnerWeightsValueStore final : public IntInnerWeightsValueStoreBase {
 public:
  explicit IntInnerWeightsValueStore(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}
};

class IntInnerWeightsValueStoreMerge final : public IntInnerWeightsValueStoreBase {
 public:
  explicit IntInnerWeightsValueStoreMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  uint64_t AddValueMerge(const char* p, uint64_t v, bool* no_minimization) { return v; }
};

class IntInnerWeightsValueStoreAppendMerge final : public IntInnerWeightsValueStoreBase {
 public:
  explicit IntInnerWeightsValueStoreAppendMerge(
      const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  explicit IntInnerWeightsValueStoreAppendMerge(
      const std::vector<std::string>&, const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  uint64_t AddValueAppendMerge(size_t fileIndex, uint64_t oldIndex) { return oldIndex; }
};

class IntInnerWeightsValueStoreReader final : public IValueStoreReader {
 public:
  using IValueStoreReader::IValueStoreReader;

  value_store_t GetValueStoreType() const override { return value_store_t::INT_WITH_WEIGHTS; }

  attributes_t GetValueAsAttributeVector(uint64_t fsa_value) const override {
    attributes_t attributes(new attributes_raw_t());

    (*attributes)["weight"] = std::to_string(fsa_value);
    return attributes;
  }

  std::string GetValueAsString(uint64_t fsa_value) const override { return std::to_string(fsa_value); }
};

template <>
struct ValueStoreComponents<value_store_t::INT_WITH_WEIGHTS> {
  using value_store_writer_t = IntInnerWeightsValueStore;
  using value_store_reader_t = IntInnerWeightsValueStoreReader;
  using value_store_merger_t = IntInnerWeightsValueStoreMerge;
  using value_store_append_merger_t = IntInnerWeightsValueStoreAppendMerge;
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_INT_INNER_WEIGHTS_VALUE_STORE_H_
