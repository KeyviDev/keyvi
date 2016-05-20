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
 * json_value_store.h
 *
 *  Created on: October 13, 2015
 *      Author: David Mark Nemeskey<nemeskey.david@gmail.com>
 */

#ifndef JSON_VALUE_STORE_H_
#define JSON_VALUE_STORE_H_

#include "dictionary/fsa/internal/intrinsics.h"
#if defined(KEYVI_SSE42)
#define RAPIDJSON_SSE42
#endif

#include <functional>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "dictionary/fsa/internal/ivalue_store.h"
#include "dictionary/fsa/internal/memory_map_flags.h"
#include "dictionary/fsa/internal/serialization_utils.h"
#include "dictionary/fsa/internal/lru_generation_cache.h"
#include "dictionary/fsa/internal/memory_map_manager.h"
#include "dictionary/fsa/internal/value_store_persistence.h"
#include "dictionary/dictionary_merger_fwd.h"

#include "msgpack.hpp"
// from 3rdparty/xchange: msgpack <-> rapidjson converter
#include "msgpack/type/rapidjson.hpp"
#include "msgpack/zbuffer.hpp"

#include "dictionary/util/json_value.h"
#include "dictionary/fsa/internal/constants.h"
#include "compression/compression_selector.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * Value store where the value consists of a single integer.
 */
class JsonValueStore final : public IValueStoreWriter {
 public:
  typedef std::string value_t;
  static const uint64_t no_value = 0;
  static const bool inner_weight = false;

