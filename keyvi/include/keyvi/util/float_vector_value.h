/** keyvi - A key value store.
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

#ifndef KEYVI_UTIL_FLOAT_VECTOR_VALUE_H_
#define KEYVI_UTIL_FLOAT_VECTOR_VALUE_H_

#include <algorithm>
#include <array>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "keyvi/compression/compression_selector.h"
#include "keyvi/dictionary/util/endian.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace util {

inline std::vector<float> DecodeFloatVector(const std::string& encoded_value) {
  compression::decompress_func_t decompressor = compression::decompressor_by_code(encoded_value);
  std::string unompressed_string_value = decompressor(encoded_value);

  const size_t vector_size = unompressed_string_value.size() / sizeof(uint32_t);
  std::vector<float> float_vector(vector_size);

#ifdef KEYVI_LITTLE_ENDIAN
  const float* float_vector_le = reinterpret_cast<const float*>(unompressed_string_value.data());

  for (size_t i = 0; i < vector_size; ++i) {
    float_vector[i] = float_vector_le[i];
  }
#else
  const uint32_t* float_vector_le = reinterpret_cast<const uint32_t*>(unompressed_string_value.data());

  union {
    float f;
    uint32_t i;
  } mem;

  for (size_t i = 0; i < vector_size; ++i) {
    mem.i = le32toh(float_vector_le[i]);
    float_vector[i] = mem.f;
  }
#endif

  return float_vector;
}

inline void EncodeFloatVector(std::function<void(compression::buffer_t*, const char*, size_t)> compress,
                              std::vector<uint32_t>* helper_buffer, compression::buffer_t* compression_buffer,
                              std::vector<float> value) {
  // do not copy if little endian
#ifdef KEYVI_LITTLE_ENDIAN
  compress(compression_buffer, reinterpret_cast<const char*>(value.data()), value.size() * sizeof(uint32_t));
#else
  union {
    float f;
    uint32_t i;
  } mem;

  for (size_t i = 0; i < helper_buffer->size(); ++i) {
    mem.f = value[i];
    helper_buffer->operator[](i) = htole32(mem.i);
  }

  // compression
  compress(compression_buffer, reinterpret_cast<const char*>(helper_buffer->data()),
           helper_buffer->size() * sizeof(uint32_t));

#endif
}

inline std::string EncodeFloatVector(std::vector<float> value, size_t size) {
  std::vector<uint32_t> helper_buffer(size);
  compression::buffer_t compression_buffer;

  EncodeFloatVector(static_cast<void (*)(compression::buffer_t*, const char*, size_t)>(
                        &compression::RawCompressionStrategy::DoCompress),
                    &helper_buffer, &compression_buffer, value);
  return std::string(reinterpret_cast<char*>(compression_buffer.data()), compression_buffer.size());
}

inline std::string FloatVectorAsString(const std::vector<float> float_vector, const std::string delimiter) {
  std::stringstream s;
  if (float_vector.size() == 0) {
    return "";
  }
  std::copy(float_vector.begin(), float_vector.end() - 1, std::ostream_iterator<float>(s, delimiter.c_str()));
  s << float_vector.back();
  return s.str();
}

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_FLOAT_VECTOR_VALUE_H_
