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
 * null_value_store.h
 *
 *  Created on: May 13, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_NULL_VALUE_STORE_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_NULL_VALUE_STORE_H_

#include <string>
#include <vector>

#include "keyvi/dictionary/fsa/internal/ivalue_store.h"
#include "keyvi/dictionary/fsa/internal/value_store_types.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

class NullValueStoreBase {
 public:
  typedef uint32_t value_t;
  static const uint64_t no_value = 0;
  static const bool inner_weight = false;

  NullValueStoreBase() {}

  NullValueStoreBase& operator=(NullValueStoreBase const&) = delete;
  NullValueStoreBase(const NullValueStoreBase& that) = delete;

  uint64_t AddValue(value_t value, bool* no_minimization) { return 0; }

  uint32_t GetWeightValue(value_t value) const { return 0; }

  uint32_t GetMergeWeight(uint64_t fsa_value) { return 0; }

  void CloseFeeding() {}

  void Write(std::ostream& stream) const {}

  static value_store_t GetValueStoreType() { return value_store_t::KEY_ONLY; }
};

/**
 * A value store for key-only dictionaries.
 * Simply return 0 for all values.
 */
class NullValueStore final : public NullValueStoreBase {
 public:
  explicit NullValueStore(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}
};

class NullValueStoreMerge final : public NullValueStoreBase {
 public:
  explicit NullValueStoreMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  uint64_t AddValueMerge(const char* p, uint64_t v, bool* no_minimization) { return 0; }
};

class NullValueStoreAppendMerge final : public NullValueStoreBase {
 public:
  explicit NullValueStoreAppendMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  explicit NullValueStoreAppendMerge(const std::vector<std::string>&,
                                     const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  uint64_t AddValueAppendMerge(size_t fileIndex, uint64_t oldIndex) { return 0; }
};

class NullValueStoreReader final : public IValueStoreReader {
 public:
  using IValueStoreReader::IValueStoreReader;

  value_store_t GetValueStoreType() const override { return value_store_t::KEY_ONLY; }

  attributes_t GetValueAsAttributeVector(uint64_t fsa_value) const override { return attributes_t(); }

  std::string GetValueAsString(uint64_t fsa_value) const override { return ""; }
};

template <>
struct ValueStoreComponents<value_store_t::KEY_ONLY> {
  using value_store_writer_t = NullValueStore;
  using value_store_reader_t = NullValueStoreReader;
  using value_store_merger_t = NullValueStoreMerge;
  using value_store_append_merger_t = NullValueStoreAppendMerge;
};

} /* namespace internal */
}  // namespace fsa
}  // namespace dictionary
}  // namespace keyvi

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_NULL_VALUE_STORE_H_
