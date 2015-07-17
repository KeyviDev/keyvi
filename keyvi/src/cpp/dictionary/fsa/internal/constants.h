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

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define NUMBER_OF_STATE_CODINGS 255
#define FINAL_OFFSET_TRANSITION 256
#define FINAL_OFFSET_CODE 1
#define INNER_WEIGHT_TRANSITION 257

#define INNER_WEIGHT_TRANSITION_COMPACT 260

#define MAX_TRANSITIONS_OF_A_STATE 261

// Compact mode definitions

#define COMPACT_SIZE_RELATIVE_MAX_VALUE 32768
#define COMPACT_SIZE_ABSOLUTE_MAX_VALUE 16384
#define COMPACT_SIZE_WINDOW 512
#define COMPACT_SIZE_INNER_WEIGHT_MAX_VALUE 0xffff

// how many buckets to go left doing (brute force) search for free buckets in the sparse array where the new state fits in
#define SPARSE_ARRAY_SEARCH_OFFSET 151

#endif /* CONSTANTS_H_ */