  JsonValueStore(const vs_param_t& parameters = vs_param_t(), size_t memory_limit = 104857600)
      : IValueStoreWriter(parameters), hash_(memory_limit) {
    temporary_directory_ = parameters_[TEMPORARY_PATH_KEY];
    temporary_directory_ /= boost::filesystem::unique_path(
        "dictionary-fsa-json_value_store-%%%%-%%%%-%%%%-%%%%");
    boost::filesystem::create_directory(temporary_directory_);

    if (parameters_.count(COMPRESSION_THRESHOLD_KEY) > 0) {
      compression_threshold_ = boost::lexical_cast<size_t>(
          parameters_[COMPRESSION_THRESHOLD_KEY]);
    } else {
      compression_threshold_ = 32;
    }

    std::string compressor;
    if (parameters_.count(COMPRESSION_KEY) > 0) {
      compressor = parameters_[COMPRESSION_KEY];
    }

    if (parameters_.count(MINIMIZATION_KEY) > 0 && parameters_[MINIMIZATION_KEY] == "off") {
      minimize_ = false;
    }

    compressor_.reset(compression::compression_strategy(compressor));
    raw_compressor_.reset(compression::compression_strategy("raw"));
    // This is beyond ugly, but needed for EncodeJsonValue :(
    long_compress_ = std::bind(static_cast<compression::compress_mem_fn_t>
        (&compression::CompressionStrategy::Compress),
        compressor_.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    short_compress_ = std::bind(static_cast<compression::compress_mem_fn_t>
        (&compression::CompressionStrategy::Compress),
        raw_compressor_.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    size_t external_memory_chunk_size = 1073741824;

    values_extern_ = new MemoryMapManager(external_memory_chunk_size,
                                          temporary_directory_,
                                          "json_values_filebuffer");
  }

  ~JsonValueStore() {
    delete values_extern_;
    boost::filesystem::remove_all(temporary_directory_);
  }

  JsonValueStore& operator=(JsonValueStore const&) = delete;
  JsonValueStore(const JsonValueStore& that) = delete;

  /**
   * Simple implementation of a value store for json values:
   * todo: performance improvements?
   */
  uint64_t GetValue(const value_t& value, bool& no_minimization) {
    msgpack_buffer_.clear();

    util::EncodeJsonValue(
        long_compress_, short_compress_, msgpack_buffer_,
        string_buffer_, value,
        compression_threshold_);

    ++number_of_values_;

    if (!minimize_){
      TRACE("Minimization is turned off.");
      no_minimization = true;
      return AddValue();
    }

    const RawPointerForCompare<MemoryMapManager> stp(string_buffer_.data(), string_buffer_.size(),
                                                     values_extern_);
    const RawPointer<> p = hash_.Get(stp);

    if (!p.IsEmpty()) {
      // found the same value again, minimize
      TRACE("Minimized value");
      return p.GetOffset();
    } // else persist string value

    no_minimization = true;
    TRACE("New unique value");
    ++number_of_unique_values_;

    uint64_t pt = AddValue();

    TRACE("add value to hash at %d, length %d", pt, string_buffer_.size());
    hash_.Add(RawPointer<>(pt, stp.GetHashcode(), string_buffer_.size()));

    return pt;
  }

  uint32_t GetWeightValue(value_t value) const {
    return 0;
  }

  static value_store_t GetValueStoreType() {
    return JSON_VALUE_STORE;
  }

  /**
   * Close the value store, so no more updates;
   */
  void CloseFeeding() {
    values_extern_->Persist();
    // free up memory from hashtable
    hash_.Clear();
  }

  void Write(std::ostream& stream) {
    boost::property_tree::ptree pt;
    pt.put("size", std::to_string(values_buffer_size_));
    pt.put("values", std::to_string(number_of_values_));
    pt.put("unique_values", std::to_string(number_of_unique_values_));
    pt.put(std::string("__") + COMPRESSION_KEY, compressor_->name());
    pt.put(std::string("__") + COMPRESSION_THRESHOLD_KEY, compression_threshold_);

    internal::SerializationUtils::WriteJsonRecord(stream, pt);
    TRACE("Wrote JSON header, stream at %d", stream.tellp());

    values_extern_->Write(stream, values_buffer_size_);
  }

 private:
  MemoryMapManager* values_extern_;

  /*
   * Compressors & the associated compression functions. Ugly, but
   * needed for EncodeJsonValue.
   */
  std::unique_ptr<compression::CompressionStrategy> compressor_;
  std::unique_ptr<compression::CompressionStrategy> raw_compressor_;
  std::function<void (compression::buffer_t&, const char*, size_t)> long_compress_;
  std::function<void (compression::buffer_t&, const char*, size_t)> short_compress_;
  size_t compression_threshold_;
  bool minimize_ = true;

  LeastRecentlyUsedGenerationsCache<RawPointer<>> hash_;
  compression::buffer_t string_buffer_;
  msgpack::sbuffer msgpack_buffer_;
  size_t number_of_values_ = 0;
  size_t number_of_unique_values_ = 0;
  size_t values_buffer_size_ = 0;
  boost::filesystem::path temporary_directory_;

 private:

   template<typename , typename>
   friend class ::keyvi::dictionary::DictionaryMerger;

   uint64_t GetValue(const char* payload, uint64_t fsa_value, bool& no_minimization){
     size_t buffer_size;

     const char* full_buf = payload + fsa_value;
     const char* buf_ptr = util::decodeVarintString(full_buf, &buffer_size);

     const RawPointerForCompare<MemoryMapManager> stp(buf_ptr, buffer_size,
                                                          values_extern_);
     const RawPointer<> p = hash_.Get(stp);

     if (!p.IsEmpty()) {
       // found the same value again, minimize
       TRACE("Minimized value");
       return p.GetOffset();
     } // else persist string value

     no_minimization = true;
     TRACE("New unique value");
     ++number_of_unique_values_;

     uint64_t pt = static_cast<uint64_t>(values_buffer_size_);
     size_t full_buf_size = (buf_ptr-full_buf) + buffer_size;

     values_extern_->Append((void*)full_buf,
                                full_buf_size);
     values_buffer_size_ += full_buf_size;

     hash_.Add(RawPointer<>(pt, stp.GetHashcode(), buffer_size));

     return pt;
   }

  uint64_t AddValue(){
    uint64_t pt = static_cast<uint64_t>(values_buffer_size_);
    size_t length;

    dictionary::util::encodeVarint(string_buffer_.size(), *values_extern_, &length);
    values_buffer_size_ += length;
    values_extern_->Append((void*)string_buffer_.data(),
                           string_buffer_.size());
    values_buffer_size_ += string_buffer_.size();

    return pt;
  }

};


class JsonValueStoreReader final: public IValueStoreReader {
 public:
  using IValueStoreReader::IValueStoreReader;

  JsonValueStoreReader(std::istream& stream,
                       boost::interprocess::file_mapping* file_mapping,
                       loading_strategy_types loading_strategy = loading_strategy_types::lazy)
      : IValueStoreReader(stream, file_mapping) {
    TRACE("JsonValueStoreReader construct");

    properties_ =
        internal::SerializationUtils::ReadJsonRecord(stream);

    size_t offset = stream.tellg();
    size_t strings_size = boost::lexical_cast<size_t> (properties_.get<std::string>("size"));

    // check for file truncation
    if (strings_size > 0) {
      stream.seekg(strings_size - 1, stream.cur);
      if (stream.peek() == EOF) {
        throw std::invalid_argument("file is corrupt(truncated)");
      }
    }

    const boost::interprocess::map_options_t map_options = internal::MemoryMapFlags::ValuesGetMemoryMapOptions(loading_strategy);

    strings_region_ = new boost::interprocess::mapped_region(
        *file_mapping, boost::interprocess::read_only, offset,
        strings_size, 0, map_options);

    const auto advise = internal::MemoryMapFlags::ValuesGetMemoryMapAdvices(loading_strategy);

    strings_region_->advise(advise);

    strings_ = (const char*) strings_region_->get_address();
  }

  ~JsonValueStoreReader() {
    delete strings_region_;
  }

  virtual value_store_t GetValueStoreType() const override {
      return JSON_VALUE_STORE;
  }

  virtual attributes_t GetValueAsAttributeVector(uint64_t fsa_value) const override {
    attributes_t attributes(new attributes_raw_t());

    std::string raw_value = util::decodeVarintString(strings_ + fsa_value);

    //auto length = util::decodeVarint((uint8_t*) strings_ + fsa_value);
    //std::string raw_value(strings_ + fsa_value, length);

    (*attributes)["value"] = raw_value;
    return attributes;
  }

  virtual std::string GetRawValueAsString(uint64_t fsa_value) const override {
    return util::decodeVarintString(strings_ + fsa_value);
  }

  virtual std::string GetValueAsString(uint64_t fsa_value) const override {
    TRACE("JsonValueStoreReader GetValueAsString");
    std::string packed_string = util::decodeVarintString(strings_ + fsa_value);

    return util::DecodeJsonValue(packed_string);
  }

  virtual std::string GetStatistics() const override {
    std::ostringstream buf;
    boost::property_tree::write_json (buf, properties_, false);
    return buf.str();
  }

 private:
  boost::interprocess::mapped_region* strings_region_;
  const char* strings_;
  boost::property_tree::ptree properties_;

  virtual const char* GetValueStorePayload() const override {
    return strings_;
  }
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* JSON_VALUE_STORE_H_ */
