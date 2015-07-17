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
 * jump_consistent_hash.h
 *
 *  Created on: Feb 26, 2015
 *      Author: hendrik
 */

#ifndef JUMP_CONSISTENT_HASH_H_
#define JUMP_CONSISTENT_HASH_H_

#include "md5.h"
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace util {

inline uint32_t JumpConsistentHash(uint64_t key, uint32_t num_buckets) {
    int64_t b = -1, j = 0;
    while (j < num_buckets) {
        b   = j;
        key = key * 2862933555777941757ULL + 1;
        j   = (b + 1) * (double(1LL << 31) / double((key >> 33) + 1));
    }
    return b;
}

inline uint32_t JumpConsistentHashString(const char* key, uint32_t num_buckets) {
  misc::MD5 m = misc::MD5();

  uint64_t md5 = m.Hash(key);
  return JumpConsistentHash(md5, num_buckets);
}

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */


#endif /* JUMP_CONSISTENT_HASH_H_ */
