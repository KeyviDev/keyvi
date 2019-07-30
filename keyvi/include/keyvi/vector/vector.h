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
 *  vector.h
 *
 *  Created on: March 17, 2018
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#ifndef KEYVI_VECTOR_VECTOR_H_
#define KEYVI_VECTOR_VECTOR_H_

#include <exception>
#include <string>

#include "keyvi/dictionary/util/endian.h"
#include "keyvi/vector/vector_file.h"

namespace keyvi {
namespace vector {

template <value_store_t value_store_type>
class Vector final {
 public:
  explicit Vector(const std::string& filename)
      : vector_file_(filename, value_store_type),
        index_ptr_(static_cast<offset_type*>(vector_file_.index_region_.get_address())) {}

  std::string Get(const size_t index) const {
    if (index >= vector_file_.size_) {
      throw std::out_of_range("out of range access");
    }
    const offset_type offset = le64toh(index_ptr_[index]);
    return vector_file_.value_store_reader_->GetValueAsString(offset);
  }

  size_t Size() const { return vector_file_.size_; }

  std::string Manifest() const { return vector_file_.manifest_; }

 private:
  const VectorFile vector_file_;
  const offset_type* const index_ptr_;
};

} /* namespace vector */
} /* namespace keyvi */

#endif  //  KEYVI_VECTOR_VECTOR_H_
