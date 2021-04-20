/* * keyvi - A key value store.
 *
 * Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 *  Created on: Jan 14, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_CONSTANTS_H_
#define KEYVI_INDEX_CONSTANTS_H_

#include <cstddef>

static const char INDEX_REFRESH_INTERVAL[] = "refresh_interval";
static const char MERGE_POLICY[] = "merge_policy";
static const char DEFAULT_MERGE_POLICY[] = "tiered";
static const char KEYVIMERGER_BIN[] = "keyvimerger_bin";
static const char INDEX_MAX_SEGMENTS[] = "max_segments";
static const char SEGMENT_COMPILE_KEY_THRESHOLD[] = "segment_compile_key_threshold";
static const char SEGMENT_EXTERNAL_MERGE_KEY_THRESHOLD[] = "segment_external_merge_key_threshold";
static const char MAX_CONCURRENT_MERGES[] = "max_concurrent_merges";

// defaults
static const size_t DEFAULT_REFRESH_INTERVAL = 1000ul;
static const size_t DEFAULT_COMPILE_KEY_THRESHOLD = 10000ul;
static const size_t DEFAULT_EXTERNAL_MERGE_KEY_THRESHOLD = 100000ul;
#if defined(_WIN32)
static const char DEFAULT_KEYVIMERGER_BIN[] = "keyvimerger.exe";
#else
static const char DEFAULT_KEYVIMERGER_BIN[] = "keyvimerger";
#endif

// spinlock wait time if there are too many segments
static const size_t SPINLOCK_WAIT_FOR_SEGMENT_MERGES_MS = 10;

// max parallel process for segment merging
static const size_t MAX_CONCURRENT_MERGES_DEFAULT = 8;

#endif  // KEYVI_INDEX_CONSTANTS_H_
