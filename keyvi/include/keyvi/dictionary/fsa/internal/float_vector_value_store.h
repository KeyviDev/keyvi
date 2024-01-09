/** keyvi - A key value store.
 *
 * Copyright 2021 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_FLOAT_VECTOR_VALUE_STORE_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_FLOAT_VECTOR_VALUE_STORE_H_

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "keyvi/compression/compression_selector.h"
#include "keyvi/dictionary/dictionary_properties.h"
#include "keyvi/dictionary/fsa/internal/ivalue_store.h"
#include "keyvi/dictionary/fsa/internal/lru_generation_cache.h"
#include "keyvi/dictionary/fsa/internal/memory_map_flags.h"
#include "keyvi/dictionary/fsa/internal/memory_map_manager.h"
#include "keyvi/dictionary/fsa/internal/value_store_persistence.h"
#include "keyvi/dictionary/fsa/internal/value_store_properties.h"
#include "keyvi/dictionary/fsa/internal/value_store_types.h"
#include "keyvi/dictionary/util/endian.h"
#include "keyvi/util/configuration.h"
#include "keyvi/util/float_vector_value.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * Value store where the value consists of an array of floats.
 */
class FloatVectorValueStoreBase {
 public:
  using value_t = std::vector<float>;
  static const bool inner_weight = false;

  FloatVectorValueStoreBase() {}

  FloatVectorValueStoreBase(const FloatVectorValueStoreBase& that) = delete;
  FloatVectorValueStoreBase& operator=(FloatVectorValueStoreBase const&) = delete;

  uint64_t AddValue(value_t value, bool* no_minimization) const { return 0; }

  uint32_t GetWeightValue(value_t value) const { return 0; }

  uint32_t GetMergeWeight(uint64_t fsa_value) { return 0; }

  static value_store_t GetValueStoreType() { return value_store_t::FLOAT_VECTOR; }

 protected:
  size_t number_of_values_ = 0;
  size_t number_of_unique_values_ = 0;
  size_t values_buffer_size_ = 0;
};

class FloatVectorValueStoreMinimizationBase : public FloatVectorValueStoreBase {
 public:
  explicit FloatVectorValueStoreMinimizationBase(const keyvi::util::parameters_t& parameters)
      : size_(keyvi::util::mapGet(parameters, VECTOR_SIZE_KEY, DEFAULT_VECTOR_SIZE)),
        hash_(keyvi::util::mapGetMemory(parameters, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_VALUE_STORE)) {
    temporary_directory_ = keyvi::util::mapGetTemporaryPath(parameters);

    temporary_directory_ /=
        boost::filesystem::unique_path("dictionary-fsa-floatvector_value_store-%%%%-%%%%-%%%%-%%%%");
    boost::filesystem::create_directory(temporary_directory_);
    // use memory limit as an indicator for the external memory chunksize
    const size_t external_memory_chunk_size =
        keyvi::util::mapGetMemory(parameters, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_VALUE_STORE);

    TRACE("size: %d", keyvi::util::mapGet(parameters, VECTOR_SIZE_KEY, DEFAULT_VECTOR_SIZE));

    TRACE("External Memory chunk size: %d", external_memory_chunk_size);

    values_extern_.reset(
        new MemoryMapManager(external_memory_chunk_size, temporary_directory_, "floatvector_values_filebuffer"));
  }

  ~FloatVectorValueStoreMinimizationBase() { boost::filesystem::remove_all(temporary_directory_); }

  void CloseFeeding() {
    values_extern_->Persist();
    // free up memory from hashtable
    hash_.Clear();
  }

 protected:
  const size_t size_;
  boost::filesystem::path temporary_directory_;
  std::unique_ptr<MemoryMapManager> values_extern_;
  LeastRecentlyUsedGenerationsCache<RawPointer<>> hash_;
};

class FloatVectorValueStore final : public FloatVectorValueStoreMinimizationBase {
 public:
  using typename FloatVectorValueStoreBase::value_t;

