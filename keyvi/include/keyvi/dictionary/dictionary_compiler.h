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
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#ifdef _MSC_VER
// workaround for https://github.com/boostorg/sort/issues/43
#include <ciso646>
#endif
#include <boost/sort/sort.hpp>

#include "keyvi/dictionary/dictionary_compiler_common.h"
#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/generator_adapter.h"
#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/dictionary/fsa/internal/null_value_store.h"
#include "keyvi/dictionary/fsa/segment_iterator.h"
#include "keyvi/util/configuration.h"
#include "keyvi/util/os_utils.h"
#include "keyvi/util/serialization_utils.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

/**
 * Dictionary Compiler
 *
 * @tparam ValueStoreType The type of the value store to use
 * @tparam N Array size for fixed size value vectors, ignored otherwise
 */
template <keyvi::dictionary::fsa::internal::value_store_t ValueStoreType = fsa::internal::value_store_t::KEY_ONLY>
class DictionaryCompiler final {
 public:
  using ValueStoreT = typename fsa::internal::ValueStoreComponents<ValueStoreType>::value_store_writer_t;

 private:
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
  explicit DictionaryCompiler(const keyvi::util::parameters_t& params = keyvi::util::parameters_t()) : params_(params) {
    temporary_directory_ = keyvi::util::mapGetTemporaryPath(params);
    temporary_directory_ /= boost::filesystem::unique_path("keyvi-fsa-chunks-%%%%-%%%%-%%%%-%%%%");

    TRACE("tmp path set to %s", params_[TEMPORARY_PATH_KEY].c_str());

    memory_limit_ = keyvi::util::mapGetMemory(params_, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_COMPILER);

    if (memory_limit_ < 1024 * 1024) {
      throw compiler_exception("Memory limit must be at least 1MB");
    }

    parallel_sort_threshold_ =
        keyvi::util::mapGet(params_, PARALLEL_SORT_THRESHOLD_KEY, DEFAULT_PARALLEL_SORT_THRESHOLD);

    value_store_ = new ValueStoreT(params_);
  }

  ~DictionaryCompiler() {
    if (!generator_) {
      // if generator was not created we have to delete the value store
      // ourselves
      delete value_store_;
    }
    if (chunk_ > 0) {
      boost::filesystem::remove_all(temporary_directory_);
    }
  }

  DictionaryCompiler& operator=(DictionaryCompiler const&) = delete;
  DictionaryCompiler(const DictionaryCompiler& that) = delete;

  void Add(const std::string& input_key, typename ValueStoreT::value_t value = ValueStoreT::no_value) {
    if (generator_) {
      throw compiler_exception("You're not supposed to add more data once compilation is done!");
    }

    size_of_keys_ += input_key.size();

    memory_estimate_ += EstimateMemory(input_key);
    // no move, we have no ownership
    key_values_.push_back(key_value_t(input_key, RegisterValue(value)));
    TriggerSortAndChunkGenerationIfNeeded();
  }

