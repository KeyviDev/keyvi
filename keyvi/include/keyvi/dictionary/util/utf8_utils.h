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

#ifndef UTF8_UTILS_H_
#define UTF8_UTILS_H_


namespace keyvi {
namespace dictionary {
namespace util {

class Utf8Utils final{
 public:
  static bool IsLeadByte(char utf8_byte) {
    int intValue = utf8_byte & 0xFF;

    if (intValue >= 0xF8) {
      throw std::invalid_argument("Illegal UTF-8 byte: " + std::to_string(intValue));
    }

    return (intValue < 0x80) || (intValue >= 0xC0);
  }

  static size_t GetCharLength(char utf8_lead_byte)
  {
     int intValue = utf8_lead_byte & 0xff;

     if (intValue < 0x80)
     {
         return 1;
     }
     else if (intValue < 0xc0)
     {
       std::invalid_argument("Illegal UTF-8 lead byte: " + std::to_string(intValue));
     }
     else if (intValue < 0xe0)
     {
         return 2;
     }
     else if (intValue < 0xf0)
     {
         return 3;
     }
     else if (intValue < 0xf8)
     {
         return 4;
     }

     throw std::invalid_argument("Illegal UTF-8 lead byte: " + std::to_string(intValue));
  }
};

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */
#endif /* UTF8_UTILS_H_ */
