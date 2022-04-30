/* * keyvi - A key value store.
 *
 * Copyright 2015, 2016 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * dictionary_merger.h
 *
 *  Created on: Feb 27, 2016
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_DICTIONARY_MERGER_H_
#define KEYVI_DICTIONARY_DICTIONARY_MERGER_H_

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>

#include <msgpack.hpp>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/entry_iterator.h"
#include "keyvi/dictionary/fsa/generator_adapter.h"
#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/dictionary/fsa/internal/value_store_factory.h"
#include "keyvi/dictionary/fsa/segment_iterator.h"
#include "keyvi/util/configuration.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

/**
 * Exception class for dictionary merger
 */

struct merger_exception : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct MergeStats {
  size_t number_of_keys_ = 0;
  size_t deleted_keys_ = 0;
  size_t updated_keys_ = 0;
};

template <keyvi::dictionary::fsa::internal::value_store_t ValueStoreType = fsa::internal::value_store_t::KEY_ONLY>
class DictionaryMerger final {
  using ValueStoreMergeT = typename fsa::internal::ValueStoreComponents<ValueStoreType>::value_store_merger_t;
  using ValueStoreAppendMergeT =
      typename fsa::internal::ValueStoreComponents<ValueStoreType>::value_store_append_merger_t;
  using GeneratorAdapter = fsa::GeneratorAdapterInterface<typename ValueStoreMergeT::value_t>;
  using parameters_t = keyvi::util::parameters_t;

 public:
  /**
   * Instantiate a dictionary merger.
   *
   * @params params merger parameters
   */
  explicit DictionaryMerger(const parameters_t& params = parameters_t())
      : dicts_to_merge_(), params_(params), stats_() {
    params_[TEMPORARY_PATH_KEY] = keyvi::util::mapGetTemporaryPath(params);

    append_merge_ = MERGE_APPEND == keyvi::util::mapGet<std::string>(params_, MERGE_MODE, "");
  }

  void Add(const std::string& filename) {
    if (std::count(inputFiles_.begin(), inputFiles_.end(), filename)) {
      throw std::invalid_argument("File is added already: " + filename);
    }

    fsa::automata_t fsa;
    if (append_merge_) {
      // TODO(hendrik) https://github.com/KeyviDev/keyvi/issues/102
      fsa.reset(new fsa::Automata(std::make_shared<DictionaryProperties>(DictionaryProperties::FromFile(filename)),
                                  loading_strategy_types::lazy, false));
    } else {
      fsa.reset(new fsa::Automata(filename));
    }

    if (fsa->GetValueStoreType() != ValueStoreMergeT::GetValueStoreType()) {
      throw std::invalid_argument("Dictionaries must have the same type.");
    }

    // check if value stores are compatible, TODO: how to check for append merge?
    if (!append_merge_ && dicts_to_merge_.size() > 0) {
      dicts_to_merge_[0]->GetValueStore()->CheckCompatibility(*(fsa->GetValueStore()));
    }

    // check whether dictionary is completely empty
    const auto segment_iterator = fsa::SegmentIterator(fsa::EntryIterator(fsa), segments_pqueue_.size());
    if (!segment_iterator) {
      return;
    }

    // push back deleted keys (list might be empty)
    deleted_keys_.push_back(TryLoadDeletedKeys(filename));

    segments_pqueue_.push(segment_iterator);
    inputFiles_.push_back(filename);
    dicts_to_merge_.push_back(fsa);
  }

  /**
   * Set a custom manifest to be embedded into the index file.
   *
   * @param manifest as string
   */
  void SetManifest(const std::string& manifest) {
    manifest_ = manifest;
    if (generator_) {
      generator_->SetManifest(manifest_);
    }
  }

  void Merge(const std::string& filename) {
    Merge();
    generator_->WriteToFile(filename);
  }

  void Merge() {
    if (append_merge_) {
      AppendMerge();
    } else {
      CompleteMerge();
    }
    generator_->SetManifest(manifest_);
  }

  void Write(std::ostream& stream) {
    if (!generator_) {
      throw merger_exception("not merged yet");
    }

    generator_->Write(stream);
  }

  void WriteToFile(const std::string& filename) {
    if (!generator_) {
      throw merger_exception("not merged yet");
    }
    generator_->WriteToFile(filename);
  }

  const MergeStats& GetStats() const { return stats_; }

 private:
  typename GeneratorAdapter::AdapterPtr generator_;
  bool append_merge_ = false;
  std::vector<fsa::automata_t> dicts_to_merge_;
  std::vector<std::vector<std::string>> deleted_keys_;
  std::vector<std::string> inputFiles_;
  std::priority_queue<fsa::SegmentIterator> segments_pqueue_;
  parameters_t params_;
  std::string manifest_ = std::string();
  MergeStats stats_;

  size_t GetTotalSparseArraySize() const {
    size_t sparse_array_size_sum = 0;
    for (auto fsa : dicts_to_merge_) {
      sparse_array_size_sum += fsa->SparseArraySize();
    }
    return sparse_array_size_sum;
  }

