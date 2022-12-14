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
 * constants.h
 *
 *  Created on: May 5, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_CONSTANTS_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_CONSTANTS_H_

#include <cstddef>
#include <cstdint>

// file format definitions

// file magic
static const char KEYVI_FILE_MAGIC[] = "KEYVIFSA";
static const size_t KEYVI_FILE_MAGIC_LEN = 8;

// min version of the file
static const int KEYVI_FILE_VERSION_MIN = 2;
// max version of the file we support
static const int KEYVI_FILE_VERSION_MAX = 2;
// the current version of the file format
static const int KEYVI_FILE_VERSION_CURRENT = 2;

// min version of the persistence part
static const int KEYVI_FILE_PERSISTENCE_VERSION_MIN = 2;
static const size_t NUMBER_OF_STATE_CODINGS = 255;
static const uint16_t FINAL_OFFSET_TRANSITION = 256;
static const size_t FINAL_OFFSET_CODE = 1;
static const size_t INNER_WEIGHT_TRANSITION_COMPACT = 260;
static const size_t MAX_TRANSITIONS_OF_A_STATE = 261;

// Compact mode definitions
static const size_t COMPACT_SIZE_RELATIVE_MAX_VALUE = 32768;
static const size_t COMPACT_SIZE_ABSOLUTE_MAX_VALUE = 16384;
static const size_t COMPACT_SIZE_WINDOW = 512;
static const size_t COMPACT_SIZE_INNER_WEIGHT_MAX_VALUE = 0xffff;

// how many buckets to go left doing (brute force) search for free buckets in
// the sparse array where the new state fits in
static const size_t SPARSE_ARRAY_SEARCH_OFFSET = 151;

// 1 GB default memory limit for the dictionary compiler
static const size_t DEFAULT_MEMORY_LIMIT_COMPILER = 1 * 1024 * 1024 * 1024;

// 1 GB default memory limit for the generator
static const size_t DEFAULT_MEMORY_LIMIT_GENERATOR = 1 * 1024 * 1024 * 1024;

// 500MB default memory limit for value stores
static const size_t DEFAULT_MEMORY_LIMIT_VALUE_STORE = 500 * 1024 * 1024;

static const size_t DEFAULT_PARALLEL_SORT_THRESHOLD = 10000;

// default for vector values
static const size_t DEFAULT_VECTOR_SIZE = 10;

// option key names
static const char MEMORY_LIMIT_KEY[] = "memory_limit";
static const char TEMPORARY_PATH_KEY[] = "temporary_path";
static const char COMPRESSION_KEY[] = "compression";
static const char COMPRESSION_THRESHOLD_KEY[] = "compression_threshold";
static const char MINIMIZATION_KEY[] = "minimization";
static const char SINGLE_PRECISION_FLOAT_KEY[] = "floating_point_precision";
static const char PARALLEL_SORT_THRESHOLD_KEY[] = "parallel_sort_threshold";
static const char VECTOR_SIZE_KEY[] = "vector_size";
static const char MERGE_MODE[] = "merge_mode";
static const char MERGE_APPEND[] = "append";

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_CONSTANTS_H_