 private:
  using FloatVectorValueStoreBase::number_of_unique_values_;
  using FloatVectorValueStoreBase::number_of_values_;
  using FloatVectorValueStoreBase::values_buffer_size_;
  using FloatVectorValueStoreMinimizationBase::hash_;
  using FloatVectorValueStoreMinimizationBase::size_;
  using FloatVectorValueStoreMinimizationBase::values_extern_;

 public:
  explicit FloatVectorValueStore(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t())
      : FloatVectorValueStoreMinimizationBase(parameters), float_mapped_to_uint32_buffer_(size_) {
    compression_threshold_ = keyvi::util::mapGet(parameters, COMPRESSION_THRESHOLD_KEY, 32);
    std::string compressor = keyvi::util::mapGet<std::string>(parameters, COMPRESSION_KEY, {});
    minimize_ = keyvi::util::mapGetBool(parameters, MINIMIZATION_KEY, true);

    compressor_.reset(compression::compression_strategy(compressor));
    compress_ = std::bind(static_cast<compression::compress_mem_fn_t>(&compression::CompressionStrategy::Compress),
                          compressor_.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  }

  uint64_t AddValue(const value_t& value, bool* no_minimization) {
    if (value.size() != size_) {
      throw std::invalid_argument("value must have " + std::to_string(size_) + " dimensions, configure it using the " +
                                  VECTOR_SIZE_KEY + " parameter");
    }

    keyvi::util::EncodeFloatVector(compress_, &float_mapped_to_uint32_buffer_, &compression_buffer_, value);

    ++number_of_values_;

    if (!minimize_) {
      TRACE("Minimization is turned off.");
      *no_minimization = true;
      return CreateNewValue();
    }

    const RawPointerForCompare<MemoryMapManager> stp(compression_buffer_.data(), compression_buffer_.size(),
                                                     values_extern_.get());
    const RawPointer<> p = hash_.Get(stp);

    if (!p.IsEmpty()) {
      // found the same value again, minimize
      TRACE("Minimized value");
      return p.GetOffset();
    }  // else persist new value

    *no_minimization = true;
    TRACE("New unique value");
    ++number_of_unique_values_;

    uint64_t pt = CreateNewValue();

    TRACE("add value to hash at %d, length %d", pt, compression_buffer_.size());
    hash_.Add(RawPointer<>(pt, stp.GetHashcode(), compression_buffer_.size()));

    return pt;
  }

  void Write(std::ostream& stream) {
    ValueStoreProperties properties(0, values_buffer_size_, number_of_values_, number_of_unique_values_,
                                    compressor_->name());

    properties.WriteAsJsonV2(stream);
    TRACE("Wrote JSON header, stream at %d", stream.tellp());

    values_extern_->Write(stream, values_buffer_size_);
  }

 private:
  std::unique_ptr<compression::CompressionStrategy> compressor_;
  std::function<void(compression::buffer_t*, const char*, size_t)> compress_;
  size_t compression_threshold_;
  bool minimize_ = true;
  std::vector<uint32_t> float_mapped_to_uint32_buffer_;
  compression::buffer_t compression_buffer_;

  uint64_t CreateNewValue() {
    uint64_t pt = static_cast<uint64_t>(values_buffer_size_);
    size_t length;

    keyvi::util::encodeVarInt(compression_buffer_.size(), values_extern_.get(), &length);
    values_buffer_size_ += length;
    values_extern_->Append(reinterpret_cast<const void*>(compression_buffer_.data()), compression_buffer_.size());
    values_buffer_size_ += compression_buffer_.size();

    return pt;
  }
};

class FloatVectorValueStoreMergeBase {
 public:
  using value_t = std::vector<float>;  //, 1>;

  static const bool inner_weight = false;

  FloatVectorValueStoreMergeBase() {}

  FloatVectorValueStoreMergeBase(const FloatVectorValueStoreMergeBase& that) = delete;
  FloatVectorValueStoreMergeBase& operator=(FloatVectorValueStoreMergeBase const&) = delete;

  uint64_t AddValue(value_t value, bool* no_minimization) const { return 0; }

  uint32_t GetWeightValue(value_t value) const { return 0; }

  uint32_t GetMergeWeight(uint64_t fsa_value) { return 0; }

