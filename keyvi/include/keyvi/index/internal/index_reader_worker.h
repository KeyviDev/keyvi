//
// keyvi - A key value store.
//
// Copyright 2017 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * index_reader_worker.h
 *
 *  Created on: Apr 15, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INTERNAL_INDEX_READER_WORKER_H_
#define KEYVI_INDEX_INTERNAL_INDEX_READER_WORKER_H_

#include <algorithm>
#include <atomic>
#include <chrono>  //NOLINT
#include <ctime>
#include <memory>
#include <mutex>  //NOLINT
#include <string>
#include <thread>  //NOLINT
#include <unordered_map>
#include <vector>

// boost json parser depends on boost::spirit, and spirit is not thread-safe by default. so need to enable thread-safety
#define BOOST_SPIRIT_THREADSAFE
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "dictionary/dictionary.h"
#include "dictionary/match.h"
#include "index/constants.h"
#include "index/internal/read_only_segment.h"
#include "util/configuration.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class IndexReaderWorker final {
 public:
  explicit IndexReaderWorker(const std::string index_directory, const keyvi::util::parameters_t& params)
      : index_toc_(),
        segments_(),
        refresh_interval_(
            std::chrono::milliseconds(keyvi::util::mapGet<uint64_t>(params, INDEX_REFRESH_INTERVAL, 1000))),
        stop_update_thread_(true) {
    index_directory_ = index_directory;

    index_toc_file_ = index_directory_;
    index_toc_file_ /= "index.toc";

    TRACE("Reader worker, index TOC: %s", index_toc_file_.string().c_str());

    last_modification_time_ = 0;
    ReloadIndex();
  }

  ~IndexReaderWorker() {
    stop_update_thread_ = true;
    if (update_thread_.joinable()) {
      update_thread_.join();
    }
  }

  void StartWorkerThread() {
    if (stop_update_thread_ == false) {
      // already runs
      return;
    }

    stop_update_thread_ = false;
    update_thread_ = std::thread(&IndexReaderWorker::UpdateWatcher, this);
  }

  void StopWorkerThread() {
    stop_update_thread_ = true;
    if (update_thread_.joinable()) {
      update_thread_.join();
    }
  }

  void Reload() {
    ReloadIndex();
    ReloadDeletedKeys();
  }

  const_read_only_segments_t Segments() {
    read_only_segments_t segments = segments_weak_.lock();
    if (!segments) {
      std::unique_lock<std::mutex> lock(mutex_);
      segments_weak_ = segments_;
      segments = segments_;
    }
    return segments;
  }

 private:
  boost::filesystem::path index_directory_;
  boost::filesystem::path index_toc_file_;
  std::time_t last_modification_time_;
  boost::property_tree::ptree index_toc_;
  read_only_segments_t segments_;
  std::weak_ptr<read_only_segment_vec_t> segments_weak_;
  std::mutex mutex_;
  std::unordered_map<std::string, read_only_segment_t> segments_by_name_;
  std::chrono::milliseconds refresh_interval_;
  std::thread update_thread_;
  std::atomic_bool stop_update_thread_;

  void ReloadIndex() {
    std::time_t t = boost::filesystem::last_write_time(index_toc_file_);

    if (t <= last_modification_time_) {
      TRACE("no modifications found");
      return;
    }

    TRACE("reload toc");
    last_modification_time_ = t;
    if (!boost::filesystem::exists(index_directory_)) {
      TRACE("No index found.");
      return;
    }
    std::ifstream toc_fstream(index_toc_file_.string());
    TRACE("rereading %s", index_toc_file_.string().c_str());

    if (!toc_fstream.good()) {
      throw std::invalid_argument("file not found");
    }

    boost::property_tree::read_json(toc_fstream, index_toc_);
    TRACE("index_toc loaded");

    TRACE("reading segments");

    read_only_segments_t new_segments = std::make_shared<read_only_segment_vec_t>();
    std::unordered_map<std::string, read_only_segment_t> new_segments_by_name;

    for (boost::property_tree::ptree::value_type& f : index_toc_.get_child("files")) {
      // check if segment is already loaded and reuse if possible
      if (segments_by_name_.count(f.second.data())) {
        new_segments->push_back(segments_by_name_.at(f.second.data()));
        new_segments_by_name[f.second.data()] = segments_by_name_.at(f.second.data());
      } else {
        boost::filesystem::path p(index_directory_);
        p /= f.second.data();
        read_only_segment_t w(new ReadOnlySegment(p));
        new_segments->push_back(w);
        new_segments_by_name[f.second.data()] = w;
      }
    }

    // thread-safe swap
    {
      std::unique_lock<std::mutex> lock(mutex_);
      segments_.swap(new_segments);
    }

    segments_by_name_.swap(new_segments_by_name);
    TRACE("Loaded new segments");
  }

  void ReloadDeletedKeys() {
    for (const read_only_segment_t& s : *segments_) {
      s->ReloadDeletedKeys();
    }
  }

  void UpdateWatcher() {
    while (!stop_update_thread_) {
      TRACE("UpdateWatcher: Check for new segments");
      // reload
      ReloadIndex();
      ReloadDeletedKeys();
      // sleep for next refresh
      std::this_thread::sleep_for(refresh_interval_);
    }
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_READER_WORKER_H_