  /**
   * Do the final compilation
   */
  void Compile(callback_t progress_callback = nullptr, void* user_data = nullptr) {
    value_store_->CloseFeeding();

    if (chunk_ == 0) {
      CompileSingleChunk(progress_callback, user_data);
    } else {
      CompileByMergingChunks(progress_callback, user_data);
    }

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
  key_values_t key_values_;
  ValueStoreT* value_store_;
  typename GeneratorAdapter::AdapterPtr generator_;
  std::string manifest_;
  size_t memory_limit_;
  size_t memory_estimate_ = 0;
  size_t chunk_ = 0;
  size_t size_of_keys_ = 0;
  size_t parallel_sort_threshold_;
  boost::filesystem::path temporary_directory_;

  inline void Sort() {
    if (key_values_.size() > parallel_sort_threshold_ && parallel_sort_threshold_ != 0) {
      boost::sort::block_indirect_sort(key_values_.begin(), key_values_.end());
    } else {
      std::sort(key_values_.begin(), key_values_.end());
    }
  }

  inline void TriggerSortAndChunkGenerationIfNeeded() {
    if (memory_estimate_ < memory_limit_) {
      return;
    }
    CreateChunk();
  }

  inline void CreateChunk() {
    TRACE("create chunk %ul", key_values_.size());
    if (chunk_ == 0) {
      boost::filesystem::create_directory(temporary_directory_);
    }

    Sort();

    // disable minimization for faster compile
    keyvi::util::parameters_t params(params_);
    params[MINIMIZATION_KEY] = "off";
    fsa::Generator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>, fsa::internal::NullValueStore,
                   uint32_t, int32_t>
        generator(params);

    for (const key_value_t& key_value : key_values_) {
      TRACE("adding to generator: %s", key_value.key.c_str());
      generator.Add(key_value.key, key_value.value);
    }

    key_values_.clear();
    memory_estimate_ = 0;
    generator.CloseFeeding();

    boost::filesystem::path filename(temporary_directory_);
    filename /= "fsa_";
    filename += std::to_string(chunk_);
    TRACE("write chunk to %s", filename.string().c_str());
    generator.WriteToFile(filename.string());
    ++chunk_;
  }

  inline void CompileSingleChunk(callback_t progress_callback = nullptr, void* user_data = nullptr) {
    size_t added_key_values = 0;
    size_t callback_trigger = 0;
    Sort();

    generator_ =
        GeneratorAdapter::template CreateGenerator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>>(
            size_of_keys_, params_, value_store_);

    if (key_values_.size() > 0) {
      size_t number_of_items = key_values_.size();

      callback_trigger = 1 + (number_of_items - 1) / 100;

      if (callback_trigger > 100000) {
        callback_trigger = 100000;
      }

      for (const key_value_t& key_value : key_values_) {
        TRACE("adding to generator: %s", key_value.key.c_str());

        generator_->Add(key_value.key, key_value.value);
        ++added_key_values;
        if (progress_callback && (added_key_values % callback_trigger == 0)) {
          progress_callback(added_key_values, number_of_items, user_data);
        }
      }
      key_values_.clear();
    }
    generator_->CloseFeeding();
  }

  inline void CompileByMergingChunks(callback_t progress_callback = nullptr, void* user_data = nullptr) {
    size_t added_key_values = 0;
    size_t callback_trigger = 0;
    size_t number_of_items = 0;

    // create the last chunk
    if (key_values_.size() > 0) {
      CreateChunk();
    }

    TRACE("merge chunks");
    keyvi::util::parameters_t params;

    std::priority_queue<fsa::SegmentIterator> segments_pqueue;

    // add all chunks
    for (size_t i = 0; i < chunk_; ++i) {
      boost::filesystem::path filename(temporary_directory_);
      filename /= "fsa_";
      filename += std::to_string(i);

      TRACE("add for merge %s", filename.string().c_str());

      // todo: make shared
      fsa::automata_t fsa(new fsa::Automata(filename.string()));
      segments_pqueue.emplace(fsa::EntryIterator(fsa), segments_pqueue.size());
      number_of_items += fsa->GetNumberOfKeys();
    }

    callback_trigger = 1 + (number_of_items - 1) / 100;

    if (callback_trigger > 100000) {
      callback_trigger = 100000;
    }

    generator_ =
        GeneratorAdapter::template CreateGenerator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>>(
            size_of_keys_, params_, value_store_);

    std::string top_key;
    while (!segments_pqueue.empty()) {
      auto segment_it = segments_pqueue.top();
      segments_pqueue.pop();

      top_key = segment_it.entryIterator().GetKey();

      // check for same keys and merge only the most recent one
      while (!segments_pqueue.empty() && segments_pqueue.top().entryIterator().operator==(top_key)) {
        auto to_inc = segments_pqueue.top();

        segments_pqueue.pop();
        if (++to_inc) {
          TRACE("push iterator");
          segments_pqueue.push(to_inc);
        }

        ++added_key_values;
        if (progress_callback && (added_key_values % callback_trigger == 0)) {
          progress_callback(added_key_values, number_of_items, user_data);
        }
      }
      fsa::ValueHandle handle;
      handle.no_minimization_ = false;

      // get the weight value, for now simple: does not require access to the
      // value store itself
      handle.weight_ = value_store_->GetMergeWeight(segment_it.entryIterator().GetValueId());
      handle.value_idx_ = segment_it.entryIterator().GetValueId();

      TRACE("Add key: %s", top_key.c_str());
      generator_->Add(std::move(top_key), handle);

      if (++segment_it) {
        segments_pqueue.push(segment_it);
      }
      ++added_key_values;
      if (progress_callback && (added_key_values % callback_trigger == 0)) {
        progress_callback(added_key_values, number_of_items, user_data);
      }
    }

    // free up disk space as early as possible
    boost::filesystem::remove_all(temporary_directory_);
    chunk_ = 0;
    generator_->CloseFeeding();
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

#endif  // KEYVI_DICTIONARY_DICTIONARY_COMPILER_H_
