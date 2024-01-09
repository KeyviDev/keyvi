/* * keyvi - A key value store.
 *
 * Copyright 2020 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * dictionary_index_compiler.h
 *
 *  Created on: Nov 17, 2020
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_DICTIONARY_INDEX_COMPILER_H_
#define KEYVI_DICTIONARY_DICTIONARY_INDEX_COMPILER_H_

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#ifdef _MSC_VER
// workaround for https://github.com/boostorg/sort/issues/43
#include <ciso646>
#endif
#include <boost/sort/sort.hpp>

#include "keyvi/dictionary/dictionary_compiler_common.h"
#include "keyvi/dictionary/fsa/generator_adapter.h"
#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/util/configuration.h"
#include "keyvi/util/os_utils.h"
#include "keyvi/util/serialization_utils.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

/**
 * Dictionary Index Compiler
 *
 * specialized compiler to be used in keyvi index. The difference to the normal compiler
 *
 * - handling of order (stable sort)
 * - support for delete
 * - no merge as part of compilation, the index handles this
 *
 * @tparam ValueStoreType The type of the value store to use
 * @tparam N Array size for fixed size value vectors, ignored otherwise
 *
 */
template <keyvi::dictionary::fsa::internal::value_store_t ValueStoreType = fsa::internal::value_store_t::KEY_ONLY>
class DictionaryIndexCompiler final {
  using ValueStoreT = typename fsa::internal::ValueStoreComponents<ValueStoreType>::value_store_writer_t;
  using GeneratorAdapter = fsa::GeneratorAdapterInterface<typename ValueStoreT::value_t>;

 public:
  /**
   * Instantiate a dictionary compiler.
   *
   * Note the memory limit only limits the memory used for internal buffers,
   * memory usage for small short-lived objects and the library itself is not
   * part of the limit.
   *
   * @param params compiler parameters
   */
  explicit DictionaryIndexCompiler(const keyvi::util::parameters_t& params = keyvi::util::parameters_t())
      : params_(params) {
    params_[TEMPORARY_PATH_KEY] = keyvi::util::mapGetTemporaryPath(params);

    parallel_sort_threshold_ =
        keyvi::util::mapGet(params_, PARALLEL_SORT_THRESHOLD_KEY, DEFAULT_PARALLEL_SORT_THRESHOLD);

    TRACE("tmp path set to %s", params_[TEMPORARY_PATH_KEY].c_str());
    value_store_ = new ValueStoreT(params_);
  }

  ~DictionaryIndexCompiler() {
    if (!generator_) {
      // if generator was not created we have to delete the value store
      // ourselves
      delete value_store_;
    }
  }

  DictionaryIndexCompiler& operator=(DictionaryIndexCompiler const&) = delete;
  DictionaryIndexCompiler(const DictionaryIndexCompiler& that) = delete;

  void Add(const std::string& input_key, typename ValueStoreT::value_t value = ValueStoreT::no_value) {
    if (generator_) {
      throw compiler_exception("You're not supposed to add more data once compilation is done!");
    }

    size_of_keys_ += input_key.size();

    memory_estimate_ += EstimateMemory(input_key);
    // no move, we have no ownership
    key_values_.push_back(key_value_t(input_key, RegisterValue(value)));
  }

  void Delete(const std::string& input_key) {
    fsa::ValueHandle handle(0,      // offset of value
                            0,      // weight
                            false,  // minimization
                            true);  // deleted flag

    memory_estimate_ += EstimateMemory(input_key);
    key_values_.push_back(key_value_t(input_key, handle));
  }

  /**
   * Do the final compilation
   */
  void Compile() {
    value_store_->CloseFeeding();
    Sort();

    generator_ =
        GeneratorAdapter::template CreateGenerator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>>(
            size_of_keys_, params_, value_store_);

    // special mode for stable (incremental) inserts, in this case we have
    // to respect the order and take
    // the last value if keys are equal
    if (key_values_.size() > 0) {
      auto key_values_it = key_values_.begin();
      key_value_t last_key_value = *key_values_it++;

      while (key_values_it != key_values_.end()) {
        key_value_t key_value = *key_values_it++;

        // dedup with last one wins
        if (last_key_value.key == key_value.key) {
          last_key_value = key_value;
          continue;
        }

        if (!last_key_value.value.deleted_) {
          TRACE("adding to generator: %s", last_key_value.key.c_str());
          generator_->Add(std::move(last_key_value.key), last_key_value.value);
        } else {
          TRACE("skipping deleted key: %s", last_key_value.key.c_str());
        }

        last_key_value = key_value;
      }

      // add the last one
      TRACE("adding to generator: %s", last_key_value.key.c_str());
      if (!last_key_value.value.deleted_) {
        generator_->Add(std::move(last_key_value.key), last_key_value.value);
      }
      key_values_.clear();
    }
    generator_->CloseFeeding();
    generator_->SetManifest(manifest_);
  }

  /**
   * Set a custom manifest to be embedded into the index file.
   *
   * @param manifest as string
   */
  void SetManifest(const std::string& manifest) {
    manifest_ = manifest;

    // if generator object is already there, set it otherwise cache it until it
    // is created
    if (generator_) {
      generator_->SetManifest(manifest);
    }
  }

  void Write(std::ostream& stream) {
    if (!generator_) {
      throw compiler_exception("not compiled yet");
    }

    generator_->Write(stream);
  }

  void WriteToFile(const std::string& filename) {
    if (!generator_) {
      throw compiler_exception("not compiled yet");
    }

    std::ofstream out_stream = keyvi::util::OsUtils::OpenOutFileStream(filename);

    generator_->Write(out_stream);
    out_stream.close();
  }

 private:
  keyvi::util::parameters_t params_;
  std::vector<key_value_t> key_values_;
  ValueStoreT* value_store_;
  typename GeneratorAdapter::AdapterPtr generator_;
  std::string manifest_;
  size_t memory_estimate_ = 0;
  size_t size_of_keys_ = 0;
  size_t parallel_sort_threshold_;

  inline void Sort() {
    if (key_values_.size() > parallel_sort_threshold_ && parallel_sort_threshold_ != 0) {
      // see gh#215 parallel_stable_sort segfaults
      // boost::sort::parallel_stable_sort(key_values_.begin(), key_values_.end());
      boost::sort::spinsort(key_values_.begin(), key_values_.end());
    } else {
      std::stable_sort(key_values_.begin(), key_values_.end());
    }
  }

  /**
   * Register a value before inserting the key(for optimization purposes).
   *
   * @param value The Value
   * @return a handle that later needs to be passed to Add()
   */
  fsa::ValueHandle RegisterValue(typename ValueStoreT::value_t value = ValueStoreT::no_value) {
    bool no_minimization = false;
    uint64_t value_idx = value_store_->AddValue(value, &no_minimization);

    fsa::ValueHandle handle(value_idx,                            // offset of value
                            value_store_->GetWeightValue(value),  // weight
                            no_minimization,                      // minimization
                            false);                               // deleted flag

    return handle;
  }
};

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_DICTIONARY_INDEX_COMPILER_H_
