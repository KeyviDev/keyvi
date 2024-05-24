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
 * iterator_utils.h
 *
 *  Created on: Jun 4, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_UTIL_ITERATOR_UTILS_H_
#define KEYVI_DICTIONARY_UTIL_ITERATOR_UTILS_H_

#include <utility>

namespace keyvi {
namespace dictionary {
namespace util {

template <typename Iterator>
struct iterator_pair : std::pair<Iterator, Iterator> {
  using std::pair<Iterator, Iterator>::pair;

  Iterator begin() const { return this->first; }
  Iterator end() const { return this->second; }
};

template <typename Iterator>
iterator_pair<Iterator> make_iterator_pair(Iterator f, Iterator l) {
  return iterator_pair<Iterator>(f, l);
}

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* KEYVI_DICTIONARY_UTIL_ITERATOR_UTILS_H_ */
