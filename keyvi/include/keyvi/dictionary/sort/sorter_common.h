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
 * sorter_common.h
 *
 *  Created on: Jul 17, 2014
 *      Author: hendrik
 */

#ifndef SORTER_COMMON_H_
#define SORTER_COMMON_H_

#include <map>
#include <string>

namespace keyvi {
namespace dictionary {
namespace sort {

/**
 * structure for internal processing
 * Note: Not using std::pair because it did not compile with Tpie
 */
template<typename KeyT, typename ValueT>
struct key_value_pair {
  key_value_pair() : key(), value() {
  }

  key_value_pair(const KeyT& k, const ValueT& v): key(k), value(v) {}

  bool operator<(const key_value_pair kv) const {
    return key < kv.key;
  }

  bool operator==(const key_value_pair other) const {
    if (key != other.key) {
      return false;
    }

    return value == other.value;
  }

  KeyT key;
  ValueT value;
};

typedef std::map<std::string, std::string> sorter_param_t;

} /* namespace sort */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* SORTER_COMMON_H_ */
