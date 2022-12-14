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

#include "keyvi/dictionary/dictionary_compiler.h"
#include "keyvi/dictionary/dictionary_index_compiler.h"
#include "keyvi/dictionary/dictionary_merger.h"
#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/internal/int_inner_weights_value_store.h"
#include "keyvi/dictionary/fsa/internal/int_value_store.h"
#include "keyvi/dictionary/fsa/internal/ivalue_store.h"
#include "keyvi/dictionary/fsa/internal/json_value_store.h"
#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/dictionary/fsa/internal/string_value_store.h"

namespace keyvi {
namespace dictionary {

// forward define the value store types as dictionary types
using dictionary_type_t = fsa::internal::value_store_t;

using KeyOnlyDictionaryGenerator =
    keyvi::dictionary::fsa::Generator<keyvi::dictionary::fsa::internal::SparseArrayPersistence<>>;

using CompletionDictionaryCompiler = keyvi::dictionary::DictionaryCompiler<dictionary_type_t::INT_WITH_WEIGHTS>;

using FloatVectorDictionaryCompiler = keyvi::dictionary::DictionaryCompiler<dictionary_type_t::FLOAT_VECTOR>;

using IntDictionaryCompiler = keyvi::dictionary::DictionaryCompiler<dictionary_type_t::INT>;

using KeyOnlyDictionaryCompiler = keyvi::dictionary::DictionaryCompiler<dictionary_type_t::KEY_ONLY>;

using JsonDictionaryCompiler = keyvi::dictionary::DictionaryCompiler<dictionary_type_t::JSON>;

using StringDictionaryCompiler = keyvi::dictionary::DictionaryCompiler<dictionary_type_t::STRING>;

using JsonDictionaryMerger = keyvi::dictionary::DictionaryMerger<dictionary_type_t::JSON>;

using CompletionDictionaryMerger = keyvi::dictionary::DictionaryMerger<dictionary_type_t::INT_WITH_WEIGHTS>;

using IntDictionaryMerger = keyvi::dictionary::DictionaryMerger<dictionary_type_t::INT>;

using StringDictionaryMerger = keyvi::dictionary::DictionaryMerger<dictionary_type_t::STRING>;

using KeyOnlyDictionaryMerger = keyvi::dictionary::DictionaryMerger<dictionary_type_t::KEY_ONLY>;

using JsonDictionaryIndexCompiler = keyvi::dictionary::DictionaryIndexCompiler<dictionary_type_t::JSON>;

#ifndef KEYVI_REMOVE_DEPRECATED
using IntDictionaryCompilerSmallData = IntDictionaryCompiler;

using JsonDictionaryCompilerSmallData = JsonDictionaryCompiler;
#endif

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_DICTIONARY_TYPES_H_
