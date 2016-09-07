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

#ifndef DICTIONARY_COMPILER_H_
#define DICTIONARY_COMPILER_H_

#include <algorithm>
#include <functional>
#include <boost/property_tree/ptree.hpp>
#include "tpie/serialization_sorter.h"

#include "dictionary/util/tpie_initializer.h"
#include "dictionary/fsa/internal/null_value_store.h"
#include "dictionary/fsa/internal/serialization_utils.h"
#include "dictionary/fsa/generator_adapter.h"
#include "dictionary/fsa/internal/constants.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

/**
 * structure for internal processing
 * Note: Not using std::pair because it did not compile with Tpie
 */
struct key_value_pair {
  key_value_pair() : key(), value() {
  }

  key_value_pair(const std::string& k, const fsa::ValueHandle& v): key(k), value(v) {}

  bool operator<(const key_value_pair kv) const {
    return key < kv.key;
  }

  std::string key;
  fsa::ValueHandle value;
};

/**
 * Tpie serialization and deserialization for sorting.
 */
template<typename Dst>
void serialize(Dst & d, const keyvi::dictionary::key_value_pair & pt) {
  using tpie::serialize;
  serialize(d, pt.key);
  serialize(d, pt.value);
}
template<typename Src>
void unserialize(Src & s, keyvi::dictionary::key_value_pair & pt) {
  using tpie::unserialize;
  unserialize(s, pt.key);
  unserialize(s, pt.value);
}

typedef const fsa::internal::IValueStoreWriter::vs_param_t compiler_param_t;

/**
 * Dictionary Compiler
 */
