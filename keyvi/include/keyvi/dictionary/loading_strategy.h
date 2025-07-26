/* keyvi - A key value store.
 *
 * Copyright 2025 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#ifndef KEYVI_DICTIONARY_LOADING_STRATEGY_H_
#define KEYVI_DICTIONARY_LOADING_STRATEGY_H_

namespace keyvi {
namespace dictionary {

enum class loading_strategy_types {
  default_os,                    // no special treatment, use whatever the OS/Boost has as default
  lazy,                          // load data as needed with some read-ahead
  populate,                      // immediately load everything in memory (blocks until everything is fully read)
  populate_key_part,             // populate only the key part, load value part lazy
  populate_lazy,                 // load data lazy but ask the OS to read ahead if possible (does not block)
  lazy_no_readahead,             // disable any read-ahead (for cases when index > x * main memory)
  lazy_no_readahead_value_part,  // disable read-ahead only for the value part
  populate_key_part_no_readahead_value_part  // populate the key part, but disable read ahead value part
};

using LoadingStrategy = loading_strategy_types;

} /* namespace dictionary */
} /* namespace keyvi */
#endif  // KEYVI_DICTIONARY_LOADING_STRATEGY_H_
