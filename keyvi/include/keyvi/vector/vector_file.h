/* * keyvi - A key value store.
 *
 * Copyright 2018   Narek Gharibyan<narekgharibyan@gmail.com>
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
 *  vector_file.h
 *
 *  Created on: March 17, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#ifndef KEYVI_VECTOR_VECTOR_FILE_H_
#define KEYVI_VECTOR_VECTOR_FILE_H_

#include <exception>
#include <fstream>
#include <memory>
#include <string>

#include <boost/property_tree/ptree.hpp>

#include "dictionary/fsa/internal/value_store_factory.h"
#include "util/serialization_utils.h"
#include "vector/types.h"

static const char KEYVI_VECTOR_BEGIN[] = "KEYVI_VECTOR_BEGIN";
static const size_t KEYVI_VECTOR_BEGIN_LEN = 18;
static const char KEYVI_VECTOR_END[] = "KEYVI_VECTOR_END";
static const size_t KEYVI_VECTOR_END_LEN = 16;
static const char MANIFEST_LABEL[] = "manifest";
static const char SIZE_LABEL[] = "size";
static const char VALUE_STORE_TYPE_LABEl[] = "value_store_type";

namespace keyvi {
namespace vector {

template <value_store_t>
class Vector;

class VectorFile {
  template <value_store_t>
  friend class Vector;

  using mapped_region = boost::interprocess::mapped_region;
  using IValueStoreReader = dictionary::fsa::internal::IValueStoreReader;

 public:
  explicit VectorFile(const std::string& filename, const value_store_t value_store_type) {
    const auto loading_strategy = dictionary::loading_strategy_types::lazy_no_readahead;
    std::ifstream in_stream(filename, std::ios::binary);

    CheckValidity(&in_stream);

    in_stream.seekg(KEYVI_VECTOR_BEGIN_LEN);
    const auto file_ptree = util::SerializationUtils::ReadJsonRecord(in_stream);
    if (static_cast<int>(value_store_type) !=
        boost::lexical_cast<int>(file_ptree.get<std::string>(VALUE_STORE_TYPE_LABEl))) {
      throw std::invalid_argument("wrong vector file");
    }
    manifest_ = file_ptree.get<std::string>(MANIFEST_LABEL);

    const auto index_ptree = util::SerializationUtils::ReadJsonRecord(in_stream);
    size_ = boost::lexical_cast<size_t>(index_ptree.get<std::string>(SIZE_LABEL));
    const auto index_size = size_ * sizeof(offset_type);

    auto file_mapping = boost::interprocess::file_mapping(filename.c_str(), boost::interprocess::read_only);
    const auto map_options = dictionary::fsa::internal::MemoryMapFlags::FSAGetMemoryMapOptions(loading_strategy);
    index_region_ = mapped_region(file_mapping, boost::interprocess::read_only, in_stream.tellg(), index_size, nullptr,
                                  map_options);
    const auto advise = dictionary::fsa::internal::MemoryMapFlags::FSAGetMemoryMapAdvices(loading_strategy);
    index_region_.advise(advise);

    in_stream.seekg(size_t(in_stream.tellg()) + index_size);
    value_store_reader_.reset(dictionary::fsa::internal::ValueStoreFactory::MakeReader(
        value_store_type, in_stream, &file_mapping, loading_strategy));
  }

  template <typename ValueStoreT>
  static void WriteToFile(const std::string& filename, const std::string& manifest,
                          const std::unique_ptr<MemoryMapManager>& index_store, const size_t size,
                          const std::unique_ptr<ValueStoreT>& value_store) {
    std::ofstream out_stream(filename, std::ios::binary);

    boost::property_tree::ptree file_ptree;
    file_ptree.put("file_version", "1");
    file_ptree.put(MANIFEST_LABEL, manifest);
    file_ptree.put(VALUE_STORE_TYPE_LABEl, static_cast<int>(value_store->GetValueStoreType()));
    file_ptree.put("index_version", "1");

    boost::property_tree::ptree index_ptree;
    index_ptree.put(SIZE_LABEL, std::to_string(size));

    out_stream.write(KEYVI_VECTOR_BEGIN, KEYVI_VECTOR_BEGIN_LEN);
    util::SerializationUtils::WriteJsonRecord(out_stream, file_ptree);
    util::SerializationUtils::WriteJsonRecord(out_stream, index_ptree);

    index_store->Write(out_stream, index_store->GetSize());
    value_store->Write(out_stream);

    out_stream.write(KEYVI_VECTOR_END, KEYVI_VECTOR_END_LEN);

    out_stream.close();
  }

 private:
  void CheckValidity(std::ifstream* in_stream) const {
    if (!in_stream->good()) {
      throw std::invalid_argument("file not found");
    }
    char magic_start[KEYVI_VECTOR_BEGIN_LEN];
    in_stream->read(magic_start, KEYVI_VECTOR_BEGIN_LEN);
    if (std::strncmp(magic_start, KEYVI_VECTOR_BEGIN, KEYVI_VECTOR_BEGIN_LEN)) {
      throw std::invalid_argument("not a keyvi vector file");
    }

    in_stream->seekg(-KEYVI_VECTOR_END_LEN, std::ios_base::end);
    char magic_end[KEYVI_VECTOR_END_LEN];
    in_stream->read(magic_end, KEYVI_VECTOR_END_LEN);
    if (std::strncmp(magic_end, KEYVI_VECTOR_END, KEYVI_VECTOR_END_LEN)) {
      throw std::invalid_argument("the file is corrupt(truncated)");
    }
  }

 private:
  mapped_region index_region_;
  std::unique_ptr<IValueStoreReader> value_store_reader_;

  size_t size_;

  std::string manifest_;
};

} /* namespace vector */
} /* namespace keyvi */

#endif  //  KEYVI_VECTOR_VECTOR_FILE_H_
