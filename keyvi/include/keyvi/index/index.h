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
 * index.h
 *
 *  Created on: Jan 11, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INDEX_H_
#define KEYVI_INDEX_INDEX_H_

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

#include "dictionary/dictionary.h"
#include "index/internal/base_index_reader.h"
#include "index/internal/index_writer_worker.h"
#include "index/internal/segment.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {

class Index final : public internal::BaseIndexReader<internal::IndexWriterWorker, internal::Segment> {
 public:
  explicit Index(const std::string& index_directory,
                 const keyvi::util::parameters_t& params = keyvi::util::parameters_t())
      : BaseIndexReader(index_directory, params), lock_file_() {
    index_directory_ = index_directory;

    index_toc_file_ = index_directory_;
    index_toc_file_ /= "index.toc";

    // lock the index (filesystem lock)
    boost::filesystem::path index_lock_file = index_directory_;

    // create dir if it does not exist yet
    boost::filesystem::create_directories(index_directory_);

    index_lock_file /= "index.lock";

    TRACE("locking index %s", index_lock_file.string().c_str());

    lock_file_.open(index_lock_file.string(), std::ios_base::app);

    index_lock_ = boost::interprocess::file_lock(index_lock_file.string().c_str());
    index_lock_.lock();
  }

  ~Index() {
    // todo: happens to early, move into own class, destruct after worker is destructed
    TRACE("Unlock Index");
    try {
      index_lock_.unlock();
    } catch (std::exception& e) {
      TRACE("exception %s", e.what());
    }
  }

  void Set(const std::string& key, const std::string& value) { Payload().Add(key, value); }

  void Delete(const std::string& key) { Payload().Delete(key); }

  void Flush() {
    TRACE("Flush (manually)");
    Payload().Flush();
  }

  void FlushAsync() {
    TRACE("Flush (manually)");
    Payload().FlushAsync();
  }

 private:
  boost::filesystem::path index_directory_;
  boost::filesystem::path index_toc_file_;
  std::ofstream lock_file_;
  boost::interprocess::file_lock index_lock_;
};

} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INDEX_H_
