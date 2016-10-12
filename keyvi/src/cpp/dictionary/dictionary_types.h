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

#ifndef DICTIONARY_TYPES_H_
#define DICTIONARY_TYPES_H_

#include "dictionary/fsa/generator.h"
#include "dictionary/dictionary_compiler.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/fsa/internal/int_value_store.h"
#include "dictionary/fsa/internal/json_value_store.h"
#include "dictionary/fsa/internal/string_value_store.h"
#include "dictionary/dictionary_merger.h"

namespace keyvi {
namespace dictionary {

typedef keyvi::dictionary::fsa::Generator<
    keyvi::dictionary::fsa::internal::SparseArrayPersistence<>> KeyOnlyDictionaryGenerator;

typedef keyvi::dictionary::DictionaryCompiler<
    keyvi::dictionary::fsa::internal::SparseArrayPersistence<>,
    keyvi::dictionary::fsa::internal::IntValueStoreWithInnerWeights> CompletionDictionaryCompiler;

typedef keyvi::dictionary::DictionaryCompiler<
    keyvi::dictionary::fsa::internal::SparseArrayPersistence<>> KeyOnlyDictionaryCompiler;

typedef keyvi::dictionary::DictionaryCompiler<
    keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>,
    keyvi::dictionary::fsa::internal::JsonValueStore> JsonDictionaryCompiler;

typedef keyvi::dictionary::DictionaryCompiler<
    keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>,
    keyvi::dictionary::fsa::internal::JsonValueStore,
    keyvi::dictionary::sort::InMemorySorter<key_value_t>> JsonDictionaryCompilerSmallData;

typedef keyvi::dictionary::DictionaryCompiler<
    keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>,
    keyvi::dictionary::fsa::internal::StringValueStore> StringDictionaryCompiler;

typedef keyvi::dictionary::DictionaryMerger<
    keyvi::dictionary::fsa::internal::SparseArrayPersistence<uint16_t>,
    keyvi::dictionary::fsa::internal::JsonValueStore> JsonDictionaryMerger;


} /* namespace dictionary */
} /* namespace keyvi */

#endif /* DICTIONARY_TYPES_H_ */
