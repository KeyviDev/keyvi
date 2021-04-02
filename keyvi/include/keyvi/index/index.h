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
#include <memory>
#include <string>
#include <thread>  //NOLINT
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/index/internal/base_index_reader.h"
#include "keyvi/index/internal/index_writer_worker.h"
#include "keyvi/index/internal/segment.h"
#include "keyvi/index/types.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace unit_test {
class IndexFriend;
}

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

  /**
   * Set key to given value
   *
   * @param key the key
   * @param value the value
   */
  void Set(const std::string& key, const std::string& value) { Payload().Add(key, value); }

  /**
   * Set multiple keys and to multiple values
   *
   * @param key_values a set of keys and values, the container must implement an iterator where each entry
   * has uses `.first` for the key and `.second` for the value, e.g. `std::map`
   */
  template <typename ContainerType>
  void MSet(const std::shared_ptr<ContainerType>& key_values) {
    Payload().Add(key_values);
  }

  /**
   * Delete a key
   *
   * @param key the key
   */
  void Delete(const std::string& key) { Payload().Delete(key); }

  /**
   * Flush the index, persists all pending writes and makes the accessible.
   *
   * @param async if true only trigger a flush, if false(default) wait until flush has been executed.
   */
  void Flush(const bool async = false) {
    TRACE("Flush (manually)");
    Payload().Flush(async);
  }

  /**
   * Force merge all segment to the number of segments given (default 1)
   *
   * @param max_segments maximum number of segments the index should have afterwards
   */
  void ForceMerge(const size_t max_segments = 1) {
    if (max_segments < 1) {
      throw std::invalid_argument("max_segments must be > 1");
    }
    Payload().ForceMerge(max_segments);
  }

 private:
  boost::filesystem::path index_directory_;
  boost::filesystem::path index_toc_file_;
  std::ofstream lock_file_;
  boost::interprocess::file_lock index_lock_;

  // friend for unit testing only
  friend class unit_test::IndexFriend;
};

} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INDEX_H_
