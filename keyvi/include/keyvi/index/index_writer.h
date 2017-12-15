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
 * index_writer.h
 *
 *  Created on: Jan 11, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INDEX_WRITER_H_
#define KEYVI_INDEX_INDEX_WRITER_H_

#include <algorithm>
#include <atomic>
#include <chrono>              //NOLINT
#include <condition_variable>  //NOLINT
#include <ctime>
#include <string>
#include <thread>  //NOLINT
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "dictionary/dictionary.h"
#include "dictionary/dictionary_compiler.h"
#include "dictionary/dictionary_types.h"
#include "dictionary/fsa/internal/serialization_utils.h"
#include "dictionary/match.h"
#include "index/internal/base_index_reader.h"
#include "index/internal/index_writer_worker.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {

class IndexWriter final : public internal::BaseIndexReader<internal::IndexWriterWorker> {
 public:
  explicit IndexWriter(const std::string& index_directory)
      : index_finalizer_(index_directory), BaseIndexReader(index_finalizer_) {
    index_directory_ = index_directory;

    index_toc_file_ = index_directory_;
    index_toc_file_ /= "index.toc";

    // lock the index (filesystem lock)
    boost::filesystem::path index_lock_file = index_directory_;

    // create dir if it does not exist yet
    boost::filesystem::create_directories(index_directory_);

    index_lock_file /= "index.lock";

    TRACE("locking index %s", index_lock_file.string().c_str());

    std::ofstream o(index_lock_file.string(), std::ios_base::app);

    index_lock_ = boost::interprocess::file_lock(index_lock_file.string().c_str());
    index_lock_.lock();

    TRACE("Start Finalizer thread");
    index_finalizer_.StartWorkerThread();
  }

  ~IndexWriter() {
    TRACE("Stop Finalizer thread");
    index_finalizer_.StopWorkerThread();
    TRACE("Unlock Index");
    index_lock_.unlock();
  }

  void Set(const std::string& key, const std::string& value) {
    index_finalizer_.AcquireCompiler()->Add(key, value);
    index_finalizer_.ReleaseCompiler();
  }

  void Delete(const std::string& key) {
    index_finalizer_.AcquireCompiler();

    internal::segment_t last_segment_with_key;
    for (const internal::segment_t s : index_finalizer_.Segments()) {
      if (s->operator*()->Contains(key)) {
        last_segment_with_key = s;
      }
    }

    if (last_segment_with_key.get() != nullptr) {
      last_segment_with_key->DeleteKey(key);
    }

    index_finalizer_.ReleaseCompiler();
  }

  void Flush(bool async = true) {
    TRACE("Flush (manually)");
    index_finalizer_.Flush(async);
  }

 private:
  boost::filesystem::path index_directory_;
  boost::filesystem::path index_toc_file_;
  boost::interprocess::file_lock index_lock_;
  internal::IndexWriterWorker index_finalizer_;
};

} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INDEX_WRITER_H_
