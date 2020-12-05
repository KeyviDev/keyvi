/* * keyvi - A key value store.
 *
 * Copyright 2020 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#ifndef KEYVI_DICTIONARY_DICTIONARY_COMPILER_COMMON_H_
#define KEYVI_DICTIONARY_DICTIONARY_COMPILER_COMMON_H_

#include <map>
#include <string>
#include <vector>

#include "keyvi/dictionary/fsa/generator.h"

namespace keyvi {
namespace dictionary {

/**
 * structure for internal processing
 * Note: Not using std::pair because it did not compile with Tpie
 */
template <typename KeyT, typename ValueT>
struct key_value_pair {
  key_value_pair() : key(), value() {}

  key_value_pair(const KeyT& k, const ValueT& v) : key(k), value(v) {}

  bool operator<(const key_value_pair kv) const { return key < kv.key; }

  bool operator==(const key_value_pair other) const {
    if (key != other.key) {
      return false;
    }

    return value == other.value;
  }

  KeyT key;
  ValueT value;
};

using key_value_t = key_value_pair<std::string, fsa::ValueHandle>;
using key_values_t = std::vector<key_value_t>;
/**
 * Exception class for generator, thrown when generator is used in the wrong
 * order.
 */

struct compiler_exception : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline size_t EstimateMemory(const std::string& key) {
  size_t string_size = key.capacity();

  // see https://shaharmike.com/cpp/std-string/ short strings are inlined
#ifdef __clang__
  if (string_size < 23) {
    string_size = 0;
  }
#elif __GNUC__
  if (string_size < 16) {
    string_size = 0;
  }
#elif _MSC_VER
  if (string_size < 16) {
    string_size = 0;
  }
#endif

  /* memory estimation explained:
   *   - size for the std:string instance
   *   - size for the value handle
   *   - dynamic allocation for non-inlined strings
   *   - size for the bucket in the vector
   *
   *   Note: The vector estimate is not correct as a vector allocates more buckets in advance,
   *   we ignore this as we only want an estimate
   */
  return sizeof(key) + string_size + sizeof(fsa::ValueHandle) + sizeof(key_values_t);
}

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_DICTIONARY_COMPILER_COMMON_H_
