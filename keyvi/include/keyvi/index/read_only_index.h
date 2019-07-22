//
// keyvi - A key value store.
//
// Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/*
 * read_only_index.h
 *
 *  Created on: Jan 11, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_READ_ONLY_INDEX_H_
#define KEYVI_INDEX_READ_ONLY_INDEX_H_

#include <string>

#include "keyvi/index/internal/base_index_reader.h"
#include "keyvi/index/internal/index_reader_worker.h"
#include "keyvi/util/configuration.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {

class ReadOnlyIndex final : public internal::BaseIndexReader<internal::IndexReaderWorker> {
 public:
  explicit ReadOnlyIndex(const std::string index_directory,
                         const keyvi::util::parameters_t& params = keyvi::util::parameters_t())
      : BaseIndexReader(index_directory, params) {
    Payload().StartWorkerThread();
  }

  ~ReadOnlyIndex() { Payload().StopWorkerThread(); }

  void Reload() { Payload().Reload(); }
};
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_READ_ONLY_INDEX_H_
