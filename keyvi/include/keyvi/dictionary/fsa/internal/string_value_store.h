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
 * string_value_store.h
 *
 *  Created on: Jul 16, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_STRING_VALUE_STORE_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_STRING_VALUE_STORE_H_

#include <string>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

#include "dictionary/fsa/internal/ivalue_store.h"
#include "dictionary/fsa/internal/lru_generation_cache.h"
#include "dictionary/fsa/internal/memory_map_flags.h"
#include "dictionary/fsa/internal/memory_map_manager.h"
#include "dictionary/fsa/internal/minimization_hash.h"
#include "dictionary/fsa/internal/value_store_persistence.h"
#include "dictionary/fsa/internal/value_store_types.h"
#include "dictionary/keyvi_file.h"
#include "util/serialization_utils.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

class StringValueStoreBase {
 public:
  typedef std::string value_t;
  static const std::string no_value;
  static const bool inner_weight = false;

  StringValueStoreBase() {}

  StringValueStoreBase& operator=(StringValueStoreBase const&) = delete;
  StringValueStoreBase(const StringValueStoreBase& that) = delete;

  uint32_t GetWeightValue(value_t value) const { return 0; }

  uint32_t GetMergeWeight(uint64_t fsa_value) { return 0; }

  uint64_t AddValue(const value_t& value, bool* no_minimization) { return 0; }

  static value_store_t GetValueStoreType() { return value_store_t::STRING; }

 protected:
  size_t number_of_values_ = 0;
  size_t number_of_unique_values_ = 0;
  size_t values_buffer_size_ = 0;
};

class StringValueStoreMinimizationBase : public StringValueStoreBase {
 public:
  explicit StringValueStoreMinimizationBase(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t())
      : parameters_(parameters),
        hash_(keyvi::util::mapGetMemory(parameters, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_VALUE_STORE)) {
    temporary_directory_ = parameters_[TEMPORARY_PATH_KEY];

    temporary_directory_ /= boost::filesystem::unique_path("dictionary-fsa-string_value_store-%%%%-%%%%-%%%%-%%%%");
    boost::filesystem::create_directory(temporary_directory_);
    // use memory limit as an indicator for the external memory chunksize
    const size_t external_memory_chunk_size =
        keyvi::util::mapGetMemory(parameters, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_VALUE_STORE);

    TRACE("External Memory chunk size: %d", external_memory_chunk_size);

    values_extern_.reset(
        new MemoryMapManager(external_memory_chunk_size, temporary_directory_, "string_values_filebuffer"));
  }

  ~StringValueStoreMinimizationBase() { boost::filesystem::remove_all(temporary_directory_); }

  /**
   * Close the value store, so no more updates;
   */
  void CloseFeeding() {
    values_extern_->Persist();
    // free up memory from hashtable
    hash_.Clear();
  }

 protected:
  keyvi::util::parameters_t parameters_;
  boost::filesystem::path temporary_directory_;
  std::unique_ptr<MemoryMapManager> values_extern_;
  LeastRecentlyUsedGenerationsCache<RawPointer<>> hash_;
};

/**
 * Value store where the value consists of a string.
 */
class StringValueStore final : public StringValueStoreMinimizationBase {
 public:
  explicit StringValueStore(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t())
      : StringValueStoreMinimizationBase(parameters) {}

  /**
   * Simple implementation of a value store for strings:
   * todo: performance improvements / port stuff from json_value_store
   */
  uint64_t AddValue(const value_t& value, bool* no_minimization) {
    const RawPointerForCompareString<MemoryMapManager> stp(value.data(), value.size(), values_extern_.get());

    const RawPointer<> p = hash_.Get(stp);

    if (!p.IsEmpty()) {
      // found the same value again, minimize
      return p.GetOffset();
    }

    *no_minimization = true;

    // else persist string value
    uint64_t pt = static_cast<uint64_t>(values_buffer_size_);

    values_extern_->Append(value.data(), value.size());
    values_buffer_size_ += value.size();

    // add zero termination
    values_extern_->push_back('\0');
    ++values_buffer_size_;

    hash_.Add(RawPointer<>(pt, stp.GetHashcode(), value.size()));

    return pt;
  }

  uint32_t GetWeightValue(value_t value) const { return 0; }

  void Write(std::ostream& stream) const {
    boost::property_tree::ptree pt;
    pt.put("size", std::to_string(values_buffer_size_));
    pt.put("values", std::to_string(number_of_values_));
    pt.put("unique_values", std::to_string(number_of_unique_values_));

    keyvi::util::SerializationUtils::WriteJsonRecord(stream, pt);
    TRACE("Wrote JSON header, stream at %d", stream.tellp());

    values_extern_->Write(stream, values_buffer_size_);
  }
};

