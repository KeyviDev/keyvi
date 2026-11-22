/* keyvi - A key value store.
 *
 * Copyright 2026 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * value_store_append_merge.h
 *
 * Helpers for append merges of value stores.
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_APPEND_MERGE_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_APPEND_MERGE_H_

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "keyvi/dictionary/fsa/internal/value_store_properties.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * Copies the value store payload of a dictionary file to stream (used by append merges).
 *
 * The file is read in binary mode, exactly the size of the value store is copied, throws if the file can not be
 * read or is truncated.
 */
inline void CopyValueStorePayload(const std::string& filename, const ValueStoreProperties& properties,
                                  std::ostream* stream) {
  std::ifstream in_stream(filename, std::ios::binary);
  if (!in_stream.good()) {
    throw std::invalid_argument("can not open dictionary file: " + filename);
  }
  in_stream.seekg(properties.GetOffset());
  if (!in_stream.good()) {
    throw std::invalid_argument("file is corrupt(truncated): " + filename);
  }

  std::vector<char> buffer(64 * 1024);
  size_t remaining = properties.GetSize();
  while (remaining > 0) {
    const size_t chunk_size = std::min(remaining, buffer.size());
    in_stream.read(buffer.data(), chunk_size);
    if (static_cast<size_t>(in_stream.gcount()) != chunk_size) {
      throw std::invalid_argument("file is corrupt(truncated): " + filename);
    }
    stream->write(buffer.data(), chunk_size);
    remaining -= chunk_size;
  }
}

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_VALUE_STORE_APPEND_MERGE_H_