  bool KeyDeleted(size_t segment_index, const std::string& key) {
    if (!deleted_keys_[segment_index].empty() && key == deleted_keys_[segment_index].back()) {
      deleted_keys_[segment_index].pop_back();
      ++stats_.deleted_keys_;
      // check the other deleted_keys for duplicates
      for (auto& deleted_keys : deleted_keys_) {
        if (!deleted_keys.empty() && key == deleted_keys.back()) {
          ++stats_.deleted_keys_;
          deleted_keys.pop_back();
        }
      }
      return true;
    }
    return false;
  }

  void CompleteMerge() {
    ValueStoreMergeT* value_store = new ValueStoreMergeT(params_);
    generator_ =
        GeneratorAdapter::template CreateGenerator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>>(
            GetTotalSparseArraySize(), params_, value_store);

    std::string top_key;

    while (!segments_pqueue_.empty()) {
      auto segment_it = segments_pqueue_.top();
      segments_pqueue_.pop();

      top_key = segment_it.entryIterator().GetKey();

      // check for same keys and merge only the most recent one
      while (!segments_pqueue_.empty() && segments_pqueue_.top().entryIterator().operator==(top_key)) {
        ++stats_.updated_keys_;
        auto to_inc = segments_pqueue_.top();

        segments_pqueue_.pop();
        if (++to_inc) {
          TRACE("push iterator");
          segments_pqueue_.push(to_inc);
        }
      }

      if (KeyDeleted(segment_it.segmentIndex(), top_key) == false) {
        fsa::ValueHandle handle;
        handle.no_minimization_ = false;

        // get the weight value, for now simple: does not require access to the
        // value store itself
        handle.weight_ = value_store->GetMergeWeight(segment_it.entryIterator().GetValueId());
        handle.value_idx_ =
            value_store->AddValueMerge(segment_it.entryIterator().GetFsa()->GetValueStore()->GetValueStorePayload(),
                                       segment_it.entryIterator().GetValueId(), &handle.no_minimization_);

        TRACE("Add key: %s", top_key.c_str());
        ++stats_.number_of_keys_;
        generator_->Add(std::move(top_key), handle);
      }
      if (++segment_it) {
        segments_pqueue_.push(segment_it);
      }
    }
    dicts_to_merge_.clear();
    TRACE("finished iterating, do final compile.");
    generator_->CloseFeeding();
  }

  void AppendMerge() {
    ValueStoreAppendMergeT* value_store = new ValueStoreAppendMergeT(inputFiles_);
    generator_ =
        GeneratorAdapter::template CreateGenerator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>>(
            GetTotalSparseArraySize(), params_, value_store);

    std::string top_key;

    while (!segments_pqueue_.empty()) {
      auto segment_it = segments_pqueue_.top();
      segments_pqueue_.pop();

      top_key = segment_it.entryIterator().GetKey();

      // check for same keys and merge only the most recent one
      while (!segments_pqueue_.empty() && segments_pqueue_.top().entryIterator().operator==(top_key)) {
        ++stats_.updated_keys_;
        auto to_inc = segments_pqueue_.top();

        segments_pqueue_.pop();
        if (++to_inc) {
          TRACE("push iterator");
          segments_pqueue_.push(to_inc);
        }
      }

      if (KeyDeleted(segment_it.segmentIndex(), top_key) == false) {
        fsa::ValueHandle handle;
        handle.no_minimization_ = false;

        // get the weight value, for now simple: does not require access to the
        // value store itself
        handle.weight_ = value_store->GetMergeWeight(segment_it.entryIterator().GetValueId());
        handle.value_idx_ =
            value_store->AddValueAppendMerge(segment_it.segmentIndex(), segment_it.entryIterator().GetValueId());

        TRACE("Add key: %s", top_key.c_str());
        ++stats_.number_of_keys_;
        generator_->Add(std::move(top_key), handle);
      }
      if (++segment_it) {
        segments_pqueue_.push(segment_it);
      }
    }
    dicts_to_merge_.clear();
    TRACE("finished iterating, do final compile.");
    generator_->CloseFeeding();
  }

  /**
   * Load a file with deleted keys if it exists
   */
  std::vector<std::string> TryLoadDeletedKeys(const std::string& filename) {
    std::vector<std::string> deleted_keys;
    boost::filesystem::path deleted_keys_file{filename};
    deleted_keys_file += ".dk";

    TRACE("check for deleted keys file: %s", deleted_keys_file.string().c_str());
    std::ifstream deleted_keys_stream(deleted_keys_file.string(), std::ios::binary);

    if (deleted_keys_stream.good()) {
      TRACE("found deleted keys file");

      {
        // reads the buffer as 1 big chunk, could be improved
        // msgpack v2.x provides a better interface (visitor)
        std::stringstream buffer;
        buffer << deleted_keys_stream.rdbuf();

        msgpack::unpacked unpacked_object;
        msgpack::unpack(unpacked_object, buffer.str().data(), buffer.str().size());

        unpacked_object.get().convert(deleted_keys);
      }

      // sort in reverse order
      std::sort(deleted_keys.begin(), deleted_keys.end(), std::greater<std::string>());
    }

    return deleted_keys;
  }
};

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_DICTIONARY_MERGER_H_
