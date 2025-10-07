/* * keyvi - A key value store.
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

#ifndef KEYVI_COMPRESSION_COMPRESSION_ALGORITHM_H_
#define KEYVI_COMPRESSION_COMPRESSION_ALGORITHM_H_

namespace keyvi {
namespace compression {

enum CompressionAlgorithm {
  NO_COMPRESSION = 0,
  ZLIB_COMPRESSION = 1,
  SNAPPY_COMPRESSION = 2,
  ZSTD_COMPRESSION = 3,
};

} /* namespace compression */
} /* namespace keyvi */

#endif  // KEYVI_COMPRESSION_COMPRESSION_ALGORITHM_H_
