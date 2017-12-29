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
#include <string>
#include <thread>  //NOLINT
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "dictionary/dictionary.h"
#include "dictionary/fsa/internal/serialization_utils.h"
#include "dictionary/match.h"
#include "index/internal/segment.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class IndexReaderWorker final {
 public:
  explicit IndexReaderWorker(const std::string index_directory,
                             size_t refresh_interval = 1 /*, optional external logger*/)
      : index_toc_(), segments_(), stop_update_thread_(true) {
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

  void Reload() { ReloadIndex(); }

  segments_t Segments() const { return segments_; }

 private:
  boost::filesystem::path index_directory_;
  boost::filesystem::path index_toc_file_;
  std::time_t last_modification_time_;
  boost::property_tree::ptree index_toc_;
  segments_t segments_;
  std::thread update_thread_;
  std::atomic_bool stop_update_thread_;

  void LoadIndex() {
    if (!boost::filesystem::exists(index_directory_)) {
      TRACE("No index found.");
      return;
    }
    TRACE("read toc");

    std::ifstream toc_fstream(index_toc_file_.string());

    TRACE("rereading %s", index_toc_file_.string().c_str());

    if (!toc_fstream.good()) {
      throw std::invalid_argument("file not found");
    }

    TRACE("read toc 2");

    boost::property_tree::read_json(toc_fstream, index_toc_);
    TRACE("index_toc loaded");
  }

  void ReloadIndex() {
    std::time_t t = boost::filesystem::last_write_time(index_toc_file_);

    if (t <= last_modification_time_) {
      TRACE("no modifications found");
      return;
    }

    TRACE("reload toc");
    last_modification_time_ = t;
    LoadIndex();

    TRACE("reading segments");

    segments_t new_segments = std::make_shared<segment_vec_t>();

    for (boost::property_tree::ptree::value_type& f : index_toc_.get_child("files")) {
      boost::filesystem::path p(index_directory_);
      p /= f.second.data();
      segment_t w(new Segment(p));
      new_segments->push_back(w);
    }

    segments_ = new_segments;
    TRACE("Loaded new segments");
  }

  void UpdateWatcher() {
    while (!stop_update_thread_) {
      TRACE("UpdateWatcher: Check for new segments");
      // reload
      ReloadIndex();

      // sleep for some time
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_READER_WORKER_H_
