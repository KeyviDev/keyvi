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
 *  vector_generator.h
 *
 *  Created on: March 17, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#ifndef KEYVI_VECTOR_VECTOR_GENERATOR_H_
#define KEYVI_VECTOR_VECTOR_GENERATOR_H_

#include <memory>
#include <string>

#include <boost/filesystem/path.hpp>

#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/dictionary/util/endian.h"
#include "keyvi/util/configuration.h"
#include "keyvi/vector/vector_file.h"

namespace keyvi {
namespace vector {

/**
 * Vector generator
 *
 * @tparam ValueStoreType The type of the value store to use
 * @tparam N Array size for fixed size value vectors, ignored otherwise
 */
template <keyvi::dictionary::fsa::internal::value_store_t ValueStoreType>
class VectorGenerator final {
  static const size_t INDEX_EXTERNAL_MEMORY_CHUNK_SIZE = sizeof(offset_type) * 100 * 1000 * 1000;

  using ValueStoreT =
      typename keyvi::dictionary::fsa::internal::ValueStoreComponents<ValueStoreType>::value_store_writer_t;
  using parameters_t = keyvi::util::parameters_t;

 public:
  explicit VectorGenerator(const parameters_t& params_arg = parameters_t()) {
    parameters_t params = params_arg;
    params[TEMPORARY_PATH_KEY] = keyvi::util::mapGetTemporaryPath(params);
    params[MINIMIZATION_KEY] = "off";

    temporary_directory_ = params[TEMPORARY_PATH_KEY];
    temporary_directory_ /= boost::filesystem::unique_path("keyvi-vector-%%%%-%%%%-%%%%-%%%%");
    boost::filesystem::create_directory(temporary_directory_);

    index_store_.reset(new MemoryMapManager(INDEX_EXTERNAL_MEMORY_CHUNK_SIZE, temporary_directory_, "index-chunk"));
    value_store_.reset(new ValueStoreT(params));
  }

  VectorGenerator& operator=(VectorGenerator const&) = delete;

  VectorGenerator(const VectorGenerator&) = delete;

  ~VectorGenerator() { boost::filesystem::remove_all(temporary_directory_); }

  void PushBack(typename ValueStoreT::value_t value) {
    static_assert(sizeof(offset_type) == 8, "");

    bool dummy_minimization = false;
    const offset_type value_idx = htole64(value_store_->AddValue(value, &dummy_minimization));

    index_store_->Append(&value_idx, sizeof(offset_type));
    ++size_;
  }

  void SetManifest(const std::string& manifest) { manifest_ = manifest; }

  void WriteToFile(const std::string& filename) {
    VectorFile::WriteToFile(filename, manifest_, index_store_, size_, value_store_);
  }

 private:
  boost::filesystem::path temporary_directory_;
  std::unique_ptr<MemoryMapManager> index_store_;
  std::unique_ptr<ValueStoreT> value_store_;
  size_t size_ = 0;

  std::string manifest_;
};

} /* namespace vector */
} /* namespace keyvi */

#endif  // KEYVI_VECTOR_VECTOR_GENERATOR_H_