class StringValueStoreMerge final : public StringValueStoreMinimizationBase {
 public:
  explicit StringValueStoreMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  uint64_t AddValueMerge(const char* payload, uint64_t fsa_value, bool* no_minimization) {
    const char* value = payload + fsa_value;
    const size_t value_size = std::strlen(value);
    const RawPointerForCompareString<MemoryMapManager> stp(value, value_size, values_extern_.get());

    const RawPointer<> p = hash_.Get(stp);

    if (!p.IsEmpty()) {
      // found the same value again, minimize
      return p.GetOffset();
    }

    *no_minimization = true;

    // else persist string value
    uint64_t pt = static_cast<uint64_t>(values_buffer_size_);

    values_extern_->Append(value, value_size);
    values_buffer_size_ += value_size;

    // add zero termination
    values_extern_->push_back('\0');
    ++values_buffer_size_;

    hash_.Add(RawPointer<>(pt, stp.GetHashcode(), value_size));

    return pt;
  }

  void Write(std::ostream& stream) {
    boost::property_tree::ptree pt;
    pt.put("size", std::to_string(values_buffer_size_));
    pt.put("values", std::to_string(number_of_values_));
    pt.put("unique_values", std::to_string(number_of_unique_values_));

    keyvi::util::SerializationUtils::WriteJsonRecord(stream, pt);
    TRACE("Wrote JSON header, stream at %d", stream.tellp());

    values_extern_->Write(stream, values_buffer_size_);
  }
};

class StringValueStoreAppendMerge final : public StringValueStoreBase {
 public:
  explicit StringValueStoreAppendMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {}

  explicit StringValueStoreAppendMerge(const std::vector<std::string>& inputFiles,
                                       const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t())
      : input_files_(inputFiles), offsets_() {
    for (const auto& filename : inputFiles) {
      KeyViFile keyvi_file(filename);

      auto& vsStream = keyvi_file.valueStoreStream();
      const boost::property_tree::ptree props = keyvi::util::SerializationUtils::ReadValueStoreProperties(vsStream);
      offsets_.push_back(values_buffer_size_);

      number_of_values_ += boost::lexical_cast<size_t>(props.get<std::string>("values"));
      number_of_unique_values_ += boost::lexical_cast<size_t>(props.get<std::string>("unique_values"));
      values_buffer_size_ += boost::lexical_cast<size_t>(props.get<std::string>("size"));
    }
  }

  uint64_t AddValueAppendMerge(size_t fileIndex, uint64_t oldIndex) const { return offsets_[fileIndex] + oldIndex; }

  void CloseFeeding() {}

  void Write(std::ostream& stream) {
    boost::property_tree::ptree pt;
    pt.put("size", std::to_string(values_buffer_size_));
    pt.put("values", std::to_string(number_of_values_));
    pt.put("unique_values", std::to_string(number_of_unique_values_));

    keyvi::util::SerializationUtils::WriteJsonRecord(stream, pt);

    for (const auto& filename : input_files_) {
      KeyViFile keyvi_file(filename);
      auto& in_stream = keyvi_file.valueStoreStream();
      keyvi::util::SerializationUtils::ReadValueStoreProperties(in_stream);

      stream << in_stream.rdbuf();
    }
  }

 private:
  std::vector<std::string> input_files_;
  std::vector<size_t> offsets_;
};

class StringValueStoreReader final : public IValueStoreReader {
 public:
  using IValueStoreReader::IValueStoreReader;

  StringValueStoreReader(std::istream& stream, boost::interprocess::file_mapping* file_mapping,
                         loading_strategy_types loading_strategy = loading_strategy_types::lazy)
      : IValueStoreReader(stream, file_mapping) {
    const boost::property_tree::ptree properties = keyvi::util::SerializationUtils::ReadValueStoreProperties(stream);

    const size_t offset = stream.tellg();
    const size_t strings_size = boost::lexical_cast<size_t>(properties.get<std::string>("size"));

    const boost::interprocess::map_options_t map_options =
        internal::MemoryMapFlags::ValuesGetMemoryMapOptions(loading_strategy);

    strings_region_ = new boost::interprocess::mapped_region(*file_mapping, boost::interprocess::read_only, offset,
                                                             strings_size, 0, map_options);

    const auto advise = internal::MemoryMapFlags::ValuesGetMemoryMapAdvices(loading_strategy);

    strings_region_->advise(advise);

    strings_ = (const char*)strings_region_->get_address();
  }

  ~StringValueStoreReader() { delete strings_region_; }

  value_store_t GetValueStoreType() const override { return value_store_t::STRING; }

  attributes_t GetValueAsAttributeVector(uint64_t fsa_value) const override {
    attributes_t attributes(new attributes_raw_t());

    std::string raw_value(strings_ + fsa_value);

    (*attributes)["value"] = raw_value;
    return attributes;
  }

  std::string GetValueAsString(uint64_t fsa_value) const override { return std::string(strings_ + fsa_value); }

 private:
  boost::interprocess::mapped_region* strings_region_;
  const char* strings_;

  const char* GetValueStorePayload() const override { return strings_; }
};

template <>
struct ValueStoreComponents<value_store_t::STRING> {
  using value_store_writer_t = StringValueStore;
  using value_store_reader_t = StringValueStoreReader;
  using value_store_merger_t = StringValueStoreMerge;
  using value_store_append_merger_t = StringValueStoreAppendMerge;
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_STRING_VALUE_STORE_H_
