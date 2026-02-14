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
 * utf8_utils.h
 *
 *  Created on: Jun 24, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_UTIL_UTF8_UTILS_H_
#define KEYVI_DICTIONARY_UTIL_UTF8_UTILS_H_

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace keyvi {
namespace dictionary {
namespace util {

class Utf8Utils final {
 public:
  static bool IsLeadByte(char utf8_byte) {
    int intValue = utf8_byte & 0xFF;

    if (intValue >= 0xF8) {
      throw std::invalid_argument("Illegal UTF-8 byte: " + std::to_string(intValue));
    }

    return (intValue < 0x80) || (intValue >= 0xC0);
  }

  [[nodiscard]]
  static size_t GetCharLength(unsigned char leadByte) {
    if ((leadByte & 0x80U) == 0x00U) {  // 0xxxxxxx
      return 1;
    }

    if ((leadByte & 0xE0u) == 0xC0U) {  // 110xxxxx
      return 2;
    }

    if ((leadByte & 0xF0U) == 0xE0U) {  // 1110xxxx
      return 3;
    }

    if ((leadByte & 0xF8U) == 0xF0U) {  // 11110xxx
      return 4;
    }

    throw std::invalid_argument("Illegal UTF-8 lead byte: " + std::to_string(leadByte));
  }
};

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */
#endif  // KEYVI_DICTIONARY_UTIL_UTF8_UTILS_H_