template<class PersistenceT, class ValueStoreT = fsa::internal::NullValueStore>
class DictionaryCompiler
  final {

    typedef key_value_pair key_value_t;
    typedef std::function<void (size_t , size_t, void*)> callback_t;

   public:
    /**
     * Instantiate a dictionary compiler.
     *
     * Note the memory limit only limits the memory used for internal buffers,
     * memory usage for small short-lived objects and the library itself is part of the limit.
     *
     * @param memory_limit memory limit for internal memory usage
     */
    DictionaryCompiler(size_t memory_limit = 1073741824,
                       const compiler_param_t& params = compiler_param_t())
        : initializer_(util::TpieIntializer::getInstance()),
          sorter_(),
          memory_limit_(memory_limit),
          params_(params) {
      sorter_.set_available_memory(memory_limit);
      sorter_.begin();

      if (params_.count(TEMPORARY_PATH_KEY) == 0) {
        params_[TEMPORARY_PATH_KEY] =
            boost::filesystem::temp_directory_path().string();
      }

      TRACE("tmp path set to %s", params_[TEMPORARY_PATH_KEY].c_str());

      // set temp path for tpie
      initializer_.SetTempDirectory(params_[TEMPORARY_PATH_KEY]);

      if (params_.count(STABLE_INSERTS) > 0 && params_[STABLE_INSERTS] == "true") {
        // minimization has to be turned off in this case.
        params_[MINIMIZATION_KEY] = "off";
        stable_insert_ = true;
      }


      value_store_= new ValueStoreT(params_);
    }

    ~DictionaryCompiler(){
      if (generator_) {
        delete generator_;
      } else {
        // if generator was not created we have to delete the value store ourselves
        delete value_store_;
      }
    }

    DictionaryCompiler& operator=(DictionaryCompiler const&) = delete;
    DictionaryCompiler(const DictionaryCompiler& that) = delete;

    void Add(const std::string& input_key, typename ValueStoreT::value_t value =
                 ValueStoreT::no_value) {

      size_of_keys_ += input_key.size();
      sorter_.push(key_value_t(std::move(input_key), RegisterValue(value)));
    }

#ifdef Py_PYTHON_H
    template<typename StringType>
    void __setitem__ (StringType input_key, typename ValueStoreT::value_t value =
        ValueStoreT::no_value) {
      return Add(input_key, value);
    }
#endif

    /**
     * Do the final compilation
     */
    void Compile(callback_t progress_callback = nullptr, void* user_data = nullptr) {
      value_store_->CloseFeeding();
      sorter_.end();
      sorter_.merge_runs();
      CreateGenerator();

      // check that at least 1 item is there
      if (sorter_.can_pull()) {
        number_of_items_ = sorter_.item_count();

        callback_trigger_ = 1+(number_of_items_-1)/100;

        if (callback_trigger_ > 100000) {
          callback_trigger_ = 100000;
        }

        if (!stable_insert_) {

          while (sorter_.can_pull()) {
            key_value_t key_value = sorter_.pull();

            TRACE("adding to generator: %s", key_value.key.c_str());

            generator_->Add(std::move(key_value.key), key_value.value);
            ++added_key_values_;
            if (progress_callback && (added_key_values_ % callback_trigger_ == 0)){
              progress_callback(added_key_values_, number_of_items_, user_data);
            }
          }

        } else {

          // special mode for stable (incremental) inserts, in this case we have to respect the order and take
          // the last value if keys are equal

          key_value_t last_key_value = sorter_.pull();

          while (sorter_.can_pull()) {
            key_value_t key_value = sorter_.pull();

            // dedup with last one wins
            if (last_key_value.key == key_value.key) {
              TRACE("Detected duplicated keys, dedup them, last one wins.");

              // we know that last added values have a lower id (minimization is turned off)
              if (last_key_value.value.value_idx < key_value.value.value_idx) {
                last_key_value = key_value;
              }
              continue;
            }

            TRACE("adding to generator: %s", last_key_value.key.c_str());

            generator_->Add(std::move(last_key_value.key), last_key_value.value);
            ++added_key_values_;
            if (progress_callback && (added_key_values_ % callback_trigger_ == 0)){
              progress_callback(added_key_values_, number_of_items_, user_data);
            }

            last_key_value = key_value;
          }

          // add the last one
          TRACE("adding to generator: %s", last_key_value.key.c_str());

          generator_->Add(std::move(last_key_value.key), last_key_value.value);
          ++added_key_values_;

        }
      }

      generator_->CloseFeeding();
    }

    /**
     * Set a custom manifest to be embedded into the index file.
     *
     * @param manifest as JSON string
     */
    void SetManifestFromString(const std::string& manifest){
      SetManifest(fsa::internal::SerializationUtils::ReadJsonRecord(manifest));
    }

    /**
     * Set a custom manifest to be embedded into the index file.
     *
     * @param manifest
     */
    void SetManifest(const boost::property_tree::ptree& manifest){
      manifest_ = manifest;

      // if generator object is already there, set it otherwise cache it until it is created
      if (generator_) {
        generator_->SetManifest(manifest);
      }
    }

    void Write(std::ostream& stream) {
       generator_->Write(stream);
    }

    template<typename StringType>
    void WriteToFile(StringType filename) {
      std::ofstream out_stream(filename, std::ios::binary);
      generator_->Write(out_stream);
      out_stream.close();
    }

   private:
    util::TpieIntializer& initializer_;
    tpie::serialization_sorter<key_value_t> sorter_;
    size_t memory_limit_;
    fsa::internal::IValueStoreWriter::vs_param_t params_;
    ValueStoreT* value_store_;
    fsa::GeneratorAdapterInterface<PersistenceT, ValueStoreT>* generator_ = nullptr;
    bool sort_finalized_ = false;
    size_t added_key_values_ = 0;
    size_t number_of_items_ = 0;
    size_t callback_trigger_ = 0;

    size_t size_of_keys_ = 0;
    boost::property_tree::ptree manifest_ = boost::property_tree::ptree();
    bool stable_insert_ = false;

    void CreateGenerator();

    /**
     * Register a value before inserting the key(for optimization purposes).
     *
     * @param value The Value
     * @return a handle that later needs to be passed to Add()
     */
    fsa::ValueHandle RegisterValue(typename ValueStoreT::value_t value =
                 ValueStoreT::no_value){

      fsa::ValueHandle handle;
      handle.no_minimization = false;

      handle.value_idx = value_store_->GetValue(value, handle.no_minimization);

      // if inner weights are used update them
      handle.weight = value_store_->GetWeightValue(value);

      return handle;
    }
};

/**
 * Initialize generator based on size of keys and configured memory
 *
 * todo: expose, so that it can be overridden from outside.
 */
template<class PersistenceT, class ValueStoreT>
inline void DictionaryCompiler<PersistenceT, ValueStoreT>::CreateGenerator()
{
  // todo: find good parameters for auto-guessing this
  if (size_of_keys_ > UINT32_MAX){
    if (memory_limit_ > 0x280000000UL /* 10 GB */)  {
      generator_ = new fsa::GeneratorAdapter<PersistenceT, ValueStoreT, uint64_t, int64_t>(memory_limit_, params_, value_store_);
    } else {
      generator_ = new fsa::GeneratorAdapter<PersistenceT, ValueStoreT, uint64_t, int32_t>(memory_limit_, params_, value_store_);
    }
  } else {
    if (memory_limit_ > 0x140000000UL) /* 5GB */ {
      generator_ = new fsa::GeneratorAdapter<PersistenceT, ValueStoreT, uint32_t, int64_t>(memory_limit_, params_, value_store_);
    } else {
      generator_ = new fsa::GeneratorAdapter<PersistenceT, ValueStoreT, uint32_t, int32_t>(memory_limit_, params_, value_store_);
    }
  }

  // set the manifest
  generator_->SetManifest(manifest_);
}



} /* namespace dictionary */
} /* namespace keyvi */

#endif /* DICTIONARY_COMPILER_H_ */
