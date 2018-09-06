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

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_INT_VALUE_STORE_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_INT_VALUE_STORE_H_

#include <string>
#include <vector>

#include "dictionary/fsa/internal/ivalue_store.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

class IntValueStoreBase {
 public:
  typedef uint32_t value_t;
  static const bool inner_weight = false;

  static value_store_t GetValueStoreType() { return INT; }

  uint64_t AddValue(value_t value, bool* no_minimization) const {
    TRACE("Value store value: %d", value);

    return value;
  }

  uint32_t GetWeightValue(value_t value) const { return 0; }
  uint32_t GetMergeWeight(uint64_t fsa_value) { return 0; }

  void CloseFeeding() {}
  void Write(std::ostream& stream) const {}
};

/**
 * Value store where the value consists of a single integer.
 */

class IntValueStore final : public IntValueStoreBase {
 public:
  explicit IntValueStore(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  IntValueStore(const IntValueStore& that) = delete;
  IntValueStore& operator=(IntValueStore const&) = delete;
};

class IntValueStoreMerge final : public IntValueStoreBase {
 public:
  explicit IntValueStoreMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  IntValueStoreMerge& operator=(IntValueStoreMerge const&) = delete;
  IntValueStoreMerge(const IntValueStoreMerge& that) = delete;
  uint64_t AddValueMerge(const char* p, uint64_t v, bool* no_minimization) { return v; }
};

class IntValueStoreAppendMerge final : public IntValueStoreBase {
 public:
  explicit IntValueStoreAppendMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  explicit IntValueStoreAppendMerge(const std::vector<std::string>&,
                                    const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  IntValueStoreAppendMerge& operator=(IntValueStoreAppendMerge const&) = delete;
  IntValueStoreAppendMerge(const IntValueStoreAppendMerge& that) = delete;
  uint64_t AddValueAppendMerge(size_t fileIndex, uint64_t oldIndex) { return oldIndex; }
};

class IntValueStoreReader final : public IValueStoreReader {
 public:
  using IValueStoreReader::IValueStoreReader;

  value_store_t GetValueStoreType() const override { return INT; }

  attributes_t GetValueAsAttributeVector(uint64_t fsa_value) const override {
    attributes_t attributes(new attributes_raw_t());

    (*attributes)["weight"] = std::to_string(fsa_value);
    return attributes;
  }

  std::string GetValueAsString(uint64_t fsa_value) const override { return std::to_string(fsa_value); }
};

template <>
struct Dict<value_store_t::INT> {
  typedef IntValueStore value_store_t;
  typedef IntValueStoreMerge value_store_merge_t;
  typedef IntValueStoreAppendMerge value_store_append_merge_t;
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_INT_VALUE_STORE_H_
