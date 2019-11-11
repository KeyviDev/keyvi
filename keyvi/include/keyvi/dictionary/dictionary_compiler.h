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
 * dictionary_compiler.h
 *
 *  Created on: Jul 17, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_DICTIONARY_COMPILER_H_
#define KEYVI_DICTIONARY_DICTIONARY_COMPILER_H_

#include <algorithm>
#include <functional>
#include <string>
#include <utility>

#include "keyvi/dictionary/fsa/generator_adapter.h"
#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/dictionary/fsa/internal/null_value_store.h"
#include "keyvi/dictionary/sort/in_memory_sorter.h"
#include "keyvi/dictionary/sort/sorter_common.h"
#include "keyvi/util/configuration.h"
#include "keyvi/util/serialization_utils.h"

#if !defined(KEYVI_DISABLE_TPIE)
#include "keyvi/dictionary/sort/tpie_sorter.h"
#endif

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

typedef sort::key_value_pair<std::string, fsa::ValueHandle> key_value_t;

/**
 * Exception class for generator, thrown when generator is used in the wrong
 * order.
 */

struct compiler_exception : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
 * Dictionary Compiler
 */
template <keyvi::dictionary::fsa::internal::value_store_t ValueStoreType = fsa::internal::value_store_t::KEY_ONLY,
#if !defined(KEYVI_DISABLE_TPIE)
          class SorterT = sort::TpieSorter<key_value_t>
#else
          class SorterT = sort::InMemorySorter<key_value_t>
#endif
          >
class DictionaryCompiler final {
  using ValueStoreT = typename fsa::internal::ValueStoreComponents<ValueStoreType>::value_store_writer_t;
  using callback_t = std::function<void(size_t, size_t, void*)>;
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
  explicit DictionaryCompiler(const keyvi::util::parameters_t& params = keyvi::util::parameters_t())
      : sorter_(params), params_(params) {
    params_[TEMPORARY_PATH_KEY] = keyvi::util::mapGetTemporaryPath(params);

    TRACE("tmp path set to %s", params_[TEMPORARY_PATH_KEY].c_str());

    stable_insert_ = keyvi::util::mapGetBool(params_, STABLE_INSERTS, false);

    value_store_ = new ValueStoreT(params_);
  }

  ~DictionaryCompiler() {
    if (!generator_) {
      // if generator was not created we have to delete the value store
      // ourselves
      delete value_store_;
    }
  }

  DictionaryCompiler& operator=(DictionaryCompiler const&) = delete;
  DictionaryCompiler(const DictionaryCompiler& that) = delete;

  void Add(const std::string& input_key, typename ValueStoreT::value_t value = ValueStoreT::no_value) {
    if (generator_) {
      throw compiler_exception("You're not supposed to add more data once compilation is done!");
    }

    size_of_keys_ += input_key.size();

    // no move, we have no ownership
    sorter_.push_back(key_value_t(input_key, RegisterValue(value)));
  }

#ifdef Py_PYTHON_H
  template <typename StringType>
  void __setitem__(StringType input_key, typename ValueStoreT::value_t value = ValueStoreT::no_value) {
    return Add(input_key, value);
  }
#endif

  void Delete(const std::string& input_key) {
    if (!stable_insert_) {
      throw compiler_exception("delete only available when using stable_inserts option");
    }

    fsa::ValueHandle handle(0,         // offset of value
                            count_++,  // counter(order)
                            0,         // weight
                            false,     // minimization
                            true);     // deleted flag

    sorter_.push_back(key_value_t(std::move(input_key), handle));
  }

  /**
   * Do the final compilation
   */
  void Compile(callback_t progress_callback = nullptr, void* user_data = nullptr) {
    size_t added_key_values = 0;
    size_t callback_trigger = 0;

    value_store_->CloseFeeding();
    sorter_.sort();
    generator_ =
        GeneratorAdapter::template CreateGenerator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>>(
            size_of_keys_, params_, value_store_);
    generator_->SetManifest(manifest_);

    if (sorter_.size() > 0) {
      size_t number_of_items = sorter_.size();

      callback_trigger = 1 + (number_of_items - 1) / 100;

      if (callback_trigger > 100000) {
        callback_trigger = 100000;
      }

      if (!stable_insert_) {
        for (auto key_value : sorter_) {
          TRACE("adding to generator: %s", key_value.key.c_str());

          generator_->Add(std::move(key_value.key), key_value.value);
          ++added_key_values;
          if (progress_callback && (added_key_values % callback_trigger == 0)) {
            progress_callback(added_key_values, number_of_items, user_data);
          }
        }

      } else {
        // special mode for stable (incremental) inserts, in this case we have
        // to respect the order and take
        // the last value if keys are equal

        auto key_values_it = sorter_.begin();
        key_value_t last_key_value = *key_values_it++;

        while (key_values_it != sorter_.end()) {
          key_value_t key_value = *key_values_it++;

          // dedup with last one wins
          if (last_key_value.key == key_value.key) {
            TRACE("Detected duplicated keys, dedup them, last one wins.");

            // check the counter to determine which key_value has been added
            // last
            if (last_key_value.value.count_ < key_value.value.count_) {
              last_key_value = key_value;
            }
            continue;
          }

          if (!last_key_value.value.deleted_) {
            TRACE("adding to generator: %s", last_key_value.key.c_str());
            generator_->Add(std::move(last_key_value.key), last_key_value.value);
            ++added_key_values;
            if (progress_callback && (added_key_values % callback_trigger == 0)) {
              progress_callback(added_key_values, number_of_items, user_data);
            }
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

        ++added_key_values;
      }
    }

    sorter_.clear();
    generator_->CloseFeeding();
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

    std::ofstream out_stream(filename, std::ios::binary);
    generator_->Write(out_stream);
    out_stream.close();
  }

 private:
  SorterT sorter_;
  keyvi::util::parameters_t params_;
  ValueStoreT* value_store_;
  typename GeneratorAdapter::AdapterPtr generator_;
  std::string manifest_;
  size_t count_ = 0;
  size_t size_of_keys_ = 0;
  bool sort_finalized_ = false;
  bool stable_insert_ = false;

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
                            count_++,                             // counter(order)
                            value_store_->GetWeightValue(value),  // weight
                            no_minimization,                      // minimization
                            false);                               // deleted flag

    return handle;
  }
};

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_DICTIONARY_COMPILER_H_