  static value_store_t GetValueStoreType() { return value_store_t::FLOAT_VECTOR; }

 protected:
  size_t number_of_values_ = 0;
  size_t number_of_unique_values_ = 0;
  size_t values_buffer_size_ = 0;
};

class FloatVectorValueStoreMerge final : public FloatVectorValueStoreMergeBase {
 public:
  explicit FloatVectorValueStoreMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t())
      : hash_(keyvi::util::mapGetMemory(parameters, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_VALUE_STORE)) {
    temporary_directory_ = keyvi::util::mapGetTemporaryPath(parameters);

    temporary_directory_ /=
        boost::filesystem::unique_path("dictionary-fsa-floatvector_value_store-%%%%-%%%%-%%%%-%%%%");
    boost::filesystem::create_directory(temporary_directory_);
    // use memory limit as an indicator for the external memory chunksize
    const size_t external_memory_chunk_size =
        keyvi::util::mapGetMemory(parameters, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_VALUE_STORE);

    TRACE("External Memory chunk size: %d", external_memory_chunk_size);

    values_extern_.reset(
        new MemoryMapManager(external_memory_chunk_size, temporary_directory_, "floatvector_values_filebuffer"));
  }

  ~FloatVectorValueStoreMerge() { boost::filesystem::remove_all(temporary_directory_); }

  uint64_t AddValueMerge(const char* payload, uint64_t fsa_value, bool* no_minimization) {
    size_t buffer_size;

    const char* full_buf = payload + fsa_value;
    const char* buf_ptr = keyvi::util::decodeVarIntString(full_buf, &buffer_size);

    const RawPointerForCompare<MemoryMapManager> stp(buf_ptr, buffer_size, values_extern_.get());
    const RawPointer<> p = hash_.Get(stp);

    if (!p.IsEmpty()) {
      // found the same value again, minimize
      TRACE("Minimized value");
      return p.GetOffset();
    }  // else persist string value

    *no_minimization = true;
    TRACE("New unique value");
    ++number_of_unique_values_;

    uint64_t pt = static_cast<uint64_t>(values_buffer_size_);
    size_t full_buf_size = (buf_ptr - full_buf) + buffer_size;

    values_extern_->Append(reinterpret_cast<const void*>(full_buf), full_buf_size);
    values_buffer_size_ += full_buf_size;

    hash_.Add(RawPointer<>(pt, stp.GetHashcode(), buffer_size));

    return pt;
  }

  void Write(std::ostream& stream) {
    ValueStoreProperties properties(0, values_buffer_size_, number_of_values_, number_of_unique_values_, {});

    properties.WriteAsJsonV2(stream);

    values_extern_->Write(stream, values_buffer_size_);
  }

  void CloseFeeding() {
    values_extern_->Persist();
    // free up memory from hashtable
    hash_.Clear();
  }

 private:
  boost::filesystem::path temporary_directory_;
  std::unique_ptr<MemoryMapManager> values_extern_;
  LeastRecentlyUsedGenerationsCache<RawPointer<>> hash_;
};

class FloatVectorValueStoreAppendMerge final : public FloatVectorValueStoreMergeBase {
 public:
  explicit FloatVectorValueStoreAppendMerge(const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t()) {
  }

  explicit FloatVectorValueStoreAppendMerge(const std::vector<std::string>& inputFiles,
                                            const keyvi::util::parameters_t& parameters = keyvi::util::parameters_t())
      : input_files_(inputFiles), offsets_() {
    for (const auto& file_name : inputFiles) {
      properties_.push_back(DictionaryProperties::FromFile(file_name));

      offsets_.push_back(values_buffer_size_);
      number_of_values_ += properties_.back().GetValueStoreProperties().GetNumberOfValues();
      number_of_unique_values_ += properties_.back().GetValueStoreProperties().GetNumberOfUniqueValues();
      values_buffer_size_ += properties_.back().GetValueStoreProperties().GetSize();
    }
  }

  uint64_t AddValueAppendMerge(size_t fileIndex, uint64_t oldIndex) const { return offsets_[fileIndex] + oldIndex; }
  void CloseFeeding() {}

