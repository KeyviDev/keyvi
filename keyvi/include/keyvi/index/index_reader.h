//
// keyvi - A key value store.
//
// Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * index_reader.h
 *
 *  Created on: Jan 11, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INDEX_READER_H_
#define KEYVI_INDEX_INDEX_READER_H_

#include <string>

#include "index/internal/base_index_reader.h"
#include "index/internal/index_reader_worker.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {

class IndexReader final : public internal::BaseIndexReader<internal::IndexReaderWorker> {
 public:
  explicit IndexReader(const std::string index_directory, size_t refresh_interval = 1 /*, optional external logger*/)
      : worker_(index_directory, refresh_interval), BaseIndexReader(worker_) {
    worker_.StartWorkerThread();
  }

  ~IndexReader() { worker_.StopWorkerThread(); }

  void Reload() { worker_.Reload(); }

 private:
  internal::IndexReaderWorker worker_;
};
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INDEX_READER_H_
