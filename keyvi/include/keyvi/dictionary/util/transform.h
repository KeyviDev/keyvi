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
 * transform.h
 *
 *  Created on: Jul 22, 2014
 *      Author: hendrik
 */

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include <boost/algorithm/string.hpp>

namespace keyvi {
namespace dictionary {
namespace util {

class Transform final{
 public:
  /**
   * Apply Bag of Words reordering for all but the last token
   * @param input
   * @return token with bow applied
   */
  static std::string BagOfWordsPartial(const std::string& input, size_t& number_of_tokens)
  {
    std::vector<std::string> strs;
    boost::split(strs, input, boost::is_any_of("\t "));
    number_of_tokens = strs.size();

    if (strs.size() == 1) {
      return input;
    }

    std::sort(strs.begin(), strs.end() - 1);
    return boost::algorithm::join(strs, " ");
  }
};

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* TRANSFORM_H_ */