  void Write(std::ostream& stream) {
    // todo: preserve compression
    ValueStoreProperties properties(0, values_buffer_size_, number_of_values_, number_of_unique_values_, {});

    properties.WriteAsJsonV2(stream);
    TRACE("Wrote JSON header, stream at %d", stream.tellp());

    for (size_t i = 0; i < input_files_.size(); ++i) {
      std::ifstream in_stream(input_files_[i]);
      in_stream.seekg(properties_[i].GetValueStoreProperties().GetOffset());
      stream << in_stream.rdbuf();
    }
  }

 private:
  std::vector<std::string> input_files_;
  std::vector<DictionaryProperties> properties_;
  std::vector<size_t> offsets_;
};

class FloatVectorValueStoreReader final : public IValueStoreReader {
 public:
  using IValueStoreReader::IValueStoreReader;

  FloatVectorValueStoreReader(boost::interprocess::file_mapping* file_mapping, const ValueStoreProperties& properties,
                              loading_strategy_types loading_strategy = loading_strategy_types::lazy)
      : IValueStoreReader(file_mapping, properties) {
    const boost::interprocess::map_options_t map_options =
        internal::MemoryMapFlags::ValuesGetMemoryMapOptions(loading_strategy);

    strings_region_ = new boost::interprocess::mapped_region(
        *file_mapping, boost::interprocess::read_only, properties.GetOffset(), properties.GetSize(), 0, map_options);

    const auto advise = internal::MemoryMapFlags::ValuesGetMemoryMapAdvices(loading_strategy);

    strings_region_->advise(advise);

    strings_ = (const char*)strings_region_->get_address();
  }

  ~FloatVectorValueStoreReader() { delete strings_region_; }

  value_store_t GetValueStoreType() const override { return value_store_t::FLOAT_VECTOR; }

  attributes_t GetValueAsAttributeVector(uint64_t fsa_value) const override {
    attributes_t attributes(new attributes_raw_t());

    std::string raw_value = keyvi::util::decodeVarIntString(strings_ + fsa_value);

    (*attributes)["value"] = raw_value;
    return attributes;
  }

  std::string GetRawValueAsString(uint64_t fsa_value) const override {
    return keyvi::util::decodeVarIntString(strings_ + fsa_value);
  }

  std::string GetValueAsString(uint64_t fsa_value) const override {
    TRACE("FloatVectorValueStoreReader GetValueAsString");
    std::string packed_string = keyvi::util::decodeVarIntString(strings_ + fsa_value);

    return keyvi::util::FloatVectorAsString(keyvi::util::DecodeFloatVector(packed_string), ", ");
  }

  void CheckCompatibility(const IValueStoreReader& other) override {
    if (other.GetValueStoreType() != GetValueStoreType()) {
      throw std::invalid_argument("Dictionaries must have the same value store type");
    }

    // compare the dimensions of the 1st vector of each value store
    std::string packed_string = keyvi::util::decodeVarIntString(strings_);
    std::vector<float> v = keyvi::util::DecodeFloatVector(packed_string);

    std::string other_packed_string =
        keyvi::util::decodeVarIntString(dynamic_cast<const FloatVectorValueStoreReader*>(&other)->strings_);
    std::vector<float> other_v = keyvi::util::DecodeFloatVector(other_packed_string);

    if (v.size() != other_v.size()) {
      throw std::invalid_argument("Float Vectors must have the same number of dimensions.");
    }
  }

 private:
  boost::interprocess::mapped_region* strings_region_;
  const char* strings_;

  const char* GetValueStorePayload() const override { return strings_; }
};

template <>
struct ValueStoreComponents<value_store_t::FLOAT_VECTOR> {
  using value_store_writer_t = FloatVectorValueStore;
  using value_store_reader_t = FloatVectorValueStoreReader;
  using value_store_merger_t = FloatVectorValueStoreMerge;
  using value_store_append_merger_t = FloatVectorValueStoreAppendMerge;
};

}  // namespace internal
}  // namespace fsa
}  // namespace dictionary
}  // namespace keyvi

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_FLOAT_VECTOR_VALUE_STORE_H_
