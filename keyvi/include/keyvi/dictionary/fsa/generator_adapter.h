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
 * generator_adapter.h
 *
 * Interface to abstract away compile time(template) parameters of the
 * generator.
 *  Created on: Oct 28, 2015
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_GENERATOR_ADAPTER_H_
#define KEYVI_DICTIONARY_FSA_GENERATOR_ADAPTER_H_

#include <memory>
#include <string>
#include <utility>

#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/util/configuration.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

template <typename ValueT>
class GeneratorAdapterInterface {
 public:
  using AdapterPtr = std::unique_ptr<GeneratorAdapterInterface>;

  template <class PersistenceT, class ValueStoreT>
  static AdapterPtr CreateGenerator(size_t size_of_keys, const keyvi::util::parameters_t& params,
                                    ValueStoreT* value_store);

 public:
  GeneratorAdapterInterface() {}

  virtual void Add(const std::string& input_key, ValueT value) {}
  virtual void Add(const std::string& input_key, const fsa::ValueHandle& value) {}

  virtual size_t GetFsaSize() const { return 0; }
  virtual void CloseFeeding() {}
  virtual void Write(std::ostream& stream) {}
  virtual void WriteToFile(const std::string& filename) {}
  virtual void SetManifest(const std::string& manifest) {}

  virtual ~GeneratorAdapterInterface() {}
};

template <class PersistenceT, class ValueStoreT, class OffsetTypeT, class HashCodeTypeT>
class GeneratorAdapter final : public GeneratorAdapterInterface<typename ValueStoreT::value_t> {
 public:
  explicit GeneratorAdapter(const keyvi::util::parameters_t& params = keyvi::util::parameters_t(),
                            ValueStoreT* value_store = NULL)
      : generator_(params, value_store) {}

  void Add(const std::string& input_key, typename ValueStoreT::value_t value) {
    generator_.Add(std::move(input_key), value);
  }

  void Add(const std::string& input_key, const fsa::ValueHandle& value) { generator_.Add(std::move(input_key), value); }

  size_t GetFsaSize() const { return generator_.GetFsaSize(); }

  void CloseFeeding() { generator_.CloseFeeding(); }

  void Write(std::ostream& stream) { generator_.Write(stream); }

  void WriteToFile(const std::string& filename) { generator_.WriteToFile(filename); }

  void SetManifest(const std::string& manifest) { generator_.SetManifest(manifest); }

 private:
  Generator<PersistenceT, ValueStoreT, OffsetTypeT, HashCodeTypeT> generator_;
};

template <typename ValueT>
template <class PersistenceT, class ValueStoreT>
typename GeneratorAdapterInterface<ValueT>::AdapterPtr GeneratorAdapterInterface<ValueT>::CreateGenerator(
    size_t size_of_keys, const keyvi::util::parameters_t& params, ValueStoreT* value_store) {
  size_t memory_limit = keyvi::util::mapGetMemory(params, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_GENERATOR);

  // todo: find good parameters for auto-guessing this
  if (size_of_keys > UINT32_MAX) {
    if (memory_limit > 0x280000000UL /* 10 GB */) {
      return AdapterPtr(new fsa::GeneratorAdapter<PersistenceT, ValueStoreT, uint64_t, int64_t>(params, value_store));
    } else {
      return AdapterPtr(new fsa::GeneratorAdapter<PersistenceT, ValueStoreT, uint64_t, int32_t>(params, value_store));
    }
  } else {
    if (memory_limit > 0x140000000UL) /* 5GB */ {
      return AdapterPtr(new fsa::GeneratorAdapter<PersistenceT, ValueStoreT, uint32_t, int64_t>(params, value_store));
    } else {
      return AdapterPtr(new fsa::GeneratorAdapter<PersistenceT, ValueStoreT, uint32_t, int32_t>(params, value_store));
    }
  }
}

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_GENERATOR_ADAPTER_H_
