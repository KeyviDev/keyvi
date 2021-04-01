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
#ifndef KEYVI_INDEX_INTERNAL_MERGE_JOB_H_
#define KEYVI_INDEX_INTERNAL_MERGE_JOB_H_

#include <atomic>
#include <chrono>  //NOLINT
#include <functional>
#include <memory>
#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "process.hpp"

#include "keyvi/dictionary/dictionary_merger.h"
#include "keyvi/dictionary/dictionary_types.h"
#include "keyvi/dictionary/fsa/internal/json_value_store.h"
#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/index/internal/index_settings.h"
#include "keyvi/index/internal/segment.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class MergeJob final {
  struct MergeJobPayload {
    explicit MergeJobPayload(std::vector<segment_t> segments, const boost::filesystem::path& output_filename,
                             const IndexSettings& settings)
        : segments_(segments), output_filename_(output_filename), settings_(settings), process_finished_(false) {}

    MergeJobPayload() = delete;
    MergeJobPayload& operator=(MergeJobPayload const&) = delete;
    MergeJobPayload(const MergeJobPayload& that) = delete;

    std::vector<segment_t> segments_;
    boost::filesystem::path output_filename_;
    const IndexSettings& settings_;
    std::chrono::time_point<std::chrono::system_clock> start_time_;
    std::chrono::time_point<std::chrono::system_clock> end_time_;
    int exit_code_ = -1;
    bool merge_done = false;
    std::atomic_bool process_finished_;
  };

 public:
  // todo: add ability to stop merging for shutdown
  explicit MergeJob(segment_vec_t segments, size_t id, const boost::filesystem::path& output_filename,
                    const IndexSettings& settings)
      : payload_(segments, output_filename, settings), id_(id), external_process_() {}

  ~MergeJob() {
    if (payload_.process_finished_ == false) {
      FinalizeMerge();
    }
  }

  MergeJob() = delete;
  MergeJob& operator=(MergeJob const&) = delete;
  MergeJob(const MergeJob& that) = delete;

  void Run(bool force_external_merge = false) {
    uint64_t job_size = 0;

    for (const segment_t& segment : payload_.segments_) {
      job_size += segment->GetDictionaryProperties()->GetNumberOfKeys();
    }

    if (force_external_merge == false && job_size < payload_.settings_.GetSegmentExternalMergeKeyThreshold()) {
      DoInternalMerge();
    } else {
      DoExternalProcessMerge();
    }
  }

  bool TryFinalize() {
    // already finished?
    if (payload_.process_finished_ == true) {
      return true;
    }
    return TryFinalizeMerge();
  }

  void Finalize() {
    if (payload_.process_finished_ == false) {
      FinalizeMerge();
    }
  }

  bool Successful() {
    // todo: handle case when process crashed
    return payload_.exit_code_ == 0;
  }

  const std::vector<segment_t>& Segments() const { return payload_.segments_; }

  const segment_t MergedSegment() const {
    return segment_t(new Segment(payload_.output_filename_, payload_.segments_));
  }

  void SetMerged() { payload_.merge_done = true; }

  const bool Merged() const { return payload_.merge_done; }

  size_t GetId() const { return id_; }

  // todo: ability to kill job/process

 private:
  MergeJobPayload payload_;
  size_t id_;
  std::shared_ptr<TinyProcessLib::Process> external_process_;
  std::thread internal_merge_;

  void DoInternalMerge() {
    payload_.start_time_ = std::chrono::system_clock::now();

    internal_merge_ = std::thread([this]() {
      try {
        keyvi::util::parameters_t params;

        // todo: make this configurable
        params[MEMORY_LIMIT_KEY] = "5242880";
        keyvi::dictionary::JsonDictionaryMerger jsonDictionaryMerger(params);
        for (const segment_t& s : payload_.segments_) {
          jsonDictionaryMerger.Add(s->GetDictionaryPath().string());
        }

        jsonDictionaryMerger.Merge(payload_.output_filename_.string());
        payload_.exit_code_ = 0;
      } catch (const std::exception& e) {
        TRACE("internal merge failed with: %s", e.what());
        payload_.exit_code_ = 1;
      }
    });
  }

  void DoExternalProcessMerge() {
    payload_.start_time_ = std::chrono::system_clock::now();

    std::stringstream command;

    command << payload_.settings_.GetKeyviMergerBin();
    command << " -m 5242880";

    for (auto s : payload_.segments_) {
      command << " -i " << s->GetDictionaryPath().string();
    }

    command << " -o " << payload_.output_filename_.string();
    external_process_.reset(new TinyProcessLib::Process(command.str()));
  }

  bool TryFinalizeMerge() {
    if (external_process_) {
      if (external_process_->try_get_exit_status(payload_.exit_code_)) {
        payload_.process_finished_ = true;
        return true;
      }
    } else if (internal_merge_.joinable()) {
      internal_merge_.join();
      // exit code set by merge thread
      payload_.process_finished_ = true;
      return true;
    }
    return false;
  }

  void FinalizeMerge() {
    if (external_process_) {
      payload_.exit_code_ = external_process_->get_exit_status();
    } else {
      internal_merge_.join();
      // exit code set by merge thread
    }
    payload_.end_time_ = std::chrono::system_clock::now();
    payload_.process_finished_ = true;
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_MERGE_JOB_H_
