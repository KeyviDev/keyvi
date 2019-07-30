/* * keyvi - A key value store.
 *
 * Copyright 2015, 2016 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * dictionary_merger_fwd.h
 *
 * forward declarations for dictionary_merger
 *
 *  Created on: Mar 11, 2016
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_DICTIONARY_MERGER_FWD_H_
#define KEYVI_DICTIONARY_DICTIONARY_MERGER_FWD_H_

#include "keyvi/dictionary/fsa/internal/value_store_types.h"

namespace keyvi {
namespace dictionary {

template <keyvi::dictionary::fsa::internal::value_store_t DictionaryType>
class DictionaryMerger;

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_DICTIONARY_MERGER_FWD_H_
