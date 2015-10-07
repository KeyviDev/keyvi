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
 * intrinsics.h
 *
 *  Created on: Oct 6, 2015
 *      Author: hendrik
 */

#ifndef INTRINSICS_H_
#define INTRINSICS_H_

#if !defined(KEYVI_DISABLE_OPTIMIZATIONS) && defined(__SSE4_2__)
#define KEYVI_SSE42
#endif

#if defined(KEYVI_SSE42)
#include <nmmintrin.h>
#endif

#endif /* INTRINSICS_H_ */
