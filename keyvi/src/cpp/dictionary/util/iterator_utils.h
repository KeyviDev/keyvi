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

#ifndef ITERATOR_UTILS_H_
#define ITERATOR_UTILS_H_

namespace keyvi {
namespace dictionary {
namespace util {

template<typename Iterator>
class iterator_pair {
 public:
  iterator_pair(){}

  iterator_pair(Iterator first, Iterator last)
      : f_(first),
        l_(last) {
  }
  Iterator begin() const {
    return f_;
  }
  Iterator end() const {
    return l_;
  }

 private:
  Iterator f_;
  Iterator l_;
};

template<typename Iterator>
iterator_pair<Iterator> make_iterator_pair(Iterator f, Iterator l) {
  return iterator_pair<Iterator>(f, l);
}

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* ITERATOR_UTILS_H_ */
