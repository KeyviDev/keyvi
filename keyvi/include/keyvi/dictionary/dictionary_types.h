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
 * key_only_dictionary.h
 *
 *  Created on: May 26, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_DICTIONARY_TYPES_H_
#define KEYVI_DICTIONARY_DICTIONARY_TYPES_H_

#include "dictionary/dictionary_compiler.h"
#include "dictionary/dictionary_merger.h"
#include "dictionary/fsa/generator.h"
#include "dictionary/fsa/internal/int_inner_weights_value_store.h"
#include "dictionary/fsa/internal/int_value_store.h"
#include "dictionary/fsa/internal/ivalue_store.h"
#include "dictionary/fsa/internal/json_value_store.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/fsa/internal/string_value_store.h"

namespace keyvi {
namespace dictionary {

// forward define the value store types as dictionary types
using dictionary_type_t = fsa::internal::value_store_t;

typedef keyvi::dictionary::fsa::Generator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<>>
    KeyOnlyDictionaryGenerator;

typedef keyvi::dictionary::DictionaryCompiler<dictionary_type_t::INT_WITH_WEIGHTS> CompletionDictionaryCompiler;

typedef keyvi::dictionary::DictionaryCompiler<dictionary_type_t::INT> IntDictionaryCompiler;

typedef keyvi::dictionary::DictionaryCompiler<dictionary_type_t::KEY_ONLY> KeyOnlyDictionaryCompiler;

typedef keyvi::dictionary::DictionaryCompiler<dictionary_type_t::JSON> JsonDictionaryCompiler;

typedef keyvi::dictionary::DictionaryCompiler<dictionary_type_t::JSON,
                                              keyvi::dictionary::sort::InMemorySorter<key_value_t>>
    JsonDictionaryCompilerSmallData;

typedef keyvi::dictionary::DictionaryCompiler<dictionary_type_t::STRING> StringDictionaryCompiler;

typedef keyvi::dictionary::DictionaryMerger<dictionary_type_t::JSON> JsonDictionaryMerger;

typedef keyvi::dictionary::DictionaryMerger<dictionary_type_t::INT_WITH_WEIGHTS> CompletionDictionaryMerger;

typedef keyvi::dictionary::DictionaryMerger<dictionary_type_t::INT> IntDictionaryMerger;

typedef keyvi::dictionary::DictionaryMerger<dictionary_type_t::STRING> StringDictionaryMerger;

typedef keyvi::dictionary::DictionaryMerger<dictionary_type_t::KEY_ONLY> KeyOnlyDictionaryMerger;

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_DICTIONARY_TYPES_H_
